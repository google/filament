// File: basisu_astc_ldr_common.cpp
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
#include "../transcoder/basisu_astc_helpers.h"
#include "../transcoder/basisu_astc_hdr_core.h"
#include "basisu_astc_hdr_common.h"
#include "basisu_astc_ldr_common.h"

#define BASISU_ASTC_LDR_DEBUG_MSGS (1)

namespace basisu
{

namespace astc_ldr
{
	static bool g_initialized;
	static vec4F g_astc_ls_raw_weights_ise[ASTC_LDR_MAX_RAW_WEIGHTS];
			
	color_rgba blue_contract_enc(color_rgba orig, bool& did_clamp, int encoded_b)
	{
		color_rgba enc;

		int tr = orig.r * 2 - encoded_b;
		int tg = orig.g * 2 - encoded_b;
		if ((tr < 0) || (tr > 255) || (tg < 0) || (tg > 255))
			did_clamp = true;

		enc.r = (uint8_t)basisu::clamp<int>(tr, 0, 255);
		enc.g = (uint8_t)basisu::clamp<int>(tg, 0, 255);
		enc.b = (uint8_t)orig.b;
		enc.a = orig.a;
		return enc;
	}

	color_rgba blue_contract_dec(int enc_r, int enc_g, int enc_b, int enc_a)
	{
		color_rgba dec;
		dec.r = (uint8_t)((enc_r + enc_b) >> 1);
		dec.g = (uint8_t)((enc_g + enc_b) >> 1);
		dec.b = (uint8_t)enc_b;
		dec.a = (uint8_t)enc_a;
		return dec;
	}

	void global_init()
	{
		if (g_initialized)
			return;

		// Precomputed weight constants used during least fit determination. For each entry: w * w, (1.0f - w) * w, (1.0f - w) * (1.0f - w), w
		for (uint32_t iw = 0; iw <= 64; iw++)
		{
			float w = (float)iw * (1.0f / 64.0f);

			g_astc_ls_raw_weights_ise[iw].set(w * w, (1.0f - w) * w, (1.0f - w) * (1.0f - w), w);
		}
				
		g_initialized = true;
	}

	static inline const vec4F* get_ls_weights_ise(uint32_t weight_ise_range)
	{
		assert((weight_ise_range <= astc_helpers::BISE_32_LEVELS) || (weight_ise_range == astc_helpers::BISE_64_LEVELS));

		// astc_helpers::BISE_64_LEVELS indicates raw [0,64] weights (65 total), otherwise ISE weights (<= 32 levels total)
		return (weight_ise_range == astc_helpers::BISE_64_LEVELS) ? g_astc_ls_raw_weights_ise : &g_astc_ls_weights_ise[weight_ise_range][0];
	}
	
	static bool compute_least_squares_endpoints_1D(
		uint32_t N, const uint8_t* pSelectors, const vec4F* pSelector_weights,
		float* pXl, float* pXh, const float* pVals, float bounds_min, float bounds_max)
	{
		float z00 = 0.0f, z01 = 0.0f, z10 = 0.0f, z11 = 0.0f;
		float q00_r = 0.0f, q10_r = 0.0f, t_r = 0.0f;

		for (uint32_t i = 0; i < N; i++)
		{
			const uint32_t sel = pSelectors[i];

			z00 += pSelector_weights[sel][0];
			z10 += pSelector_weights[sel][1];
			z11 += pSelector_weights[sel][2];

			float w = pSelector_weights[sel][3];

			q00_r += w * pVals[i];
			t_r += pVals[i];
		}

		q10_r = t_r - q00_r;

		z01 = z10;

		float det = z00 * z11 - z01 * z10;
		if (fabs(det) < 1e-8f)
			return false;

		det = 1.0f / det;

		float iz00, iz01, iz10, iz11;
		iz00 = z11 * det;
		iz01 = -z01 * det;
		iz10 = -z10 * det;
		iz11 = z00 * det;

		*pXh = (float)(iz00 * q00_r + iz01 * q10_r); *pXl = (float)(iz10 * q00_r + iz11 * q10_r);

		float l = saturate(*pXl), h = saturate(*pXh);

		if (bounds_min == bounds_max)
		{
			l = bounds_min;
			h = bounds_max;
		}

		*pXl = l;
		*pXh = h;

		return true;
	}

	static bool compute_least_squares_endpoints_2D(
		uint32_t N, const uint8_t* pSelectors, const vec4F* pSelector_weights,
		vec2F* pXl, vec2F* pXh, const vec2F* pColors, const vec2F& bounds_min, const vec2F& bounds_max)
	{
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
		if (fabs(det) < 1e-8f)
			return false;

		det = 1.0f / det;

		float iz00, iz01, iz10, iz11;
		iz00 = z11 * det;
		iz01 = -z01 * det;
		iz10 = -z10 * det;
		iz11 = z00 * det;

		(*pXh)[0] = (float)(iz00 * q00_r + iz01 * q10_r); (*pXl)[0] = (float)(iz10 * q00_r + iz11 * q10_r);
		(*pXh)[1] = (float)(iz00 * q00_g + iz01 * q10_g); (*pXl)[1] = (float)(iz10 * q00_g + iz11 * q10_g);

		for (uint32_t c = 0; c < 2; c++)
		{
			float l = saturate((*pXl)[c]), h = saturate((*pXh)[c]);

			if (bounds_min[c] == bounds_max[c])
			{
				l = bounds_min[c];
				h = bounds_max[c];
			}

			(*pXl)[c] = l;
			(*pXh)[c] = h;
		}

		return true;
	}

	static bool compute_least_squares_endpoints_3D(
		uint32_t N, const uint8_t* pSelectors, const vec4F* pSelector_weights,
		vec4F* pXl, vec4F* pXh, const vec4F* pColors, const vec4F& bounds_min, const vec4F& bounds_max)
	{
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
		if (fabs(det) < 1e-8f)
			return false;

		det = 1.0f / det;

		float iz00, iz01, iz10, iz11;
		iz00 = z11 * det;
		iz01 = -z01 * det;
		iz10 = -z10 * det;
		iz11 = z00 * det;

		(*pXh)[0] = (float)(iz00 * q00_r + iz01 * q10_r); (*pXl)[0] = (float)(iz10 * q00_r + iz11 * q10_r);
		(*pXh)[1] = (float)(iz00 * q00_g + iz01 * q10_g); (*pXl)[1] = (float)(iz10 * q00_g + iz11 * q10_g);
		(*pXh)[2] = (float)(iz00 * q00_b + iz01 * q10_b); (*pXl)[2] = (float)(iz10 * q00_b + iz11 * q10_b);

		(*pXh)[3] = 0;
		(*pXl)[3] = 0;

		for (uint32_t c = 0; c < 3; c++)
		{
			float l = saturate((*pXl)[c]), h = saturate((*pXh)[c]);

			if (bounds_min[c] == bounds_max[c])
			{
				l = bounds_min[c];
				h = bounds_max[c];
			}

			(*pXl)[c] = l;
			(*pXh)[c] = h;
		}

		return true;
	}
		
	static bool compute_least_squares_endpoints_4D(
		uint32_t N, const uint8_t* pSelectors, const vec4F* pSelector_weights,
		vec4F* pXl, vec4F* pXh, const vec4F* pColors, const vec4F& bounds_min, const vec4F& bounds_max)
	{
		float z00 = 0.0f, z01 = 0.0f, z10 = 0.0f, z11 = 0.0f;
		float q00_r = 0.0f, q10_r = 0.0f, t_r = 0.0f;
		float q00_g = 0.0f, q10_g = 0.0f, t_g = 0.0f;
		float q00_b = 0.0f, q10_b = 0.0f, t_b = 0.0f;
		float q00_a = 0.0f, q10_a = 0.0f, t_a = 0.0f;

		for (uint32_t i = 0; i < N; i++)
		{
			const uint32_t sel = pSelectors[i];
			z00 += pSelector_weights[sel][0];
			z10 += pSelector_weights[sel][1];
			z11 += pSelector_weights[sel][2];

			float w = pSelector_weights[sel][3];
			q00_r += w * pColors[i][0]; t_r += pColors[i][0];
			q00_g += w * pColors[i][1]; t_g += pColors[i][1];
			q00_b += w * pColors[i][2]; t_b += pColors[i][2];
			q00_a += w * pColors[i][3]; t_a += pColors[i][3];
		}

		q10_r = t_r - q00_r;
		q10_g = t_g - q00_g;
		q10_b = t_b - q00_b;
		q10_a = t_a - q00_a;

		z01 = z10;

		float det = z00 * z11 - z01 * z10;
		if (fabs(det) < 1e-8f)
			return false;

		det = 1.0f / det;

		float iz00, iz01, iz10, iz11;
		iz00 = z11 * det;
		iz01 = -z01 * det;
		iz10 = -z10 * det;
		iz11 = z00 * det;

		(*pXh)[0] = (float)(iz00 * q00_r + iz01 * q10_r); (*pXl)[0] = (float)(iz10 * q00_r + iz11 * q10_r);
		(*pXh)[1] = (float)(iz00 * q00_g + iz01 * q10_g); (*pXl)[1] = (float)(iz10 * q00_g + iz11 * q10_g);
		(*pXh)[2] = (float)(iz00 * q00_b + iz01 * q10_b); (*pXl)[2] = (float)(iz10 * q00_b + iz11 * q10_b);
		(*pXh)[3] = (float)(iz00 * q00_a + iz01 * q10_a); (*pXl)[3] = (float)(iz10 * q00_a + iz11 * q10_a);

		for (uint32_t c = 0; c < 4; c++)
		{
			float l = saturate((*pXl)[c]), h = saturate((*pXh)[c]);

			if (bounds_min[c] == bounds_max[c])
			{
				l = bounds_min[c];
				h = bounds_max[c];
			}

			(*pXl)[c] = l;
			(*pXh)[c] = h;
		}

		return true;
	}

#if 0
	static void dequant_astc_weights(uint32_t n, const uint8_t* pSrc_ise_vals, uint32_t from_ise_range, uint8_t* pDst_raw_weights)
	{
		const auto& dequant_tab = astc_helpers::g_dequant_tables.get_weight_tab(from_ise_range).m_ISE_to_val;

		for (uint32_t i = 0; i < n; i++)
			pDst_raw_weights[i] = dequant_tab[pSrc_ise_vals[i]];
	}
#endif

#if 0
	static void dequant_astc_endpoints(uint32_t n, const uint8_t* pSrc_ise_vals, uint32_t from_ise_range, uint8_t* pDst_raw_weights)
	{
		const auto& dequant_tab = astc_helpers::g_dequant_tables.get_endpoint_tab(from_ise_range).m_ISE_to_val;

		for (uint32_t i = 0; i < n; i++)
			pDst_raw_weights[i] = dequant_tab[pSrc_ise_vals[i]];
	}
#endif
		
	int apply_delta_to_bise_weight_val(uint32_t weight_ise_range, int ise_val, int delta)
	{
		if (delta == 0)
			return ise_val;

		uint32_t num_ise_levels = astc_helpers::get_ise_levels(weight_ise_range);

		const auto& ISE_to_rank = astc_helpers::g_dequant_tables.get_weight_tab(weight_ise_range).m_ISE_to_rank;
		const auto& rank_to_ISE = astc_helpers::g_dequant_tables.get_weight_tab(weight_ise_range).m_rank_to_ISE;

		int cur_rank = ISE_to_rank[ise_val];
		int new_rank = basisu::clamp<int>(cur_rank + delta, 0, (int)num_ise_levels - 1);

		return rank_to_ISE[new_rank];
	}

	// v must be [0,1]
	// converts to nearest ISE index with proper precise rounding
	static uint8_t precise_round_bise_endpoint_val(float v, uint32_t endpoint_ise_range)
	{
		assert((v >= 0) && (v <= 1.0f));

		const auto& quant_tab = astc_helpers::g_dequant_tables.get_endpoint_tab(endpoint_ise_range).m_val_to_ise;
		const auto& dequant_tab = astc_helpers::g_dequant_tables.get_endpoint_tab(endpoint_ise_range).m_ISE_to_val;

		v = saturate(v);

		const int iv = clamp((int)std::roundf(v * 255.0f), 0, 255);

		uint8_t ise_index = 0;

		float best_err = BIG_FLOAT_VAL;
		for (int iscale_delta = -1; iscale_delta <= 1; iscale_delta++)
		{
			const int trial_ise_index = astc_helpers::apply_delta_to_bise_endpoint_val(endpoint_ise_range, quant_tab[iv], iscale_delta);

			const float dequant_val = dequant_tab[trial_ise_index] * (1.0f / 255.0f);

			const float dequant_err = fabs(dequant_val - v);
			if (dequant_err < best_err)
			{
				best_err = dequant_err;
				ise_index = (uint8_t)trial_ise_index;
			}
		} // iscale_delta

		return ise_index;
	}

	// returns true if blue contraction was actually used
	// note the encoded endpoints may be swapped
	// TODO: Pass in vec4F l/h and let it more precisely quantize in here.
	struct cem_encode_ldr_rgb_or_rgba_direct_result
	{
		bool m_is_blue_contracted;
		bool m_endpoints_are_swapped;
		bool m_any_degen;
	};

	static cem_encode_ldr_rgb_or_rgba_direct_result cem_encode_ldr_rgb_or_rgba_direct(
		uint32_t cem_index, uint32_t endpoint_ise_range, const color_rgba& l, const color_rgba& h, uint8_t* pEndpoint_vals,
		bool try_blue_contract)
	{
		assert((cem_index == astc_helpers::CEM_LDR_RGB_DIRECT) || (cem_index == astc_helpers::CEM_LDR_RGBA_DIRECT));

		cem_encode_ldr_rgb_or_rgba_direct_result res;

		bool& endpoints_are_swapped = res.m_endpoints_are_swapped;
		bool& any_degen = res.m_any_degen;
		bool& is_blue_contracted = res.m_is_blue_contracted;

		assert((cem_index == astc_helpers::CEM_LDR_RGB_DIRECT) || (cem_index == astc_helpers::CEM_LDR_RGBA_DIRECT));

		const bool has_alpha = (cem_index == astc_helpers::CEM_LDR_RGBA_DIRECT);

		const auto& quant_tab = astc_helpers::g_dequant_tables.get_endpoint_tab(endpoint_ise_range).m_val_to_ise;
		const auto& dequant_tab = astc_helpers::g_dequant_tables.get_endpoint_tab(endpoint_ise_range).m_ISE_to_val;

		//const auto &ISE_to_rank = astc_helpers::g_dequant_tables.get_endpoint_tab(endpoint_ise_range).m_ISE_to_rank;
		//const auto &rank_to_ISE = astc_helpers::g_dequant_tables.get_endpoint_tab(endpoint_ise_range).m_rank_to_ISE;

		color_rgba enc_l(l), enc_h(h);
		endpoints_are_swapped = false;

		is_blue_contracted = false;
		if (try_blue_contract)
		{
			int enc_v4 = quant_tab[enc_l.b], enc_v5 = quant_tab[enc_h.b];
			int dec_v4 = dequant_tab[enc_v4], dec_v5 = dequant_tab[enc_v5];

			bool did_clamp = false;
			enc_l = blue_contract_enc(h, did_clamp, dec_v5); // yes, they're swapped in the spec
			enc_h = blue_contract_enc(l, did_clamp, dec_v4);

			if (!did_clamp)
			{
				is_blue_contracted = true;
				endpoints_are_swapped = true;
			}
			else
			{
				enc_l = l;
				enc_h = h;
			}
		}

		int enc_v0 = quant_tab[enc_l.r], enc_v2 = quant_tab[enc_l.g], enc_v4 = quant_tab[enc_l.b];
		int enc_v1 = quant_tab[enc_h.r], enc_v3 = quant_tab[enc_h.g], enc_v5 = quant_tab[enc_h.b];

		int enc_v6 = 0, enc_v7 = 0;
		if (has_alpha)
		{
			enc_v6 = quant_tab[enc_l.a];
			enc_v7 = quant_tab[enc_h.a];
		}

		any_degen = false;
		if ((enc_v0 == enc_v1) && (l.r != h.r))
			any_degen = true;
		if ((enc_v2 == enc_v3) && (l.g != h.g))
			any_degen = true;
		if ((enc_v4 == enc_v5) && (l.b != h.b))
			any_degen = true;
		if (has_alpha)
		{
			if ((enc_v6 == enc_v7) && (l.a != h.a))
				any_degen = true;
		}

		int dec_v0 = dequant_tab[enc_v0], dec_v2 = dequant_tab[enc_v2], dec_v4 = dequant_tab[enc_v4];
		int dec_v1 = dequant_tab[enc_v1], dec_v3 = dequant_tab[enc_v3], dec_v5 = dequant_tab[enc_v5];

		int s0 = dec_v0 + dec_v2 + dec_v4;
		int s1 = dec_v1 + dec_v3 + dec_v5;

		bool should_swap = false;

		if ((s1 == s0) && (is_blue_contracted))
		{
			// if sums are equal we can't use blue contraction at all, so undo it
			enc_l = l;
			enc_h = h;

			is_blue_contracted = false;
			endpoints_are_swapped = false;

			enc_v0 = quant_tab[enc_l.r], enc_v2 = quant_tab[enc_l.g], enc_v4 = quant_tab[enc_l.b];
			enc_v1 = quant_tab[enc_h.r], enc_v3 = quant_tab[enc_h.g], enc_v5 = quant_tab[enc_h.b];

			dec_v0 = dequant_tab[enc_v0], dec_v2 = dequant_tab[enc_v2], dec_v4 = dequant_tab[enc_v4];
			dec_v1 = dequant_tab[enc_v1], dec_v3 = dequant_tab[enc_v3], dec_v5 = dequant_tab[enc_v5];

			if (has_alpha)
			{
				enc_v6 = quant_tab[enc_l.a];
				enc_v7 = quant_tab[enc_h.a];
			}

			s0 = dec_v0 + dec_v2 + dec_v4;
			s1 = dec_v1 + dec_v3 + dec_v5;
		}

		if (s1 >= s0)
		{
			if (is_blue_contracted)
				should_swap = true;
		}
		else
		{
			if (!is_blue_contracted)
				should_swap = true;
		}

		if (should_swap)
		{
			endpoints_are_swapped = !endpoints_are_swapped;

			std::swap(enc_v0, enc_v1);
			std::swap(enc_v2, enc_v3);
			std::swap(enc_v4, enc_v5);
			std::swap(enc_v6, enc_v7);
		}

		pEndpoint_vals[0] = (uint8_t)enc_v0;
		pEndpoint_vals[1] = (uint8_t)enc_v1;

		pEndpoint_vals[2] = (uint8_t)enc_v2;
		pEndpoint_vals[3] = (uint8_t)enc_v3;

		pEndpoint_vals[4] = (uint8_t)enc_v4;
		pEndpoint_vals[5] = (uint8_t)enc_v5;

		if (has_alpha)
		{
			pEndpoint_vals[6] = (uint8_t)enc_v6;
			pEndpoint_vals[7] = (uint8_t)enc_v7;
		}

	#ifdef _DEBUG
		{
			int check_s0 = dequant_tab[enc_v0] + dequant_tab[enc_v2] + dequant_tab[enc_v4];
			int check_s1 = dequant_tab[enc_v1] + dequant_tab[enc_v3] + dequant_tab[enc_v5];

			if (check_s1 >= check_s0)
			{
				assert(!is_blue_contracted);
			}
			else
			{
				assert(is_blue_contracted);
			}
		}
	#endif

		return res;
	}

	// Cannot fail
	// scale=1 cannot be packed
	static void cem_encode_ldr_rgb_or_rgba_base_scale(
		uint32_t cem_index, uint32_t endpoint_ise_range, float scale, float l_a, const vec4F& h, uint8_t* pEndpoint_vals)
	{
		assert((cem_index == astc_helpers::CEM_LDR_RGB_BASE_SCALE) || (cem_index == astc_helpers::CEM_LDR_RGB_BASE_SCALE_PLUS_TWO_A));
		assert((scale >= 0.0f) && (scale < 1.0f));

		const bool has_alpha = (cem_index == astc_helpers::CEM_LDR_RGB_BASE_SCALE_PLUS_TWO_A);

		const auto& quant_tab = astc_helpers::g_dequant_tables.get_endpoint_tab(endpoint_ise_range).m_val_to_ise;
		const auto& dequant_tab = astc_helpers::g_dequant_tables.get_endpoint_tab(endpoint_ise_range).m_ISE_to_val;

		const uint32_t total_vals_to_pack = has_alpha ? 6 : 4;

		float vals_to_pack[6] = { 0 };

		vals_to_pack[0] = h[0];
		vals_to_pack[1] = h[1];
		vals_to_pack[2] = h[2];
		vals_to_pack[3] = clamp(scale * (256.0f / 255.0f), 0.0f, 1.0f);

		if (has_alpha)
		{
			vals_to_pack[4] = l_a;
			vals_to_pack[5] = h[3];
		}

		for (uint32_t c = 0; c < total_vals_to_pack; c++)
		{
			const float v = vals_to_pack[c];
			const int iv = clamp((int)std::roundf(v * 255.0f), 0, 255);

			float best_err = BIG_FLOAT_VAL;
			for (int iscale_delta = -1; iscale_delta <= 1; iscale_delta++)
			{
				const int trial_ise_index = astc_helpers::apply_delta_to_bise_endpoint_val(endpoint_ise_range, quant_tab[iv], iscale_delta);

				const float dequant_val = dequant_tab[trial_ise_index] * (1.0f / 255.0f);

				const float dequant_err = fabs(dequant_val - v);
				if (dequant_err < best_err)
				{
					best_err = dequant_err;
					pEndpoint_vals[c] = (uint8_t)trial_ise_index;
				}
			} // iscale_delta

		} // c
	}

#if 0
	static int clamp6(int val, bool& was_clamped)
	{
		if (val < -32)
		{
			val = -32;
			was_clamped = true;
		}
		else if (val > 31)
		{
			val = 31;
			was_clamped = true;
		}
		return val;
	}
#endif
		
	// returns true if blue contraction was used
	// note the encoded endpoints may be swapped
	struct rgb_base_offset_res
	{
		bool m_failed_flag;
		bool m_used_blue_contraction;
		bool m_blue_contraction_clamped;
		bool m_delta_clamped;
		bool m_any_degen;
		bool m_endpoints_swapped;
	};

	// May fail if the tiebreaking logic isn't strong enough.
	static rgb_base_offset_res cem_encode_ldr_rgb_or_rgba_base_offset(uint32_t cem_index, uint32_t endpoint_ise_range, const color_rgba& orig_l, const color_rgba& orig_h, uint8_t* pEndpoint_vals, bool use_blue_contract)
	{
		assert((cem_index == astc_helpers::CEM_LDR_RGB_BASE_PLUS_OFFSET) || (cem_index == astc_helpers::CEM_LDR_RGBA_BASE_PLUS_OFFSET));

		const bool has_alpha = (cem_index == astc_helpers::CEM_LDR_RGBA_BASE_PLUS_OFFSET);

		rgb_base_offset_res res;
		res.m_failed_flag = false;
		res.m_used_blue_contraction = false;
		res.m_blue_contraction_clamped = false;
		res.m_delta_clamped = false;
		res.m_any_degen = false;
		res.m_endpoints_swapped = false;

		bool blue_contraction_clamped = false;

		bool status = basist::astc_ldr_t::pack_base_offset(
			cem_index, endpoint_ise_range, pEndpoint_vals,
			convert_to_basist_color_rgba(orig_l), convert_to_basist_color_rgba(orig_h),
			use_blue_contract, true,
			blue_contraction_clamped, res.m_delta_clamped, res.m_endpoints_swapped);
		
		assert(status);

		if (!status)
		{
			res.m_failed_flag = true;
			return res;
		}
		
		// Verify the actual BC status by unpacking to be absolutely sure
		res.m_used_blue_contraction = astc_helpers::used_blue_contraction(cem_index, pEndpoint_vals, endpoint_ise_range);

		color_rgba dec_l, dec_h;
		astc_ldr::decode_endpoints(cem_index, pEndpoint_vals, endpoint_ise_range, dec_l, dec_h);
				
		const uint32_t num_comps = (has_alpha ? 4 : 3);
		for (uint32_t c = 0; c < num_comps; c++)
		{
			if (orig_l[c] != orig_h[c])
				continue;

			// Desired L/H are not equal, but packed are equal=degenerate pack (loss of freedom).
			if (dec_l[c] == dec_h[c])
			{
				res.m_any_degen = true;
				break;
			}
		} //  c

		return res;
	}

	// L or LA direct
	static void encode_cem0_4(uint32_t cem_index, float lum_l, float lum_h, float a_l, float a_h, uint32_t endpoint_ise_range, uint8_t* pEndpoints)
	{
		assert((cem_index == astc_helpers::CEM_LDR_LUM_DIRECT) || (cem_index == astc_helpers::CEM_LDR_LUM_ALPHA_DIRECT));

		const bool has_alpha = (cem_index == astc_helpers::CEM_LDR_LUM_ALPHA_DIRECT);

		pEndpoints[0] = precise_round_bise_endpoint_val(lum_l, endpoint_ise_range);
		pEndpoints[1] = precise_round_bise_endpoint_val(lum_h, endpoint_ise_range);

		if (has_alpha)
		{
			pEndpoints[2] = precise_round_bise_endpoint_val(a_l, endpoint_ise_range);
			pEndpoints[3] = precise_round_bise_endpoint_val(a_h, endpoint_ise_range);
		}
	}

	// Returned in ISE order
	uint32_t get_colors(const color_rgba& l, const color_rgba& h, uint32_t weight_ise_index, color_rgba* pColors, bool decode_mode_srgb)
	{
		const uint32_t total_weights = astc_helpers::get_ise_levels(weight_ise_index);

		for (uint32_t i = 0; i < total_weights; i++)
		{
			uint32_t w = basisu::g_ise_weight_lerps[weight_ise_index][1 + i];

			for (uint32_t c = 0; c < 4; c++)
			{
				int le = l[c], he = h[c];

				// TODO: Investigate alpha handling here vs. latest spec.
				// https://raw.githubusercontent.com/KhronosGroup/DataFormat/refs/heads/main/astc.txt
				// The safest thing to do may be to assume non-sRGB in the encoder. I don't know yet.
				// How should alpha be handled here for lowest divergence from actual ASTC decoding hardware?
				if (decode_mode_srgb)
				{
					le = (le << 8) | 0x80;
					he = (he << 8) | 0x80;
				}
				else
				{
					le = (le << 8) | le;
					he = (he << 8) | he;
				}

				uint32_t k = astc_helpers::weight_interpolate(le, he, w);

				// See https://registry.khronos.org/OpenGL/extensions/EXT/EXT_texture_compression_astc_decode_mode.txt 			
				// All channels including alpha >>8.
				pColors[i][c] = (uint8_t)(k >> 8);
			} // c
		} // i 

		return total_weights;
	}

	// Returns 65 colors (NOT just 64 - 0-64 weight levels, so 65).
	uint32_t get_colors_raw_weights(const color_rgba& l, const color_rgba& h, color_rgba* pColors, bool decode_mode_srgb)
	{
		for (uint32_t w = 0; w <= 64; w++)
		{
			for (uint32_t c = 0; c < 4; c++)
			{
				int le = l[c], he = h[c];

				// TODO: Investigate alpha handling here vs. latest spec.
				// https://raw.githubusercontent.com/KhronosGroup/DataFormat/refs/heads/main/astc.txt
				// The safest thing to do may be to assume non-sRGB in the encoder. I don't know yet.
				// How should alpha be handled here for lowest divergence from actual ASTC decoding hardware?
				if (decode_mode_srgb)
				{
					le = (le << 8) | 0x80;
					he = (he << 8) | 0x80;
				}
				else
				{
					le = (le << 8) | le;
					he = (he << 8) | he;
				}

				uint32_t k = astc_helpers::weight_interpolate(le, he, w);

				// See https://registry.khronos.org/OpenGL/extensions/EXT/EXT_texture_compression_astc_decode_mode.txt 			
				// All channels including alpha >>8.
				pColors[w][c] = (uint8_t)(k >> 8);

			} // c
		} // i 

		return ASTC_LDR_MAX_RAW_WEIGHTS;
	}

	// Assumes ise 20 (256 levels)
	void decode_endpoints_ise20(uint32_t cem_index, const uint8_t* pEndpoint_vals, color_rgba& l, color_rgba& h)
	{
		assert(astc_helpers::is_cem_ldr(cem_index));

		int ldr_endpoints[4][2];
		astc_helpers::decode_endpoint(cem_index, ldr_endpoints, pEndpoint_vals);

		for (uint32_t c = 0; c < 4; c++)
		{
			assert((ldr_endpoints[c][0] >= 0) && (ldr_endpoints[c][0] <= 255));
			assert((ldr_endpoints[c][1] >= 0) && (ldr_endpoints[c][1] <= 255));

			l[c] = (uint8_t)ldr_endpoints[c][0];
			h[c] = (uint8_t)ldr_endpoints[c][1];
		}
	}

	void decode_endpoints(uint32_t cem_index, const uint8_t* pEndpoint_vals, uint32_t endpoint_ise_index, color_rgba& l, color_rgba& h, float* pScale)
	{
		const uint32_t total_endpoint_vals = astc_helpers::get_num_cem_values(cem_index);

		const auto& endpoint_dequant_tab = astc_helpers::g_dequant_tables.get_endpoint_tab(endpoint_ise_index).m_ISE_to_val;

		uint8_t dequantized_endpoints[astc_helpers::MAX_CEM_ENDPOINT_VALS];
		for (uint32_t i = 0; i < total_endpoint_vals; i++)
			dequantized_endpoints[i] = endpoint_dequant_tab[pEndpoint_vals[i]];

		decode_endpoints_ise20(cem_index, dequantized_endpoints, l, h);

		if ((pScale) && ((cem_index == astc_helpers::CEM_LDR_RGB_BASE_SCALE) || (cem_index == astc_helpers::CEM_LDR_RGB_BASE_SCALE_PLUS_TWO_A)))
		{
			*pScale = (float)dequantized_endpoints[3] * (1.0f / 256.0f);
		}
	}

	uint32_t get_colors(uint32_t cem_index, const uint8_t* pEndpoint_vals, uint32_t endpoint_ise_index, uint32_t weight_ise_index, color_rgba* pColors, bool decode_mode_srgb)
	{
		color_rgba l, h;
		decode_endpoints(cem_index, pEndpoint_vals, endpoint_ise_index, l, h);

		return get_colors(l, h, weight_ise_index, pColors, decode_mode_srgb);
	}

	// Decodes 65 colors
	uint32_t get_colors_raw_weights(uint32_t cem_index, const uint8_t* pEndpoint_vals, uint32_t endpoint_ise_index, color_rgba* pColors, bool decode_mode_srgb)
	{
		color_rgba l, h;
		decode_endpoints(cem_index, pEndpoint_vals, endpoint_ise_index, l, h);

		return get_colors_raw_weights(l, h, pColors, decode_mode_srgb);
	}

#if 0
	static vec4F calc_incremental_pca_4D(uint32_t num_pixels, const vec4F* pPixels, const vec4F& mean_f)
	{
		vec4F mean_axis(0.0f);

		for (uint32_t i = 0; i < num_pixels; i++)
		{
			vec4F orig_color(pPixels[i]);

			vec4F color(orig_color - mean_f);

			vec4F a(color * color[0]);
			vec4F b(color * color[1]);
			vec4F c(color * color[2]);
			vec4F d(color * color[3]);
			vec4F n(i ? mean_axis : color);

			n.normalize_in_place();

			mean_axis[0] += a.dot(n);
			mean_axis[1] += b.dot(n);
			mean_axis[2] += c.dot(n);
			mean_axis[3] += d.dot(n);
		}

		if (mean_axis.norm() < 1e-5f)
			mean_axis = vec4F(1.0f, 1.0f, 1.0f, 1.0f);

		mean_axis.normalize_in_place();

		return mean_axis;
	}
#endif

	// TODO: Try two-step Lanczos iteration/Rayleigh–Ritz approximation in a 2-dimensional Krylov subspace method vs. power method.
	static vec4F calc_pca_4D(uint32_t num_pixels, const vec4F* pPixels, const vec4F& mean_f)
	{
		float m00 = 0, m01 = 0, m02 = 0, m03 = 0;
		float m11 = 0, m12 = 0, m13 = 0;
		float m22 = 0, m23 = 0;
		float m33 = 0;

		for (size_t i = 0; i < num_pixels; ++i)
		{
			const vec4F v(pPixels[i] - mean_f);

			m00 += v[0] * v[0]; m01 += v[0] * v[1]; m02 += v[0] * v[2]; m03 += v[0] * v[3];
			m11 += v[1] * v[1]; m12 += v[1] * v[2]; m13 += v[1] * v[3];
			m22 += v[2] * v[2]; m23 += v[2] * v[3];
			m33 += v[3] * v[3];
		}

		// TODO: Seed from channel variances
		vec4F v(.6f, .75f, .4f, .75f);

		const uint32_t NUM_POW_ITERS = 6; // must be even
		for (uint32_t i = 0; i < NUM_POW_ITERS; ++i)
		{
			vec4F w(
				m00 * v[0] + m01 * v[1] + m02 * v[2] + m03 * v[3],
				m01 * v[0] + m11 * v[1] + m12 * v[2] + m13 * v[3],
				m02 * v[0] + m12 * v[1] + m22 * v[2] + m23 * v[3],
				m03 * v[0] + m13 * v[1] + m23 * v[2] + m33 * v[3]
			);

			if (i & 1)
				w.normalize_in_place();
			v = w;
		}

		if (v.norm() < 1e-5f)
			v = vec4F(.5f, .5f, .5f, .5f);

		return v;
	}

	static vec4F calc_pca_3D(uint32_t num_pixels, const vec4F* pPixels, const vec4F& mean_f)
	{
		float cov[6] = { 0, 0, 0, 0, 0, 0 };

		for (uint32_t i = 0; i < num_pixels; i++)
		{
			const vec4F& v = pPixels[i];
			float r = v[0] - mean_f[0];
			float g = v[1] - mean_f[1];
			float b = v[2] - mean_f[2];
			cov[0] += r * r; cov[1] += r * g; cov[2] += r * b; cov[3] += g * g; cov[4] += g * b; cov[5] += b * b;
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
				r *= m; g *= m; b *= m;
			}

			xr = r; xg = g; xb = b;
		}

		float nrm = xr * xr + xg * xg + xb * xb;

		vec4F axis(0.57735027f, 0.57735027f, 0.57735027f, 0.0f);
		if (nrm > 1e-5f)
		{
			float inv_nrm = 1.0f / sqrtf(nrm);
			xr *= inv_nrm; xg *= inv_nrm; xb *= inv_nrm;
			axis.set(xr, xg, xb, 0);
		}

		return axis;
	}

	void pixel_stats_t::init(uint32_t num_pixels, const color_rgba* pPixels)
	{
		m_num_pixels = num_pixels;
		m_has_alpha = false;

		m_min.set(255, 255, 255, 255);
		m_max.set(0, 0, 0, 0);

		m_mean_f.clear();

		for (uint32_t i = 0; i < m_num_pixels; i++)
		{
			const color_rgba& px = pPixels[i];

			m_pixels[i] = px;

			m_pixels_f[i].set((float)px.r * (1.0f / 255.0f), (float)px.g * (1.0f / 255.0f), (float)px.b * (1.0f / 255.0f), (float)px.a * (1.0f / 255.0f));

			m_mean_f += m_pixels_f[i];

			m_min.r = basisu::minimum(m_min.r, px.r);
			m_min.g = basisu::minimum(m_min.g, px.g);
			m_min.b = basisu::minimum(m_min.b, px.b);
			m_min.a = basisu::minimum(m_min.a, px.a);

			m_max.r = basisu::maximum(m_max.r, px.r);
			m_max.g = basisu::maximum(m_max.g, px.g);
			m_max.b = basisu::maximum(m_max.b, px.b);
			m_max.a = basisu::maximum(m_max.a, px.a);
		}

		m_mean_f *= (1.0f / (float)m_num_pixels);
		m_mean_f.clamp(0.0f, 1.0f);

		m_min_f.set(m_min.r * (1.0f / 255.0f), m_min.g * (1.0f / 255.0f), m_min.b * (1.0f / 255.0f), m_min.a * (1.0f / 255.0f));
		m_max_f.set(m_max.r * (1.0f / 255.0f), m_max.g * (1.0f / 255.0f), m_max.b * (1.0f / 255.0f), m_max.a * (1.0f / 255.0f));

		m_has_alpha = (m_min.a < 255);

		// Mean and zero relative RGB (3D) PCA axes
		m_mean_rel_axis3 = calc_pca_3D(m_num_pixels, m_pixels_f, m_mean_f);
		m_zero_rel_axis3 = calc_pca_3D(m_num_pixels, m_pixels_f, vec4F(0.0f));

		// Mean and zero relative RGBA (4D) PCA axes
		m_mean_rel_axis4 = calc_pca_4D(m_num_pixels, m_pixels_f, m_mean_f);

		for (uint32_t c = 0; c < 4u; c++)
			m_rgba_stats[c].calc_simplified_with_range(m_num_pixels, &m_pixels_f[0][c], 4);
	}
			
	static inline uint32_t square_of_diff(int a, int b)
	{
		assert((a >= 0) && (a <= 255));
		assert((b >= 0) && (b <= 255));

		int d = a - b;
		return (uint32_t)(d * d);
	}

	uint64_t eval_solution(
		const pixel_stats_t& pixel_stats,
		uint32_t total_weights, const color_rgba* pWeight_colors,
		uint8_t* pWeight_vals, uint32_t weight_ise_index,
		const cem_encode_params& params)
	{
		BASISU_NOTE_UNUSED(weight_ise_index);
		assert((total_weights <= 32) || (total_weights == 65));

		uint64_t total_err = 0;

		if (params.m_pForced_weight_vals0)
		{
			for (uint32_t c = 0; c < pixel_stats.m_num_pixels; c++)
			{
				const color_rgba& px = pixel_stats.m_pixels[c];

				const uint32_t w = params.m_pForced_weight_vals0[c];
				assert(w < total_weights);

				uint32_t err =
					params.m_comp_weights[0] * square_of_diff(px.r, pWeight_colors[w].r) +
					params.m_comp_weights[1] * square_of_diff(px.g, pWeight_colors[w].g) +
					params.m_comp_weights[2] * square_of_diff(px.b, pWeight_colors[w].b) +
					params.m_comp_weights[3] * square_of_diff(px.a, pWeight_colors[w].a);

				total_err += err;
				
				pWeight_vals[c] = (uint8_t)w;
			}
		}
		else
		{
			for (uint32_t c = 0; c < pixel_stats.m_num_pixels; c++)
			{
				const color_rgba& px = pixel_stats.m_pixels[c];

				uint32_t best_err = UINT32_MAX;
				uint32_t best_sel = 0;

				for (uint32_t i = 0; i < total_weights; i++)
				{
					uint32_t err =
						params.m_comp_weights[0] * square_of_diff(px.r, pWeight_colors[i].r) +
						params.m_comp_weights[1] * square_of_diff(px.g, pWeight_colors[i].g) +
						params.m_comp_weights[2] * square_of_diff(px.b, pWeight_colors[i].b) +
						params.m_comp_weights[3] * square_of_diff(px.a, pWeight_colors[i].a);

					if (err < best_err)
					{
						best_err = err;
						best_sel = i;
					}
				}

				total_err += best_err;
				pWeight_vals[c] = (uint8_t)best_sel;
			}
		} // if (params.m_pForced_weight_vals0)

		return total_err;
	}

	// Evaluates against raw weights [0,64], or to ISE quantized weights, depending on weight_ise_index.
	uint64_t eval_solution(
		const pixel_stats_t& pixel_stats,
		uint32_t cem_index,
		const uint8_t* pEndpoint_vals, uint32_t endpoint_ise_index,
		uint8_t* pWeight_vals, uint32_t weight_ise_index,
		const cem_encode_params& params)
	{
		assert((weight_ise_index <= astc_helpers::BISE_32_LEVELS) || (weight_ise_index == astc_helpers::BISE_64_LEVELS));

		color_rgba weight_colors[ASTC_LDR_MAX_RAW_WEIGHTS];
		uint32_t num_weights;

		assert((weight_ise_index <= astc_helpers::BISE_32_LEVELS) || (weight_ise_index == astc_helpers::BISE_64_LEVELS));
		
		// 64 levels isn't valid ASTC. It's used for raw weight mode.
		if (weight_ise_index == astc_helpers::BISE_64_LEVELS)
			num_weights = get_colors_raw_weights(cem_index, pEndpoint_vals, endpoint_ise_index, weight_colors, params.m_decode_mode_srgb);
		else
			num_weights = get_colors(cem_index, pEndpoint_vals, endpoint_ise_index, weight_ise_index, weight_colors, params.m_decode_mode_srgb);

		assert(num_weights <= std::size(weight_colors));

		uint64_t trial_err = eval_solution(
			pixel_stats,
			num_weights, weight_colors,
			pWeight_vals, weight_ise_index,
			params);

		return trial_err;
	}

	// Evaluates against raw weights [0,64], or to ISE quantized weights, depending on weight_ise_index.
	uint64_t eval_solution_dp(
		uint32_t ccs_index,
		const pixel_stats_t& pixel_stats,
		uint32_t total_weights, const color_rgba* pWeight_colors,
		uint8_t* pWeight_vals0, uint8_t* pWeight_vals1, uint32_t weight_ise_index,
		const cem_encode_params& params)
	{
		BASISU_NOTE_UNUSED(weight_ise_index);

		assert((ccs_index >= 0) && (ccs_index <= 3));
		assert((total_weights <= 32) || (total_weights == 65));

		uint64_t total_err = 0;

		if (params.m_pForced_weight_vals0)
		{
			for (uint32_t c = 0; c < pixel_stats.m_num_pixels; c++)
			{
				const color_rgba& px = pixel_stats.m_pixels[c];

				const uint32_t w = params.m_pForced_weight_vals0[c];
				assert(w < total_weights);

				uint32_t err = 0;
				for (uint32_t o = 0; o < 4; o++)
					if (o != ccs_index)
						err += params.m_comp_weights[o] * square_of_diff(px[o], pWeight_colors[w][o]);

				total_err += err;

				pWeight_vals0[c] = (uint8_t)w;
			}
		}
		else
		{
			for (uint32_t c = 0; c < pixel_stats.m_num_pixels; c++)
			{
				const color_rgba& px = pixel_stats.m_pixels[c];

				uint32_t best_err = UINT32_MAX;
				uint32_t best_sel = 0;

				for (uint32_t i = 0; i < total_weights; i++)
				{
					uint32_t err = 0;
					for (uint32_t o = 0; o < 4; o++)
						if (o != ccs_index)
							err += params.m_comp_weights[o] * square_of_diff(px[o], pWeight_colors[i][o]);

					if (err < best_err)
					{
						best_err = err;
						best_sel = i;
					}
				}

				total_err += best_err;
				pWeight_vals0[c] = (uint8_t)best_sel;
			}
		}

		if (params.m_pForced_weight_vals1)
		{
			for (uint32_t c = 0; c < pixel_stats.m_num_pixels; c++)
			{
				const color_rgba& px = pixel_stats.m_pixels[c];

				const uint32_t w = params.m_pForced_weight_vals1[c];
				assert(w < total_weights);

				uint32_t err = square_of_diff(px[ccs_index], pWeight_colors[w][ccs_index]);

				total_err += err * params.m_comp_weights[ccs_index];
				pWeight_vals1[c] = (uint8_t)w;
			}
		}
		else
		{
			for (uint32_t c = 0; c < pixel_stats.m_num_pixels; c++)
			{
				const color_rgba& px = pixel_stats.m_pixels[c];

				uint32_t best_err = UINT32_MAX;
				uint32_t best_sel = 0;

				for (uint32_t i = 0; i < total_weights; i++)
				{
					uint32_t err = square_of_diff(px[ccs_index], pWeight_colors[i][ccs_index]);

					if (err < best_err)
					{
						best_err = err;
						best_sel = i;
					}
				}

				total_err += best_err * params.m_comp_weights[ccs_index];
				pWeight_vals1[c] = (uint8_t)best_sel;
			}
		}

		return total_err;
	}

	// Evaluates against raw weights [0,64], or to ISE quantized weights, depending on weight_ise_index.
	uint64_t eval_solution_dp(
		const pixel_stats_t& pixel_stats,
		uint32_t cem_index, uint32_t ccs_index,
		const uint8_t* pEndpoint_vals, uint32_t endpoint_ise_index,
		uint8_t* pWeight_vals0, uint8_t* pWeight_vals1, uint32_t weight_ise_index,
		const cem_encode_params& params)
	{
		assert((weight_ise_index <= astc_helpers::BISE_32_LEVELS) || (weight_ise_index == astc_helpers::BISE_64_LEVELS));

		color_rgba weight_colors[ASTC_LDR_MAX_RAW_WEIGHTS];
		uint32_t num_weights;
		
		// 64 levels isn't valid ASTC. It's used for raw weight mode.
		if (weight_ise_index == astc_helpers::BISE_64_LEVELS)
			num_weights = get_colors_raw_weights(cem_index, pEndpoint_vals, endpoint_ise_index, weight_colors, params.m_decode_mode_srgb);
		else
			num_weights = get_colors(cem_index, pEndpoint_vals, endpoint_ise_index, weight_ise_index, weight_colors, params.m_decode_mode_srgb);

		uint64_t trial_err = eval_solution_dp(
			ccs_index,
			pixel_stats,
			num_weights, weight_colors,
			pWeight_vals0, pWeight_vals1, weight_ise_index,
			params);

		return trial_err;
	}

	// Direct - refine ISE quantized endpoints from float endpoints
	static void refine_cem8_or_12_endpoints(uint32_t cem_index, uint32_t endpoint_ise_range, uint8_t* pTrial_endpoint_vals, const vec4F& low_color_f, const vec4F& high_color_f, bool endpoints_are_swapped)
	{
		assert((cem_index == astc_helpers::CEM_LDR_RGB_DIRECT) || (cem_index == astc_helpers::CEM_LDR_RGBA_DIRECT));

		if (endpoint_ise_range == astc_helpers::BISE_256_LEVELS)
			return;

		const uint32_t total_comps = (cem_index == astc_helpers::CEM_LDR_RGBA_DIRECT) ? 4 : 3;

		assert((cem_index == astc_helpers::CEM_LDR_RGB_DIRECT) || (cem_index == astc_helpers::CEM_LDR_RGBA_DIRECT));
		assert((endpoint_ise_range >= astc_helpers::FIRST_VALID_ENDPOINT_ISE_RANGE) && (endpoint_ise_range <= astc_helpers::LAST_VALID_ENDPOINT_ISE_RANGE));

		const uint32_t total_endpoint_vals = astc_helpers::get_num_cem_values(cem_index);
		const uint32_t num_endpoint_ise_levels = astc_helpers::get_ise_levels(endpoint_ise_range);

		const auto& endpoint_dequant_tab = astc_helpers::g_dequant_tables.get_endpoint_tab(endpoint_ise_range).m_ISE_to_val;

		const auto& ISE_to_rank = astc_helpers::g_dequant_tables.get_endpoint_tab(endpoint_ise_range).m_ISE_to_rank;
		const auto& rank_to_ISE = astc_helpers::g_dequant_tables.get_endpoint_tab(endpoint_ise_range).m_rank_to_ISE;

		const bool orig_used_blue_contraction = astc_helpers::cem8_or_12_used_blue_contraction(cem_index, pTrial_endpoint_vals, endpoint_ise_range);

		uint32_t first_comp = 0;

		uint8_t refined_endpoint_vals[astc_helpers::NUM_MODE12_ENDPOINTS];
		memcpy(refined_endpoint_vals, pTrial_endpoint_vals, total_endpoint_vals);

		if (orig_used_blue_contraction)
		{
			// TODO expensive: 2*3*9 = 54 tries
			for (uint32_t e = 0; e < 2; e++)
			{
				float best_err = BIG_FLOAT_VAL;
				uint8_t best_refined_endpoint_vals[3] = { 0, 0, 0 };

				for (int b_delta = -1; b_delta <= 1; b_delta++)
				{
					for (int k = 0; k < 9; k++)
					{
						const int r_delta = (k % 3) - 1;
						const int g_delta = (k / 3) - 1;

						const int comp_deltas[3] = { r_delta, g_delta, b_delta };

						uint8_t trial_refined_endpoint_vals[3] = { 0, 0, 0 };

						for (uint32_t c = 0; c < 3; c++)
						{
							const int enc_val = pTrial_endpoint_vals[c * 2 + e];

							const int orig_rank = ISE_to_rank[enc_val];

							const int v_delta = comp_deltas[c];
							const int new_rank = basisu::clamp<int>(orig_rank + v_delta, 0, (int)num_endpoint_ise_levels - 1);
							const int new_enc_ise_val = rank_to_ISE[new_rank];

							trial_refined_endpoint_vals[c] = (uint8_t)new_enc_ise_val;

						} // c

						color_rgba trial_refined_endpoints_dequant(blue_contract_dec(endpoint_dequant_tab[trial_refined_endpoint_vals[0]], endpoint_dequant_tab[trial_refined_endpoint_vals[1]], endpoint_dequant_tab[trial_refined_endpoint_vals[2]], 255));

						vec3F trial_refined_endpoints_dequant_f(0.0f);
						for (uint32_t c = 0; c < 3; c++)
							trial_refined_endpoints_dequant_f[c] = (float)trial_refined_endpoints_dequant[c] * (1.0f / 255.0f);

						vec3F desired_endpoint;
						if (endpoints_are_swapped)
							desired_endpoint = (e == 0) ? vec3F(high_color_f) : vec3F(low_color_f);
						else
							desired_endpoint = (e == 0) ? vec3F(low_color_f) : vec3F(high_color_f);

						float trial_err = desired_endpoint.squared_distance(trial_refined_endpoints_dequant_f);
						if (trial_err < best_err)
						{
							best_err = trial_err;
							memcpy(best_refined_endpoint_vals, trial_refined_endpoint_vals, 3);
						}

					} // k

				} // b_delta

				for (uint32_t c = 0; c < 3; c++)
				{
					refined_endpoint_vals[c * 2 + e] = best_refined_endpoint_vals[c];
				} // c

			} // e

			// just refine A now (if it exists)
			first_comp = 3;
		}

		if (first_comp < total_comps)
		{
			for (uint32_t e = 0; e < 2; e++)
			{
				for (uint32_t c = first_comp; c < total_comps; c++)
				{
					const uint32_t idx = c * 2 + e;
					const int enc_val = pTrial_endpoint_vals[idx];

					const int orig_rank = ISE_to_rank[enc_val];

					int best_rank = orig_rank;
					float best_err = BIG_FLOAT_VAL;
					for (int v_delta = -1; v_delta <= 1; v_delta++)
					{
						int new_rank = basisu::clamp<int>(orig_rank + v_delta, 0, (int)num_endpoint_ise_levels - 1);
						int new_enc_ise_val = rank_to_ISE[new_rank];

						float dequant_val = (float)endpoint_dequant_tab[new_enc_ise_val] * (1.0f / 255.0f);

						float orig_val;
						if (endpoints_are_swapped)
							orig_val = (e == 0) ? high_color_f[c] : low_color_f[c];
						else
							orig_val = (e == 0) ? low_color_f[c] : high_color_f[c];

						float err = fabsf(dequant_val - orig_val);
						if (err < best_err)
						{
							best_err = err;
							best_rank = new_rank;
						}
					}

					refined_endpoint_vals[idx] = (uint8_t)rank_to_ISE[best_rank];

				} // c
			} // e
		}

		bool refined_used_blue_contraction = astc_helpers::cem8_or_12_used_blue_contraction(cem_index, refined_endpoint_vals, endpoint_ise_range);
		if (refined_used_blue_contraction == orig_used_blue_contraction)
		{
			memcpy(pTrial_endpoint_vals, refined_endpoint_vals, total_endpoint_vals);
		}
	}

	// Direct L/LA, single plane
	static bool try_cem0_or_4(uint32_t cem_index,
		const pixel_stats_t& pixel_stats, const cem_encode_params& enc_params,
		uint32_t endpoint_ise_range, uint32_t weight_ise_range,
		float lum_l, float lum_h, float a_l, float a_h,
		uint8_t* pTrial_endpoint_vals, uint8_t* pTrial_weight_vals, uint64_t& trial_blk_error)
	{
		assert(g_initialized);
		assert((cem_index == astc_helpers::CEM_LDR_LUM_DIRECT) || (cem_index == astc_helpers::CEM_LDR_LUM_ALPHA_DIRECT));

		const bool cem_has_alpha = (cem_index == astc_helpers::CEM_LDR_LUM_ALPHA_DIRECT);

		const uint32_t num_endpoint_vals = astc_helpers::get_num_cem_values(cem_index);

		uint8_t trial_endpoint_vals[astc_helpers::NUM_MODE4_ENDPOINTS] = { 0 };
		uint8_t trial_weight_vals[ASTC_LDR_MAX_BLOCK_PIXELS] = { 0 };

		encode_cem0_4(cem_index, lum_l, lum_h, a_l, a_h, endpoint_ise_range, trial_endpoint_vals);

		uint64_t trial_err = eval_solution(
			pixel_stats,
			cem_index, trial_endpoint_vals, endpoint_ise_range,
			trial_weight_vals, weight_ise_range,
			enc_params);

		bool improved_flag = false;
		if (trial_err < trial_blk_error)
		{
			trial_blk_error = trial_err;
			memcpy(pTrial_endpoint_vals, trial_endpoint_vals, astc_helpers::get_num_cem_values(cem_index));
			memcpy(pTrial_weight_vals, trial_weight_vals, pixel_stats.m_num_pixels);
			improved_flag = true;
		}

		bool any_degen = false;
		if ((trial_endpoint_vals[0] == trial_endpoint_vals[1]) && (lum_l != lum_h))
			any_degen = true;

		if (cem_has_alpha)
		{
			if ((trial_endpoint_vals[2] == trial_endpoint_vals[3]) && (a_l != a_h))
				any_degen = true;
		}

		if (any_degen)
		{
			const int l_delta = (lum_l < lum_h) ? -1 : 1;
			const int a_delta = (a_l < a_h) ? -1 : 1;

			for (uint32_t t = 1; t <= 3; t++)
			{
				uint8_t fixed_endpoint_vals[astc_helpers::NUM_MODE4_ENDPOINTS];
				memcpy(fixed_endpoint_vals, trial_endpoint_vals, num_endpoint_vals);

				if (t & 1)
				{
					if ((trial_endpoint_vals[0] == trial_endpoint_vals[1]) && (lum_l != lum_h))
						fixed_endpoint_vals[0] = (uint8_t)astc_helpers::apply_delta_to_bise_endpoint_val(endpoint_ise_range, trial_endpoint_vals[0], l_delta);

					if (cem_has_alpha)
					{
						if ((trial_endpoint_vals[2] == trial_endpoint_vals[3]) && (a_l != a_h))
							fixed_endpoint_vals[2] = (uint8_t)astc_helpers::apply_delta_to_bise_endpoint_val(endpoint_ise_range, trial_endpoint_vals[2], a_delta);
					}
				}

				if (t & 2)
				{
					if ((trial_endpoint_vals[0] == trial_endpoint_vals[1]) && (lum_l != lum_h))
						fixed_endpoint_vals[1] = (uint8_t)astc_helpers::apply_delta_to_bise_endpoint_val(endpoint_ise_range, trial_endpoint_vals[1], -l_delta);

					if (cem_has_alpha)
					{
						if ((trial_endpoint_vals[2] == trial_endpoint_vals[3]) && (a_l != a_h))
							fixed_endpoint_vals[3] = (uint8_t)astc_helpers::apply_delta_to_bise_endpoint_val(endpoint_ise_range, trial_endpoint_vals[3], -a_delta);
					}
				}

				trial_err = eval_solution(
					pixel_stats,
					cem_index, fixed_endpoint_vals, endpoint_ise_range,
					trial_weight_vals, weight_ise_range,
					enc_params);

				if (trial_err < trial_blk_error)
				{
					trial_blk_error = trial_err;
					memcpy(pTrial_endpoint_vals, fixed_endpoint_vals, astc_helpers::get_num_cem_values(cem_index));
					memcpy(pTrial_weight_vals, trial_weight_vals, pixel_stats.m_num_pixels);
					improved_flag = true;
				}

			} // t
		}

		return improved_flag;
	}

	static bool try_cem4_dp_a(uint32_t cem_index,
		const pixel_stats_t& pixel_stats, const cem_encode_params& enc_params,
		uint32_t endpoint_ise_range, uint32_t weight_ise_range,
		float lum_l, float lum_h, float a_l, float a_h,
		uint8_t* pTrial_endpoint_vals, uint8_t* pTrial_weight_vals0, uint8_t* pTrial_weight_vals1, uint64_t& trial_blk_error)
	{
		assert(g_initialized);
		assert(cem_index == astc_helpers::CEM_LDR_LUM_ALPHA_DIRECT);

		const bool cem_has_alpha = (cem_index == astc_helpers::CEM_LDR_LUM_ALPHA_DIRECT);

		const uint32_t num_endpoint_vals = astc_helpers::get_num_cem_values(cem_index);

		uint8_t trial_endpoint_vals[astc_helpers::NUM_MODE4_ENDPOINTS] = { 0 };
		uint8_t trial_weight_vals0[ASTC_LDR_MAX_BLOCK_PIXELS] = { 0 };
		uint8_t trial_weight_vals1[ASTC_LDR_MAX_BLOCK_PIXELS] = { 0 };

		encode_cem0_4(cem_index, lum_l, lum_h, a_l, a_h, endpoint_ise_range, trial_endpoint_vals);

		uint64_t trial_err = eval_solution_dp(
			pixel_stats, cem_index, 3,
			trial_endpoint_vals, endpoint_ise_range,
			trial_weight_vals0, trial_weight_vals1, weight_ise_range,
			enc_params);

		bool improved_flag = false;
		if (trial_err < trial_blk_error)
		{
			trial_blk_error = trial_err;
			memcpy(pTrial_endpoint_vals, trial_endpoint_vals, astc_helpers::get_num_cem_values(cem_index));
			memcpy(pTrial_weight_vals0, trial_weight_vals0, pixel_stats.m_num_pixels);
			memcpy(pTrial_weight_vals1, trial_weight_vals1, pixel_stats.m_num_pixels);
			improved_flag = true;
		}

		bool any_degen = false;
		if ((trial_endpoint_vals[0] == trial_endpoint_vals[1]) && (lum_l != lum_h))
			any_degen = true;

		if (cem_has_alpha)
		{
			if ((trial_endpoint_vals[2] == trial_endpoint_vals[3]) && (a_l != a_h))
				any_degen = true;
		}

		if (any_degen)
		{
			const int l_delta = (lum_l < lum_h) ? -1 : 1;
			const int a_delta = (a_l < a_h) ? -1 : 1;

			for (uint32_t t = 1; t <= 3; t++)
			{
				uint8_t fixed_endpoint_vals[astc_helpers::NUM_MODE4_ENDPOINTS];
				memcpy(fixed_endpoint_vals, trial_endpoint_vals, num_endpoint_vals);

				if (t & 1)
				{
					if ((trial_endpoint_vals[0] == trial_endpoint_vals[1]) && (lum_l != lum_h))
						fixed_endpoint_vals[0] = (uint8_t)astc_helpers::apply_delta_to_bise_endpoint_val(endpoint_ise_range, trial_endpoint_vals[0], l_delta);

					if (cem_has_alpha)
					{
						if ((trial_endpoint_vals[2] == trial_endpoint_vals[3]) && (a_l != a_h))
							fixed_endpoint_vals[2] = (uint8_t)astc_helpers::apply_delta_to_bise_endpoint_val(endpoint_ise_range, trial_endpoint_vals[2], a_delta);
					}
				}

				if (t & 2)
				{
					if ((trial_endpoint_vals[0] == trial_endpoint_vals[1]) && (lum_l != lum_h))
						fixed_endpoint_vals[1] = (uint8_t)astc_helpers::apply_delta_to_bise_endpoint_val(endpoint_ise_range, trial_endpoint_vals[1], -l_delta);

					if (cem_has_alpha)
					{
						if ((trial_endpoint_vals[2] == trial_endpoint_vals[3]) && (a_l != a_h))
							fixed_endpoint_vals[3] = (uint8_t)astc_helpers::apply_delta_to_bise_endpoint_val(endpoint_ise_range, trial_endpoint_vals[3], -a_delta);
					}
				}

				trial_err = eval_solution_dp(
					pixel_stats, cem_index, 3,
					fixed_endpoint_vals, endpoint_ise_range,
					trial_weight_vals0, trial_weight_vals1, weight_ise_range,
					enc_params);

				if (trial_err < trial_blk_error)
				{
					trial_blk_error = trial_err;
					memcpy(pTrial_endpoint_vals, fixed_endpoint_vals, astc_helpers::get_num_cem_values(cem_index));
					memcpy(pTrial_weight_vals0, trial_weight_vals0, pixel_stats.m_num_pixels);
					memcpy(pTrial_weight_vals1, trial_weight_vals1, pixel_stats.m_num_pixels);
					improved_flag = true;
				}

			} // t
		}

		return improved_flag;
	}

	// Direct RGB/RGBA
	// Cannot fail, but may have to fall back to non-blue-contracted
	// Returns false if trial solution not improved
	static bool try_cem8_12(
		uint32_t cem_index,
		const pixel_stats_t& pixel_stats, const cem_encode_params& enc_params,
		uint32_t endpoint_ise_range, uint32_t weight_ise_range,
		const vec4F& low_color_f, const vec4F& high_color_f,
		uint8_t* pTrial_endpoint_vals, uint8_t* pTrial_weight_vals, uint64_t& trial_blk_error, bool& trial_used_blue_contraction,
		bool try_blue_contract, bool& tried_used_blue_contraction)
	{
		assert(g_initialized);
		assert((cem_index == astc_helpers::CEM_LDR_RGB_DIRECT) || (cem_index == astc_helpers::CEM_LDR_RGBA_DIRECT));

		const uint32_t num_endpoint_vals = astc_helpers::get_num_cem_values(cem_index);
		const uint32_t num_comps = (cem_index == astc_helpers::CEM_LDR_RGB_DIRECT) ? 3 : 4;

		color_rgba low_color, high_color;
		for (uint32_t c = 0; c < 4; c++)
		{
			low_color[c] = (uint8_t)basisu::clamp<int>((int)std::round(low_color_f[c] * 255.0f), 0, 255);
			high_color[c] = (uint8_t)basisu::clamp<int>((int)std::round(high_color_f[c] * 255.0f), 0, 255);
		}

		uint8_t trial_endpoint_vals[astc_helpers::NUM_MODE12_ENDPOINTS] = { 0 };
		uint8_t trial_weight_vals[ASTC_LDR_MAX_BLOCK_PIXELS] = { 0 };

		// Cannot fail, but may have to fall back to non-blue-contracted
		cem_encode_ldr_rgb_or_rgba_direct_result res = cem_encode_ldr_rgb_or_rgba_direct(cem_index, endpoint_ise_range, low_color, high_color, trial_endpoint_vals, try_blue_contract);

		// Let caller know if we tried blue contraction
		tried_used_blue_contraction = res.m_is_blue_contracted;

		if (endpoint_ise_range < astc_helpers::BISE_256_LEVELS)
		{
			refine_cem8_or_12_endpoints(cem_index, endpoint_ise_range, trial_endpoint_vals, low_color_f, high_color_f, res.m_endpoints_are_swapped);
		}

		uint64_t trial_err = eval_solution(
			pixel_stats, cem_index, 
			trial_endpoint_vals, endpoint_ise_range, 
			trial_weight_vals, weight_ise_range,
			enc_params);

		bool improved_flag = false;
		if (trial_err < trial_blk_error)
		{
			trial_blk_error = trial_err;
			memcpy(pTrial_endpoint_vals, trial_endpoint_vals, astc_helpers::get_num_cem_values(cem_index));
			memcpy(pTrial_weight_vals, trial_weight_vals, pixel_stats.m_num_pixels);
			trial_used_blue_contraction = res.m_is_blue_contracted;
			improved_flag = true;
		}

		if (res.m_any_degen)
		{
			color_rgba dec_l(0), dec_h(0);
			decode_endpoints(cem_index, trial_endpoint_vals, endpoint_ise_range, dec_l, dec_h);

			uint32_t s0 = dec_l.r + dec_l.g + dec_l.b + dec_l.a;
			uint32_t s1 = dec_h.r + dec_h.g + dec_h.b + dec_h.a;
			if (astc_helpers::cem8_or_12_used_blue_contraction(cem_index, trial_endpoint_vals, endpoint_ise_range))
				std::swap(s0, s1);

			for (uint32_t t = 1; t <= 3; t++)
			{
				uint8_t fixed_endpoint_vals[astc_helpers::NUM_MODE12_ENDPOINTS];
				memcpy(fixed_endpoint_vals, trial_endpoint_vals, num_endpoint_vals);

				if (t & 1)
				{
					for (uint32_t c = 0; c < num_comps; c++)
					{
						uint32_t l_idx = c * 2 + 0;
						uint32_t h_idx = c * 2 + 1;

						if ((trial_endpoint_vals[l_idx] == trial_endpoint_vals[h_idx]) && (low_color[c] != high_color[c]))
						{
							int delta = (s0 <= s1) ? -1 : 1;

							fixed_endpoint_vals[l_idx] = (uint8_t)astc_helpers::apply_delta_to_bise_endpoint_val(endpoint_ise_range, trial_endpoint_vals[l_idx], delta);
						}
					}
				}

				if (t & 2)
				{
					for (uint32_t c = 0; c < num_comps; c++)
					{
						uint32_t l_idx = c * 2 + 0;
						uint32_t h_idx = c * 2 + 1;

						if ((trial_endpoint_vals[l_idx] == trial_endpoint_vals[h_idx]) && (low_color[c] != high_color[c]))
						{
							int delta = (s0 <= s1) ? 1 : -1;

							fixed_endpoint_vals[h_idx] = (uint8_t)astc_helpers::apply_delta_to_bise_endpoint_val(endpoint_ise_range, trial_endpoint_vals[h_idx], delta);
						}
					}
				}

				bool fixed_used_blue_contraction = astc_helpers::cem8_or_12_used_blue_contraction(cem_index, fixed_endpoint_vals, endpoint_ise_range);
				if (fixed_used_blue_contraction != res.m_is_blue_contracted)
					continue;
								
				trial_err = eval_solution(
					pixel_stats,
					cem_index, fixed_endpoint_vals, endpoint_ise_range,
					trial_weight_vals, weight_ise_range,
					enc_params);

				if (trial_err < trial_blk_error)
				{
					trial_blk_error = trial_err;
					memcpy(pTrial_endpoint_vals, fixed_endpoint_vals, astc_helpers::get_num_cem_values(cem_index));
					memcpy(pTrial_weight_vals, trial_weight_vals, pixel_stats.m_num_pixels);
					trial_used_blue_contraction = res.m_is_blue_contracted;
					improved_flag = true;
				}

			} // t

		} // if (res.m_any_degen)

		return improved_flag;
	}

	static bool try_cem8_12_dp(
		uint32_t cem_index, uint32_t ccs_index,
		const pixel_stats_t& pixel_stats, const cem_encode_params& enc_params,
		uint32_t endpoint_ise_range, uint32_t weight_ise_range,
		const vec4F& low_color_f, const vec4F& high_color_f,
		uint8_t* pTrial_endpoint_vals, uint8_t* pTrial_weight_vals0, uint8_t* pTrial_weight_vals1, uint64_t& trial_blk_error, bool& trial_used_blue_contraction,
		bool try_blue_contract, bool& tried_used_blue_contraction)
	{
		assert(g_initialized);
		assert((cem_index == astc_helpers::CEM_LDR_RGB_DIRECT) || (cem_index == astc_helpers::CEM_LDR_RGBA_DIRECT));

		bool improved_flag = false;

		const uint32_t num_endpoint_vals = astc_helpers::get_num_cem_values(cem_index);
		const uint32_t num_comps = (cem_index == astc_helpers::CEM_LDR_RGB_DIRECT) ? 3 : 4;

		color_rgba low_color, high_color;
		for (uint32_t c = 0; c < 4; c++)
		{
			low_color[c] = (uint8_t)basisu::clamp<int>((int)std::round(low_color_f[c] * 255.0f), 0, 255);
			high_color[c] = (uint8_t)basisu::clamp<int>((int)std::round(high_color_f[c] * 255.0f), 0, 255);
		}

		uint8_t trial_endpoint_vals[astc_helpers::NUM_MODE12_ENDPOINTS] = { 0 };
		uint8_t trial_weight_vals0[ASTC_LDR_MAX_BLOCK_PIXELS] = { 0 };
		uint8_t trial_weight_vals1[ASTC_LDR_MAX_BLOCK_PIXELS] = { 0 };

		// Cannot fail, but may have to fall back to non-blue-contracted
		cem_encode_ldr_rgb_or_rgba_direct_result res = cem_encode_ldr_rgb_or_rgba_direct(cem_index, endpoint_ise_range, low_color, high_color, trial_endpoint_vals, try_blue_contract);

		// Let caller know if we tried blue contraction
		tried_used_blue_contraction = res.m_is_blue_contracted;

		if (endpoint_ise_range < astc_helpers::BISE_256_LEVELS)
		{
			refine_cem8_or_12_endpoints(cem_index, endpoint_ise_range, trial_endpoint_vals, low_color_f, high_color_f, res.m_endpoints_are_swapped);
		}

		uint64_t trial_err = eval_solution_dp(pixel_stats, cem_index, ccs_index, trial_endpoint_vals, endpoint_ise_range, trial_weight_vals0, trial_weight_vals1, weight_ise_range, enc_params);

		if (trial_err < trial_blk_error)
		{
			trial_blk_error = trial_err;
			memcpy(pTrial_endpoint_vals, trial_endpoint_vals, astc_helpers::get_num_cem_values(cem_index));
			memcpy(pTrial_weight_vals0, trial_weight_vals0, pixel_stats.m_num_pixels);
			memcpy(pTrial_weight_vals1, trial_weight_vals1, pixel_stats.m_num_pixels);
			trial_used_blue_contraction = res.m_is_blue_contracted;
			improved_flag = true;
		}

		if (res.m_any_degen)
		{
			color_rgba dec_l(0), dec_h(0);
			decode_endpoints(cem_index, trial_endpoint_vals, endpoint_ise_range, dec_l, dec_h);

			uint32_t s0 = dec_l.r + dec_l.g + dec_l.b + dec_l.a;
			uint32_t s1 = dec_h.r + dec_h.g + dec_h.b + dec_h.a;
			if (astc_helpers::cem8_or_12_used_blue_contraction(cem_index, trial_endpoint_vals, endpoint_ise_range))
				std::swap(s0, s1);

			for (uint32_t t = 1; t <= 3; t++)
			{
				uint8_t fixed_endpoint_vals[astc_helpers::NUM_MODE12_ENDPOINTS];
				memcpy(fixed_endpoint_vals, trial_endpoint_vals, num_endpoint_vals);

				if (t & 1)
				{
					for (uint32_t c = 0; c < num_comps; c++)
					{
						uint32_t l_idx = c * 2 + 0;
						uint32_t h_idx = c * 2 + 1;

						if ((trial_endpoint_vals[l_idx] == trial_endpoint_vals[h_idx]) && (low_color[c] != high_color[c]))
						{
							int delta = (s0 <= s1) ? -1 : 1;

							fixed_endpoint_vals[l_idx] = (uint8_t)astc_helpers::apply_delta_to_bise_endpoint_val(endpoint_ise_range, trial_endpoint_vals[l_idx], delta);
						}
					}
				}

				if (t & 2)
				{
					for (uint32_t c = 0; c < num_comps; c++)
					{
						uint32_t l_idx = c * 2 + 0;
						uint32_t h_idx = c * 2 + 1;

						if ((trial_endpoint_vals[l_idx] == trial_endpoint_vals[h_idx]) && (low_color[c] != high_color[c]))
						{
							int delta = (s0 <= s1) ? 1 : -1;

							fixed_endpoint_vals[h_idx] = (uint8_t)astc_helpers::apply_delta_to_bise_endpoint_val(endpoint_ise_range, trial_endpoint_vals[h_idx], delta);
						}
					}
				}

				bool fixed_used_blue_contraction = astc_helpers::cem8_or_12_used_blue_contraction(cem_index, fixed_endpoint_vals, endpoint_ise_range);
				if (fixed_used_blue_contraction != res.m_is_blue_contracted)
					continue;

				trial_err = eval_solution_dp(pixel_stats, cem_index, ccs_index, fixed_endpoint_vals, endpoint_ise_range, trial_weight_vals0, trial_weight_vals1, weight_ise_range, enc_params);

				if (trial_err < trial_blk_error)
				{
					trial_blk_error = trial_err;
					memcpy(pTrial_endpoint_vals, fixed_endpoint_vals, astc_helpers::get_num_cem_values(cem_index));
					memcpy(pTrial_weight_vals0, trial_weight_vals0, pixel_stats.m_num_pixels);
					memcpy(pTrial_weight_vals1, trial_weight_vals1, pixel_stats.m_num_pixels);
					improved_flag = true;
				}

			} // t

		} // if (res.m_any_degen)

		return improved_flag;
	}

	// base+offset rgb/rgba, single or dual plane
	static bool try_cem9_13_sp_or_dp(
		uint32_t cem_index, int ccs_index,
		const pixel_stats_t& pixel_stats, const cem_encode_params& enc_params,
		uint32_t endpoint_ise_range, uint32_t weight_ise_range,
		const vec4F& low_color_f, const vec4F& high_color_f,
		uint8_t* pTrial_endpoint_vals, uint8_t* pTrial_weight_vals0, uint8_t* pTrial_weight_vals1, uint64_t& trial_blk_error, bool& trial_used_blue_contraction,
		bool try_blue_contract, bool& tried_used_blue_contraction, bool &tried_base_ofs_clamped)
	{
		assert(g_initialized);
		assert((cem_index == astc_helpers::CEM_LDR_RGB_BASE_PLUS_OFFSET) || (cem_index == astc_helpers::CEM_LDR_RGBA_BASE_PLUS_OFFSET));
		assert((ccs_index >= -1) && (ccs_index <= 3));
		assert((pixel_stats.m_num_pixels) && (pixel_stats.m_num_pixels <= ASTC_LDR_MAX_BLOCK_PIXELS));
		assert((endpoint_ise_range >= astc_helpers::FIRST_VALID_ENDPOINT_ISE_RANGE) && (endpoint_ise_range <= astc_helpers::LAST_VALID_ENDPOINT_ISE_RANGE));
		assert(((weight_ise_range >= astc_helpers::FIRST_VALID_WEIGHT_ISE_RANGE) && (weight_ise_range <= astc_helpers::LAST_VALID_WEIGHT_ISE_RANGE)) || (weight_ise_range == astc_helpers::BISE_64_LEVELS));

		assert(pTrial_weight_vals0);
		assert((ccs_index == -1) || (pTrial_weight_vals1));

		//const uint32_t num_endpoint_vals = astc_helpers::get_num_cem_values(cem_index);
		const uint32_t num_comps = (cem_index == astc_helpers::CEM_LDR_RGB_BASE_PLUS_OFFSET) ? 3 : 4;

		color_rgba low_color, high_color;
		for (uint32_t c = 0; c < 4; c++)
		{
			low_color[c] = (uint8_t)basisu::clamp<int>((int)std::round(low_color_f[c] * 255.0f), 0, 255);
			high_color[c] = (uint8_t)basisu::clamp<int>((int)std::round(high_color_f[c] * 255.0f), 0, 255);
		}

		uint8_t trial_endpoint_vals[astc_helpers::NUM_MODE13_ENDPOINTS] = { 0 };
		uint8_t trial_weight_vals0[ASTC_LDR_MAX_BLOCK_PIXELS] = { 0 };
		uint8_t trial_weight_vals1[ASTC_LDR_MAX_BLOCK_PIXELS] = { 0 };

		rgb_base_offset_res res = cem_encode_ldr_rgb_or_rgba_base_offset(cem_index, endpoint_ise_range, low_color, high_color, trial_endpoint_vals, try_blue_contract);

		tried_used_blue_contraction = res.m_used_blue_contraction;
		tried_base_ofs_clamped = res.m_delta_clamped;

		if (res.m_failed_flag)
			return false;

		bool improved_flag = false;

		if (ccs_index == -1)
		{
			uint64_t trial_err = eval_solution(
				pixel_stats,
				cem_index, trial_endpoint_vals, endpoint_ise_range,
				trial_weight_vals0, weight_ise_range,
				enc_params);

			if (trial_err < trial_blk_error)
			{
				trial_blk_error = trial_err;
				memcpy(pTrial_endpoint_vals, trial_endpoint_vals, astc_helpers::get_num_cem_values(cem_index));
				memcpy(pTrial_weight_vals0, trial_weight_vals0, pixel_stats.m_num_pixels);
				if (pTrial_weight_vals1)
					memset(pTrial_weight_vals1, 0, pixel_stats.m_num_pixels);
				trial_used_blue_contraction = res.m_used_blue_contraction;
				improved_flag = true;
			}
		}
		else
		{
			uint64_t trial_err = eval_solution_dp(
				pixel_stats,
				cem_index, ccs_index, trial_endpoint_vals, endpoint_ise_range,
				trial_weight_vals0, trial_weight_vals1, weight_ise_range,
				enc_params);

			if (trial_err < trial_blk_error)
			{
				trial_blk_error = trial_err;
				memcpy(pTrial_endpoint_vals, trial_endpoint_vals, astc_helpers::get_num_cem_values(cem_index));
				memcpy(pTrial_weight_vals0, trial_weight_vals0, pixel_stats.m_num_pixels);
				memcpy(pTrial_weight_vals1, trial_weight_vals1, pixel_stats.m_num_pixels);
				trial_used_blue_contraction = res.m_used_blue_contraction;
				improved_flag = true;
			}
		}

		if (res.m_any_degen)
		{
			color_rgba dec_l(0), dec_h(0);
			decode_endpoints(cem_index, trial_endpoint_vals, endpoint_ise_range, dec_l, dec_h);

			// The packing in these modes is so complex that we're going to approximate the biasing, and hope for the best.
			const uint32_t num_ise_levels = astc_helpers::get_ise_levels(endpoint_ise_range);
			int vals_per_ise_level = (256 + num_ise_levels - 1) / num_ise_levels;

			// TODO: There is potential cross-talk between RGB and A with the way this is done.
			for (uint32_t p = 1; p <= 3; p++)
			{
				color_rgba trial_low_color(low_color), trial_high_color(high_color);

				for (uint32_t c = 0; c < num_comps; c++)
				{
					if (low_color[c] == high_color[c])
						continue;

					if (dec_l[c] != dec_h[c])
						continue;

					int delta = (low_color[c] < high_color[c]) ? -1 : 1;
					if (p & 1)
						trial_low_color[c] = (uint8_t)basisu::clamp<int>((int)trial_low_color[c] + vals_per_ise_level * delta, 0, 255);

					if (p & 2)
						trial_high_color[c] = (uint8_t)basisu::clamp<int>((int)trial_high_color[c] + vals_per_ise_level * -delta, 0, 255);
				} // c

				res = cem_encode_ldr_rgb_or_rgba_base_offset(cem_index, endpoint_ise_range, trial_low_color, trial_high_color, trial_endpoint_vals, try_blue_contract);

				if (res.m_failed_flag)
					continue;
								
				if (ccs_index == -1)
				{
					uint64_t trial_err = eval_solution(
						pixel_stats,
						cem_index, trial_endpoint_vals, endpoint_ise_range,
						trial_weight_vals0, weight_ise_range,
						enc_params);

					if (trial_err < trial_blk_error)
					{
						trial_blk_error = trial_err;
						memcpy(pTrial_endpoint_vals, trial_endpoint_vals, astc_helpers::get_num_cem_values(cem_index));
						memcpy(pTrial_weight_vals0, trial_weight_vals0, pixel_stats.m_num_pixels);
						if (pTrial_weight_vals1)
							memset(pTrial_weight_vals1, 0, pixel_stats.m_num_pixels);
						trial_used_blue_contraction = res.m_used_blue_contraction;
						if (res.m_delta_clamped)
							tried_base_ofs_clamped = true;
						improved_flag = true;
					}
				}
				else
				{
					uint64_t trial_err = eval_solution_dp(
						pixel_stats,
						cem_index, ccs_index, trial_endpoint_vals, endpoint_ise_range,
						trial_weight_vals0, trial_weight_vals1, weight_ise_range,
						enc_params);

					if (trial_err < trial_blk_error)
					{
						trial_blk_error = trial_err;
						memcpy(pTrial_endpoint_vals, trial_endpoint_vals, astc_helpers::get_num_cem_values(cem_index));
						memcpy(pTrial_weight_vals0, trial_weight_vals0, pixel_stats.m_num_pixels);
						memcpy(pTrial_weight_vals1, trial_weight_vals1, pixel_stats.m_num_pixels);
						trial_used_blue_contraction = res.m_used_blue_contraction;
						if (res.m_delta_clamped)
							tried_base_ofs_clamped = true;
						improved_flag = true;
					}
				}

			} // p
		}
		else
		{
			// Now factor in the quantization introduced into the low (base) color, and apply this to the offset, for gain.
			color_rgba dec_l(0), dec_h(0);
			decode_endpoints(cem_index, trial_endpoint_vals, endpoint_ise_range, dec_l, dec_h);

			if (res.m_endpoints_swapped)
				dec_l = low_color;	// high color is the quantized base
			else
				dec_h = high_color; // low color is the quantized base

			res = cem_encode_ldr_rgb_or_rgba_base_offset(cem_index, endpoint_ise_range, dec_l, dec_h, trial_endpoint_vals, try_blue_contract);

			if (!res.m_failed_flag)
			{
				if (ccs_index == -1)
				{
					uint64_t trial_err = eval_solution(
						pixel_stats,
						cem_index, trial_endpoint_vals, endpoint_ise_range,
						trial_weight_vals0, weight_ise_range,
						enc_params);

					if (trial_err < trial_blk_error)
					{
						trial_blk_error = trial_err;
						memcpy(pTrial_endpoint_vals, trial_endpoint_vals, astc_helpers::get_num_cem_values(cem_index));
						memcpy(pTrial_weight_vals0, trial_weight_vals0, pixel_stats.m_num_pixels);
						if (pTrial_weight_vals1)
							memset(pTrial_weight_vals1, 0, pixel_stats.m_num_pixels);
						trial_used_blue_contraction = res.m_used_blue_contraction;
						if (res.m_delta_clamped)
							tried_base_ofs_clamped = true;
						improved_flag = true;
					}
				}
				else
				{
					uint64_t trial_err = eval_solution_dp(
						pixel_stats,
						cem_index, ccs_index, trial_endpoint_vals, endpoint_ise_range,
						trial_weight_vals0, trial_weight_vals1, weight_ise_range,
						enc_params);

					if (trial_err < trial_blk_error)
					{
						trial_blk_error = trial_err;
						memcpy(pTrial_endpoint_vals, trial_endpoint_vals, astc_helpers::get_num_cem_values(cem_index));
						memcpy(pTrial_weight_vals0, trial_weight_vals0, pixel_stats.m_num_pixels);
						memcpy(pTrial_weight_vals1, trial_weight_vals1, pixel_stats.m_num_pixels);
						trial_used_blue_contraction = res.m_used_blue_contraction;
						if (res.m_delta_clamped)
							tried_base_ofs_clamped = true;
						improved_flag = true;
					}
				}
			}
		}

		return improved_flag;
	}

	// l/la direct, single plane
	static uint64_t encode_cem0_4(
		uint32_t cem_index,
		const pixel_stats_t& pixel_stats, const cem_encode_params& enc_params,
		uint32_t endpoint_ise_range, uint32_t weight_ise_range,
		uint8_t* pEndpoint_vals, uint8_t* pWeight_vals, uint64_t cur_blk_error)
	{
		assert(g_initialized);
		assert((cem_index == astc_helpers::CEM_LDR_LUM_DIRECT) || (cem_index == astc_helpers::CEM_LDR_LUM_ALPHA_DIRECT));
		assert((pixel_stats.m_num_pixels) && (pixel_stats.m_num_pixels <= ASTC_LDR_MAX_BLOCK_PIXELS));
		assert((endpoint_ise_range >= astc_helpers::FIRST_VALID_ENDPOINT_ISE_RANGE) && (endpoint_ise_range <= astc_helpers::LAST_VALID_ENDPOINT_ISE_RANGE));
		assert(((weight_ise_range >= astc_helpers::FIRST_VALID_WEIGHT_ISE_RANGE) && (weight_ise_range <= astc_helpers::LAST_VALID_WEIGHT_ISE_RANGE)) || (weight_ise_range == astc_helpers::BISE_64_LEVELS));

		const bool cem_has_alpha = (cem_index == astc_helpers::CEM_LDR_LUM_ALPHA_DIRECT);

		const uint32_t total_endpoint_vals = astc_helpers::get_num_cem_values(cem_index);
		const uint32_t total_weights = pixel_stats.m_num_pixels;

		float lum_l = BIG_FLOAT_VAL, lum_h = -BIG_FLOAT_VAL;

		float pixel1F[ASTC_LDR_MAX_BLOCK_PIXELS];
		vec2F pixel2F[ASTC_LDR_MAX_BLOCK_PIXELS];

		for (uint32_t i = 0; i < pixel_stats.m_num_pixels; i++)
		{
			const vec4F& px = pixel_stats.m_pixels_f[i];

			float l = (px[0] + px[1] + px[2]) * (1.0f / 3.0f);

			pixel1F[i] = l;

			pixel2F[i][0] = l;
			pixel2F[i][1] = px[3];

			lum_l = minimum(lum_l, l);
			lum_h = maximum(lum_h, l);
		}

		const float a_l = pixel_stats.m_min_f[3];
		const float a_h = pixel_stats.m_max_f[3];

		const vec2F min_pixel2F(lum_l, a_l), max_pixel2F(lum_h, a_h);

		uint8_t trial_blk_endpoints[astc_helpers::MAX_CEM_ENDPOINT_VALS] = { 0 };
		uint8_t trial_blk_weights[ASTC_LDR_MAX_BLOCK_PIXELS] = { 0 };
		uint64_t trial_blk_error = UINT64_MAX;

		bool did_improve = try_cem0_or_4(
			cem_index, pixel_stats, enc_params,
			endpoint_ise_range, weight_ise_range,
			lum_l, lum_h, a_l, a_h,
			trial_blk_endpoints, trial_blk_weights, trial_blk_error);
		BASISU_NOTE_UNUSED(did_improve);

		if (trial_blk_error == UINT64_MAX)
			return cur_blk_error;

		if (trial_blk_error < cur_blk_error)
		{
			cur_blk_error = trial_blk_error;
			memcpy(pEndpoint_vals, trial_blk_endpoints, total_endpoint_vals);
			memcpy(pWeight_vals, trial_blk_weights, total_weights);
		}

		const uint32_t NUM_LS_OPT_PASSES = 3;

		for (uint32_t pass = 0; pass < NUM_LS_OPT_PASSES; pass++)
		{
			vec2F xl(lum_l, a_l), xh(lum_h, a_h);

			bool ls_res;
			if (cem_has_alpha)
			{
				ls_res = compute_least_squares_endpoints_2D(
					pixel_stats.m_num_pixels, trial_blk_weights, get_ls_weights_ise(weight_ise_range),
					&xl, &xh, pixel2F, min_pixel2F, max_pixel2F);

			}
			else
			{
				ls_res = compute_least_squares_endpoints_1D(
					pixel_stats.m_num_pixels, trial_blk_weights, get_ls_weights_ise(weight_ise_range),
					&xl[0], &xh[0], pixel1F, lum_l, lum_h);
			}
			if (!ls_res)
				break;

			bool did_improve_res = false;

			did_improve_res = try_cem0_or_4(
				cem_index, pixel_stats, enc_params,
				endpoint_ise_range, weight_ise_range,
				xl[0], xh[0], xl[1], xh[1],
				trial_blk_endpoints, trial_blk_weights, trial_blk_error);

			BASISU_NOTE_UNUSED(did_improve_res);

			if (trial_blk_error >= cur_blk_error)
				break;

			cur_blk_error = trial_blk_error;
			memcpy(pEndpoint_vals, trial_blk_endpoints, total_endpoint_vals);
			memcpy(pWeight_vals, trial_blk_weights, total_weights);

		} // pass

		return cur_blk_error;
	}

	// lum+alpha direct, dual plane
	static uint64_t encode_cem4_dp_a(
		uint32_t cem_index,
		const pixel_stats_t& pixel_stats, const cem_encode_params& enc_params,
		uint32_t endpoint_ise_range, uint32_t weight_ise_range,
		uint8_t* pEndpoint_vals, uint8_t* pWeight_vals0, uint8_t* pWeight_vals1, uint64_t cur_blk_error)
	{
		assert(g_initialized);
		assert(cem_index == astc_helpers::CEM_LDR_LUM_ALPHA_DIRECT);
		assert((pixel_stats.m_num_pixels) && (pixel_stats.m_num_pixels <= ASTC_LDR_MAX_BLOCK_PIXELS));
		assert((endpoint_ise_range >= astc_helpers::FIRST_VALID_ENDPOINT_ISE_RANGE) && (endpoint_ise_range <= astc_helpers::LAST_VALID_ENDPOINT_ISE_RANGE));
		assert(((weight_ise_range >= astc_helpers::FIRST_VALID_WEIGHT_ISE_RANGE) && (weight_ise_range <= astc_helpers::LAST_VALID_WEIGHT_ISE_RANGE)) || (weight_ise_range == astc_helpers::BISE_64_LEVELS));

		const uint32_t total_endpoint_vals = astc_helpers::get_num_cem_values(cem_index);
		const uint32_t total_weights = pixel_stats.m_num_pixels;

		float alpha_vals[ASTC_LDR_MAX_BLOCK_PIXELS];

		for (uint32_t i = 0; i < pixel_stats.m_num_pixels; i++)
		{
			const vec4F& px = pixel_stats.m_pixels_f[i];

			alpha_vals[i] = px[3];
		}

		// First get plane0's low/high (lum)
		uint8_t lum_endpoints[astc_helpers::MAX_CEM_ENDPOINT_VALS];
		uint8_t lum_weights0[ASTC_LDR_MAX_BLOCK_PIXELS];

		uint64_t lum_blk_error = encode_cem0_4(
			astc_helpers::CEM_LDR_LUM_DIRECT,
			pixel_stats, enc_params,
			endpoint_ise_range, weight_ise_range,
			lum_endpoints, lum_weights0, UINT64_MAX);

		if (lum_blk_error == UINT64_MAX)
			return cur_blk_error;

		const auto& dequant_endpoints_tab = astc_helpers::g_dequant_tables.get_endpoint_tab(endpoint_ise_range).m_ISE_to_val;

		float lum_l = (float)dequant_endpoints_tab[lum_endpoints[0]] * (1.0f / 255.0f);
		float lum_h = (float)dequant_endpoints_tab[lum_endpoints[1]] * (1.0f / 255.0f);
		float a_l = pixel_stats.m_min_f[3];
		float a_h = pixel_stats.m_max_f[3];

		uint8_t trial_endpoints[astc_helpers::MAX_CEM_ENDPOINT_VALS];
		uint8_t trial_weights0[ASTC_LDR_MAX_BLOCK_PIXELS];
		uint8_t trial_weights1[ASTC_LDR_MAX_BLOCK_PIXELS];
		uint64_t trial_blk_error = UINT64_MAX;

		bool did_improve = try_cem4_dp_a(
			cem_index, pixel_stats, enc_params,
			endpoint_ise_range, weight_ise_range,
			lum_l, lum_h, a_l, a_h,
			trial_endpoints, trial_weights0, trial_weights1, trial_blk_error);

		if (!did_improve)
		{
			assert(0);
			return cur_blk_error;
		}

		if (trial_blk_error < cur_blk_error)
		{
			cur_blk_error = trial_blk_error;
			memcpy(pEndpoint_vals, trial_endpoints, total_endpoint_vals);
			memcpy(pWeight_vals0, trial_weights0, total_weights);
			memcpy(pWeight_vals1, trial_weights1, total_weights);
		}

		const uint32_t NUM_LS_OPT_PASSES = 3;

		for (uint32_t pass = 0; pass < NUM_LS_OPT_PASSES; pass++)
		{
			float xl = pixel_stats.m_min_f[3], xh = pixel_stats.m_max_f[3];

			bool ls_res = compute_least_squares_endpoints_1D(
				pixel_stats.m_num_pixels, trial_weights1, get_ls_weights_ise(weight_ise_range),
				&xl, &xh, alpha_vals, pixel_stats.m_min_f[3], pixel_stats.m_max_f[3]);
			if (!ls_res)
				break;

			did_improve = try_cem4_dp_a(
				cem_index, pixel_stats, enc_params,
				endpoint_ise_range, weight_ise_range,
				lum_l, lum_h, xl, xh,
				trial_endpoints, trial_weights0, trial_weights1, trial_blk_error);

			if (!did_improve)
				break;

			cur_blk_error = trial_blk_error;
			memcpy(pEndpoint_vals, trial_endpoints, total_endpoint_vals);
			memcpy(pWeight_vals0, trial_weights0, total_weights);
			memcpy(pWeight_vals1, trial_weights1, total_weights);

		} // pass

		return cur_blk_error;
	}
		
	struct weight_refiner
	{
		void init(uint32_t weight_ise_range, uint32_t total_pixels, const uint8_t *pInitial_ise_weights)
		{
			m_weight_ise_range = weight_ise_range;
			m_total_pixels = total_pixels;
			m_pISE_to_rank = &astc_helpers::g_dequant_tables.get_weight_tab(weight_ise_range).m_ISE_to_rank;
			m_pRank_to_ise = &astc_helpers::g_dequant_tables.get_weight_tab(weight_ise_range).m_rank_to_ISE;
			m_num_weight_levels = astc_helpers::get_ise_levels(weight_ise_range);
						
			for (uint32_t i = 0; i < total_pixels; i++)
				m_start_weights[i] = (*m_pISE_to_rank)[pInitial_ise_weights[i]];

			m_min_weight = UINT32_MAX;
			m_max_weight = 0;
			m_sum_weight = 0;

			for (uint32_t i = 0; i < total_pixels; i++)
			{
				const uint32_t weight = m_start_weights[i];
				m_sum_weight += weight;
				m_min_weight = minimumu(m_min_weight, weight);
				m_max_weight = maximumu(m_max_weight, weight);
			}
		}

		void refine(uint32_t pass_index, uint8_t* pTrial_ise_weights)
		{
			switch (pass_index)
			{
			case 0:
			{
				for (uint32_t i = 0; i < m_total_pixels; i++)
				{
					uint32_t v = m_start_weights[i];
					if ((v == m_min_weight) && (v < (m_num_weight_levels - 1)))
						v++;

					pTrial_ise_weights[i] = (*m_pRank_to_ise)[v];
				}
				break;
			}
			case 1:
			{
				for (uint32_t i = 0; i < m_total_pixels; i++)
				{
					uint32_t v = m_start_weights[i];
					if ((v == m_max_weight) && (v > 0))
						v--;

					pTrial_ise_weights[i] = (*m_pRank_to_ise)[v];
				}
				break;
			}
			case 2:
			{
				for (uint32_t i = 0; i < m_total_pixels; i++)
				{
					uint32_t v = m_start_weights[i];
					if ((v == m_min_weight) && (v < (m_num_weight_levels - 1)))
						v++;
					else if ((v == m_max_weight) && (v > 0))
						v--;

					pTrial_ise_weights[i] = (*m_pRank_to_ise)[v];
				}
				break;
			}
			case 3:
			{
				const int max_weight_rank_index = m_num_weight_levels - 1;
				int ly = -1, hy = max_weight_rank_index + 1;

				for (uint32_t i = 0; i < m_total_pixels; i++)
				{
					int s = (int)clampf(floor((float)max_weight_rank_index * ((float)m_start_weights[i] - (float)ly) / ((float)hy - (float)ly) + .5f), 0, (float)max_weight_rank_index);
					pTrial_ise_weights[i] = (*m_pRank_to_ise)[s];
				}

				break;
			}
			case 4:
			{
				const int max_weight_rank_index = m_num_weight_levels - 1;
				int ly = -2, hy = max_weight_rank_index + 2;

				for (uint32_t i = 0; i < m_total_pixels; i++)
				{
					int s = (int)clampf(floor((float)max_weight_rank_index * ((float)m_start_weights[i] - (float)ly) / ((float)hy - (float)ly) + .5f), 0, (float)max_weight_rank_index);
					pTrial_ise_weights[i] = (*m_pRank_to_ise)[s];
				}

				break;
			}
			case 5:
			{
				const int max_weight_rank_index = m_num_weight_levels - 1;
				int ly = -1, hy = max_weight_rank_index + 2;

				for (uint32_t i = 0; i < m_total_pixels; i++)
				{
					int s = (int)clampf(floor((float)max_weight_rank_index * ((float)m_start_weights[i] - (float)ly) / ((float)hy - (float)ly) + .5f), 0, (float)max_weight_rank_index);
					pTrial_ise_weights[i] = (*m_pRank_to_ise)[s];
				}

				break;
			}
			case 6:
			{
				const int max_weight_rank_index = m_num_weight_levels - 1;
				int ly = -2, hy = max_weight_rank_index + 1;

				for (uint32_t i = 0; i < m_total_pixels; i++)
				{
					int s = (int)clampf(floor((float)max_weight_rank_index * ((float)m_start_weights[i] - (float)ly) / ((float)hy - (float)ly) + .5f), 0, (float)max_weight_rank_index);
					pTrial_ise_weights[i] = (*m_pRank_to_ise)[s];
				}

				break;
			}
			case 7:
			{
				for (uint32_t i = 0; i < m_total_pixels; i++)
				{
					uint32_t v = m_start_weights[i];
					if ((v == m_min_weight) && (v < (m_num_weight_levels - 1)))
					{
						v++;
						if (v < (m_num_weight_levels - 1))
							v++;
					}

					pTrial_ise_weights[i] = (*m_pRank_to_ise)[v];
				}
				break;

				break;
			}
			case 8:
			{
				for (uint32_t i = 0; i < m_total_pixels; i++)
				{
					uint32_t v = m_start_weights[i];
					if ((v == m_max_weight) && (v > 0))
					{
						v--;
						if (v > 0)
							v--;
					}

					pTrial_ise_weights[i] = (*m_pRank_to_ise)[v];
				}
				break;
			}
			case 9:
			{
				for (uint32_t i = 0; i < m_total_pixels; i++)
				{
					uint32_t v = m_start_weights[i];
					if ((v == m_min_weight) && (v < (m_num_weight_levels - 1)))
					{
						v++;
						if (v < (m_num_weight_levels - 1))
							v++;
					}
					else if ((v == m_max_weight) && (v > 0))
					{
						v--;
						if (v > 0)
							v--;
					}

					pTrial_ise_weights[i] = (*m_pRank_to_ise)[v];
				}
				break;
			}
			case 10:
			{
				float mid_weight = (float)m_sum_weight / (float)m_total_pixels;

				for (uint32_t i = 0; i < m_total_pixels; i++)
				{
					int v = m_start_weights[i];

					float fv = ((float)v - mid_weight) * .8f + ((float)m_num_weight_levels * .5f);

					v = clamp<int>((int)std::round(fv), 0, m_num_weight_levels - 1);

					pTrial_ise_weights[i] = (*m_pRank_to_ise)[v];
				}
				break;
			}
			case 11:
			{
				float mid_weight = (float)m_sum_weight / (float)m_total_pixels;

				for (uint32_t i = 0; i < m_total_pixels; i++)
				{
					int v = m_start_weights[i];

					float fv = ((float)v - mid_weight) * .9f + ((float)m_num_weight_levels * .5f);

					v = clamp<int>((int)std::round(fv), 0, m_num_weight_levels - 1);

					pTrial_ise_weights[i] = (*m_pRank_to_ise)[v];
				}
				break;
			}
			case 12:
			{
				float mid_weight = (float)m_sum_weight / (float)m_total_pixels;

				for (uint32_t i = 0; i < m_total_pixels; i++)
				{
					int v = m_start_weights[i];

					float fv = ((float)v - mid_weight) * 1.1f + ((float)m_num_weight_levels * .5f);

					v = clamp<int>((int)std::round(fv), 0, m_num_weight_levels - 1);

					pTrial_ise_weights[i] = (*m_pRank_to_ise)[v];
				}
				break;
			}
			case 13:
			{
				float mid_weight = (float)m_sum_weight / (float)m_total_pixels;

				for (uint32_t i = 0; i < m_total_pixels; i++)
				{
					int v = m_start_weights[i];

					float fv;
					if (v < mid_weight)
						fv = ((float)v - mid_weight) * .8f + ((float)m_num_weight_levels * .5f);
					else
						fv = (float)v;

					v = clamp<int>((int)std::round(fv), 0, m_num_weight_levels - 1);

					pTrial_ise_weights[i] = (*m_pRank_to_ise)[v];
				}
				break;
			}
			case 14:
			{
				float mid_weight = (float)m_sum_weight / (float)m_total_pixels;

				for (uint32_t i = 0; i < m_total_pixels; i++)
				{
					int v = m_start_weights[i];

					float fv;
					if (v >= mid_weight)
						fv = ((float)v - mid_weight) * .8f + ((float)m_num_weight_levels * .5f);
					else
						fv = (float)v;

					v = clamp<int>((int)std::round(fv), 0, m_num_weight_levels - 1);

					pTrial_ise_weights[i] = (*m_pRank_to_ise)[v];
				}
				break;
			}
			case 15:
			{
				for (uint32_t i = 0; i < m_total_pixels; i++)
				{
					uint32_t v = m_start_weights[i];
					if (v < (m_num_weight_levels - 1))
						v++;

					pTrial_ise_weights[i] = (*m_pRank_to_ise)[v];
				}
				break;
			}
			case 16:
			{
				for (uint32_t i = 0; i < m_total_pixels; i++)
				{
					uint32_t v = m_start_weights[i];
					if (v)
						v--;

					pTrial_ise_weights[i] = (*m_pRank_to_ise)[v];
				}
				break;
			}
			default:
			{
				assert(0);
				memset(pTrial_ise_weights, 0, m_total_pixels);
				break;
			}
			}
		}

		uint32_t m_total_pixels;
		uint32_t m_weight_ise_range;
		uint32_t m_num_weight_levels;
		uint8_t m_start_weights[ASTC_LDR_MAX_BLOCK_PIXELS]; // ranks, not ISE

		uint32_t m_min_weight, m_max_weight, m_sum_weight;

		const basisu::vector<uint8_t>* m_pISE_to_rank;
		const basisu::vector<uint8_t>* m_pRank_to_ise;
	};

	// rgb/rgba direct or rgb/rgba base+offset, single plane
	static uint64_t encode_cem8_12_9_13(
		uint32_t cem_index,
		const pixel_stats_t& pixel_stats, const cem_encode_params& enc_params,
		uint32_t endpoint_ise_range, uint32_t weight_ise_range,
		uint8_t* pEndpoint_vals, uint8_t* pWeight_vals, uint64_t cur_blk_error, bool use_blue_contraction, bool* pBase_ofs_clamped_flag)
	{
		assert(g_initialized);
		assert((cem_index == astc_helpers::CEM_LDR_RGB_DIRECT) || (cem_index == astc_helpers::CEM_LDR_RGBA_DIRECT) ||
			(cem_index == astc_helpers::CEM_LDR_RGB_BASE_PLUS_OFFSET) || (cem_index == astc_helpers::CEM_LDR_RGBA_BASE_PLUS_OFFSET));

		assert((pixel_stats.m_num_pixels) && (pixel_stats.m_num_pixels <= ASTC_LDR_MAX_BLOCK_PIXELS));
		assert((endpoint_ise_range >= astc_helpers::FIRST_VALID_ENDPOINT_ISE_RANGE) && (endpoint_ise_range <= astc_helpers::LAST_VALID_ENDPOINT_ISE_RANGE));
		assert(((weight_ise_range >= astc_helpers::FIRST_VALID_WEIGHT_ISE_RANGE) && (weight_ise_range <= astc_helpers::LAST_VALID_WEIGHT_ISE_RANGE)) || (weight_ise_range == astc_helpers::BISE_64_LEVELS));

		if (pBase_ofs_clamped_flag)
			*pBase_ofs_clamped_flag = false;

		const bool cem_has_alpha = (cem_index == astc_helpers::CEM_LDR_RGBA_DIRECT) || (cem_index == astc_helpers::CEM_LDR_RGBA_BASE_PLUS_OFFSET);
		const bool cem_is_base_offset = (cem_index == astc_helpers::CEM_LDR_RGB_BASE_PLUS_OFFSET) || (cem_index == astc_helpers::CEM_LDR_RGBA_BASE_PLUS_OFFSET);

		const uint32_t total_endpoint_vals = astc_helpers::get_num_cem_values(cem_index);
		const uint32_t total_weights = pixel_stats.m_num_pixels;

		float best_l = BIG_FLOAT_VAL, best_h = -BIG_FLOAT_VAL;
		//int best_l_index = 0, best_h_index = 0;

		for (uint32_t c = 0; c < pixel_stats.m_num_pixels; c++)
		{
			const vec4F px(pixel_stats.m_pixels_f[c] - pixel_stats.m_mean_f);

			float p = cem_has_alpha ? px.dot(pixel_stats.m_mean_rel_axis4) : px.dot3(pixel_stats.m_mean_rel_axis3);
			if (p < best_l)
			{
				best_l = p;
				//best_l_index = c;
			}

			if (p > best_h)
			{
				best_h = p;
				//best_h_index = c;
			}
		} // c

#if 0
		vec4F low_color_f(pixel_stats.m_pixels_f[best_l_index]), high_color_f(pixel_stats.m_pixels_f[best_h_index]);
#else
		vec4F low_color_f, high_color_f;
		if (cem_has_alpha)
		{
			low_color_f = pixel_stats.m_mean_rel_axis4 * best_l + pixel_stats.m_mean_f;
			high_color_f = pixel_stats.m_mean_rel_axis4 * best_h + pixel_stats.m_mean_f;
		}
		else
		{
			low_color_f = vec4F(pixel_stats.m_mean_rel_axis3) * best_l + pixel_stats.m_mean_f;
			high_color_f = vec4F(pixel_stats.m_mean_rel_axis3) * best_h + pixel_stats.m_mean_f;
		}

		low_color_f.clamp(0.0f, 1.0f);
		high_color_f.clamp(0.0f, 1.0f);
#endif

		uint8_t trial_blk_endpoints[astc_helpers::MAX_CEM_ENDPOINT_VALS] = { 0 };
		uint8_t trial_blk_weights[ASTC_LDR_MAX_BLOCK_PIXELS] = { 0 };
		uint64_t trial_blk_error = UINT64_MAX;
		bool trial_used_blue_contraction = false;

		bool tried_used_blue_contraction = false;
				
		if (cem_is_base_offset)
		{
			bool tried_base_ofs_clamped = false;

			try_cem9_13_sp_or_dp(
				cem_index, -1, pixel_stats, enc_params,
				endpoint_ise_range, weight_ise_range,
				low_color_f, high_color_f,
				trial_blk_endpoints, trial_blk_weights, nullptr, trial_blk_error, trial_used_blue_contraction, use_blue_contraction, 
				tried_used_blue_contraction, tried_base_ofs_clamped);

			if ((pBase_ofs_clamped_flag) && (tried_base_ofs_clamped))
				*pBase_ofs_clamped_flag = true;

			if (tried_used_blue_contraction)
			{
				try_cem9_13_sp_or_dp(
					cem_index, -1, pixel_stats, enc_params,
					endpoint_ise_range, weight_ise_range,
					low_color_f, high_color_f,
					trial_blk_endpoints, trial_blk_weights, nullptr, trial_blk_error, trial_used_blue_contraction, false, 
					tried_used_blue_contraction, tried_base_ofs_clamped);

				if ((pBase_ofs_clamped_flag) && (tried_base_ofs_clamped))
					*pBase_ofs_clamped_flag = true;
			}
		}
		else
		{
			try_cem8_12(
				cem_index, pixel_stats, enc_params,
				endpoint_ise_range, weight_ise_range,
				low_color_f, high_color_f,
				trial_blk_endpoints, trial_blk_weights, trial_blk_error, trial_used_blue_contraction, use_blue_contraction, tried_used_blue_contraction);

			if (tried_used_blue_contraction)
			{
				try_cem8_12(
					cem_index, pixel_stats, enc_params,
					endpoint_ise_range, weight_ise_range,
					low_color_f, high_color_f,
					trial_blk_endpoints, trial_blk_weights, trial_blk_error, trial_used_blue_contraction, false, tried_used_blue_contraction);
			}
		}

		if (trial_blk_error == UINT64_MAX)
			return cur_blk_error;

		if (trial_blk_error < cur_blk_error)
		{
			cur_blk_error = trial_blk_error;
			memcpy(pEndpoint_vals, trial_blk_endpoints, total_endpoint_vals);
			memcpy(pWeight_vals, trial_blk_weights, total_weights);
		}
				
		for (uint32_t pass = 0; pass < enc_params.m_max_ls_passes; pass++)
		{
			vec4F xl, xh;

			bool ls_res;
			if (cem_has_alpha)
			{
				ls_res = compute_least_squares_endpoints_4D(
					pixel_stats.m_num_pixels, trial_blk_weights, get_ls_weights_ise(weight_ise_range),
					&xl, &xh, pixel_stats.m_pixels_f, pixel_stats.m_min_f, pixel_stats.m_max_f);
			}
			else
			{
				ls_res = compute_least_squares_endpoints_3D(
					pixel_stats.m_num_pixels, trial_blk_weights, get_ls_weights_ise(weight_ise_range),
					&xl, &xh, pixel_stats.m_pixels_f, pixel_stats.m_min_f, pixel_stats.m_max_f);
			}
			if (!ls_res)
				break;
						
			if (cem_is_base_offset)
			{
				bool tried_base_ofs_clamped = false;

				try_cem9_13_sp_or_dp(
					cem_index, -1, pixel_stats, enc_params,
					endpoint_ise_range, weight_ise_range,
					xl, xh,
					trial_blk_endpoints, trial_blk_weights, nullptr, trial_blk_error, trial_used_blue_contraction, use_blue_contraction, tried_used_blue_contraction, tried_base_ofs_clamped);

				if ((pBase_ofs_clamped_flag) && (tried_base_ofs_clamped))
					*pBase_ofs_clamped_flag = true;

				if (tried_used_blue_contraction)
				{
					// Try without blue contraction for a minor gain.
					try_cem9_13_sp_or_dp(
						cem_index, -1, pixel_stats, enc_params,
						endpoint_ise_range, weight_ise_range,
						xl, xh,
						trial_blk_endpoints, trial_blk_weights, nullptr, trial_blk_error, trial_used_blue_contraction, false, tried_used_blue_contraction, tried_base_ofs_clamped);

					if ((pBase_ofs_clamped_flag) && (tried_base_ofs_clamped))
						*pBase_ofs_clamped_flag = true;
				}
			}
			else
			{
				try_cem8_12(
					cem_index, pixel_stats, enc_params,
					endpoint_ise_range, weight_ise_range,
					xl, xh,
					trial_blk_endpoints, trial_blk_weights, trial_blk_error, trial_used_blue_contraction, use_blue_contraction, tried_used_blue_contraction);

				if (tried_used_blue_contraction)
				{
					// Try without blue contraction for a minor gain.
					try_cem8_12(
						cem_index, pixel_stats, enc_params,
						endpoint_ise_range, weight_ise_range,
						xl, xh,
						trial_blk_endpoints, trial_blk_weights, trial_blk_error, trial_used_blue_contraction, false, tried_used_blue_contraction);
				}
			}

			if (trial_blk_error >= cur_blk_error)
				break;

			cur_blk_error = trial_blk_error;
			memcpy(pEndpoint_vals, trial_blk_endpoints, total_endpoint_vals);
			memcpy(pWeight_vals, trial_blk_weights, total_weights);

		} // pass

		if ((enc_params.m_total_weight_refine_passes) && ((weight_ise_range != astc_helpers::BISE_2_LEVELS) && (weight_ise_range != astc_helpers::BISE_64_LEVELS)))
		{
			weight_refiner refiner;
			refiner.init(weight_ise_range, pixel_stats.m_num_pixels, pWeight_vals);

			for (uint32_t pass = 0; pass < enc_params.m_total_weight_refine_passes; pass++)
			{
				refiner.refine(pass, trial_blk_weights);

				vec4F xl, xh;

				bool ls_res;
				if (cem_has_alpha)
				{
					ls_res = compute_least_squares_endpoints_4D(
						pixel_stats.m_num_pixels, trial_blk_weights, get_ls_weights_ise(weight_ise_range),
						&xl, &xh, pixel_stats.m_pixels_f, pixel_stats.m_min_f, pixel_stats.m_max_f);
				}
				else
				{
					ls_res = compute_least_squares_endpoints_3D(
						pixel_stats.m_num_pixels, trial_blk_weights, get_ls_weights_ise(weight_ise_range),
						&xl, &xh, pixel_stats.m_pixels_f, pixel_stats.m_min_f, pixel_stats.m_max_f);
				}
				if (!ls_res)
					continue;

				if (cem_is_base_offset)
				{
					bool tried_base_ofs_clamped = false;

					try_cem9_13_sp_or_dp(
						cem_index, -1, pixel_stats, enc_params,
						endpoint_ise_range, weight_ise_range,
						xl, xh,
						trial_blk_endpoints, trial_blk_weights, nullptr, trial_blk_error, trial_used_blue_contraction, use_blue_contraction, tried_used_blue_contraction, tried_base_ofs_clamped);

					if ((pBase_ofs_clamped_flag) && (tried_base_ofs_clamped))
						*pBase_ofs_clamped_flag = true;

					if (tried_used_blue_contraction)
					{
						// Try without blue contraction for a minor gain.
						try_cem9_13_sp_or_dp(
							cem_index, -1, pixel_stats, enc_params,
							endpoint_ise_range, weight_ise_range,
							xl, xh,
							trial_blk_endpoints, trial_blk_weights, nullptr, trial_blk_error, trial_used_blue_contraction, false, tried_used_blue_contraction, tried_base_ofs_clamped);

						if ((pBase_ofs_clamped_flag) && (tried_base_ofs_clamped))
							*pBase_ofs_clamped_flag = true;
					}
				}
				else
				{
					try_cem8_12(
						cem_index, pixel_stats, enc_params,
						endpoint_ise_range, weight_ise_range,
						xl, xh,
						trial_blk_endpoints, trial_blk_weights, trial_blk_error, trial_used_blue_contraction, use_blue_contraction, tried_used_blue_contraction);

					if (tried_used_blue_contraction)
					{
						// Try without blue contraction for a minor gain.
						try_cem8_12(
							cem_index, pixel_stats, enc_params,
							endpoint_ise_range, weight_ise_range,
							xl, xh,
							trial_blk_endpoints, trial_blk_weights, trial_blk_error, trial_used_blue_contraction, false, tried_used_blue_contraction);
					}
				}

				if (trial_blk_error < cur_blk_error)
				{
					cur_blk_error = trial_blk_error;
					memcpy(pEndpoint_vals, trial_blk_endpoints, total_endpoint_vals);
					memcpy(pWeight_vals, trial_blk_weights, total_weights);
				}

			} // pass
		}

		const uint32_t N = 4;
		if ((enc_params.m_worst_weight_nudging_flag) &&
			(pixel_stats.m_num_pixels > N) && 
			((weight_ise_range != astc_helpers::BISE_2_LEVELS) && (weight_ise_range != astc_helpers::BISE_64_LEVELS)))
		{
			const uint32_t NUM_NUDGING_PASSES = 1;
			for (uint32_t pass = 0; pass < NUM_NUDGING_PASSES; pass++)
			{
				color_rgba l, h;
				decode_endpoints(cem_index, pEndpoint_vals, endpoint_ise_range, l, h);

				vec4F dir;
				dir[0] = (float)(h[0] - l[0]);
				dir[1] = (float)(h[1] - l[1]);
				dir[2] = (float)(h[2] - l[2]);
				dir[3] = cem_has_alpha ? (float)(h[3] - l[3]) : 0.0f;

				dir.normalize_in_place();

				float errs[ASTC_LDR_MAX_BLOCK_PIXELS];
				float delta_dots[ASTC_LDR_MAX_BLOCK_PIXELS];
				for (uint32_t i = 0; i < pixel_stats.m_num_pixels; i++)
				{
					vec4F ofs(pixel_stats.m_pixels_f[i] - pixel_stats.m_mean_f);

					float proj = dir.dot(ofs);

					vec4F proj_vec(pixel_stats.m_mean_f + proj * dir);

					vec4F delta_vec(pixel_stats.m_pixels_f[i] - proj_vec);

					delta_dots[i] = dir.dot(delta_vec);

					errs[i] = cem_has_alpha ? vec4F::dot_product(delta_vec, delta_vec) : vec4F::dot_product3(delta_vec, delta_vec);
				}

				uint32_t errs_indices[ASTC_LDR_MAX_BLOCK_PIXELS];
				indirect_sort(pixel_stats.m_num_pixels, errs_indices, errs);
												
				memcpy(trial_blk_weights, pWeight_vals, total_weights);

				for (uint32_t i = 0; i < N; i++)
				{
					const uint32_t idx = errs_indices[pixel_stats.m_num_pixels - 1 - i];

					int delta_to_apply = (delta_dots[idx] > 0.0f) ? 1 : -1;

					trial_blk_weights[idx] = (uint8_t)apply_delta_to_bise_weight_val(weight_ise_range, trial_blk_weights[idx], delta_to_apply);
				} // i

				vec4F xl, xh;

				bool ls_res;
				if (cem_has_alpha)
				{
					ls_res = compute_least_squares_endpoints_4D(
						pixel_stats.m_num_pixels, trial_blk_weights, get_ls_weights_ise(weight_ise_range),
						&xl, &xh, pixel_stats.m_pixels_f, pixel_stats.m_min_f, pixel_stats.m_max_f);
				}
				else
				{
					ls_res = compute_least_squares_endpoints_3D(
						pixel_stats.m_num_pixels, trial_blk_weights, get_ls_weights_ise(weight_ise_range),
						&xl, &xh, pixel_stats.m_pixels_f, pixel_stats.m_min_f, pixel_stats.m_max_f);
				}
				if (!ls_res)
					break;

				if (cem_is_base_offset)
				{
					bool tried_base_ofs_clamped = false;

					try_cem9_13_sp_or_dp(
						cem_index, -1, pixel_stats, enc_params,
						endpoint_ise_range, weight_ise_range,
						xl, xh,
						trial_blk_endpoints, trial_blk_weights, nullptr, trial_blk_error, trial_used_blue_contraction, use_blue_contraction, tried_used_blue_contraction, tried_base_ofs_clamped);

					if ((pBase_ofs_clamped_flag) && (tried_base_ofs_clamped))
						*pBase_ofs_clamped_flag = true;

					if (tried_used_blue_contraction)
					{
						// Try without blue contraction for a minor gain.
						try_cem9_13_sp_or_dp(
							cem_index, -1, pixel_stats, enc_params,
							endpoint_ise_range, weight_ise_range,
							xl, xh,
							trial_blk_endpoints, trial_blk_weights, nullptr, trial_blk_error, trial_used_blue_contraction, false, tried_used_blue_contraction, tried_base_ofs_clamped);

						if ((pBase_ofs_clamped_flag) && (tried_base_ofs_clamped))
							*pBase_ofs_clamped_flag = true;
					}
				}
				else
				{
					try_cem8_12(
						cem_index, pixel_stats, enc_params,
						endpoint_ise_range, weight_ise_range,
						xl, xh,
						trial_blk_endpoints, trial_blk_weights, trial_blk_error, trial_used_blue_contraction, use_blue_contraction, tried_used_blue_contraction);

					if (tried_used_blue_contraction)
					{
						// Try without blue contraction for a minor gain.
						try_cem8_12(
							cem_index, pixel_stats, enc_params,
							endpoint_ise_range, weight_ise_range,
							xl, xh,
							trial_blk_endpoints, trial_blk_weights, trial_blk_error, trial_used_blue_contraction, false, tried_used_blue_contraction);
					}
				}

				if (trial_blk_error < cur_blk_error)
				{
					cur_blk_error = trial_blk_error;
					memcpy(pEndpoint_vals, trial_blk_endpoints, total_endpoint_vals);
					memcpy(pWeight_vals, trial_blk_weights, total_weights);
				}
				else
				{
					break;
				}
			} // pass
		}

		if (enc_params.m_endpoint_refinement_flag)
		{
			const uint32_t num_comps = cem_has_alpha ? 4 : 3;
						
			for (uint32_t c = 0; c < num_comps; c++)
			{
				uint8_t base_endpoint_vals[astc_helpers::MAX_CEM_ENDPOINT_VALS];
				memcpy(base_endpoint_vals, pEndpoint_vals, total_endpoint_vals);

				for (int dl = -1; dl <= 1; dl++)
				{
					for (int dh = -1; dh <= 1; dh++)
					{
						if (!dl && !dh)
							continue;

						memcpy(trial_blk_endpoints, base_endpoint_vals, total_endpoint_vals);

						trial_blk_endpoints[c * 2 + 0] = (uint8_t)astc_helpers::apply_delta_to_bise_endpoint_val(endpoint_ise_range, trial_blk_endpoints[c * 2 + 0], dl);
						trial_blk_endpoints[c * 2 + 1] = (uint8_t)astc_helpers::apply_delta_to_bise_endpoint_val(endpoint_ise_range, trial_blk_endpoints[c * 2 + 1], dh);
						
						if (!use_blue_contraction)
						{
							const bool uses_blue_contraction = astc_helpers::used_blue_contraction(cem_index, trial_blk_endpoints, endpoint_ise_range);
							if (uses_blue_contraction)
								continue;
						}

						trial_blk_error = eval_solution(
							pixel_stats,
							cem_index, trial_blk_endpoints, endpoint_ise_range,
							trial_blk_weights, weight_ise_range,
							enc_params);

						if (trial_blk_error < cur_blk_error)
						{
							cur_blk_error = trial_blk_error;
							memcpy(pEndpoint_vals, trial_blk_endpoints, total_endpoint_vals);
							memcpy(pWeight_vals, trial_blk_weights, total_weights);
						}

					} // dh

				} // dl
			}
		}

		return cur_blk_error;
	}

	// rgb/rgba direct, or rgb/rgba base+offset, dual plane
	static uint64_t encode_cem8_12_9_13_dp(
		uint32_t cem_index, uint32_t ccs_index,
		const pixel_stats_t& pixel_stats, const cem_encode_params& enc_params,
		uint32_t endpoint_ise_range, uint32_t weight_ise_range,
		uint8_t* pEndpoint_vals, uint8_t* pWeight_vals0, uint8_t* pWeight_vals1,
		uint64_t cur_blk_error, bool use_blue_contraction, bool *pBase_ofs_clamped_flag)
	{
		assert(g_initialized);
		assert(ccs_index <= 3);
		assert((pixel_stats.m_num_pixels) && (pixel_stats.m_num_pixels <= ASTC_LDR_MAX_BLOCK_PIXELS));
		assert((endpoint_ise_range >= astc_helpers::FIRST_VALID_ENDPOINT_ISE_RANGE) && (endpoint_ise_range <= astc_helpers::LAST_VALID_ENDPOINT_ISE_RANGE));
		assert(((weight_ise_range >= astc_helpers::FIRST_VALID_WEIGHT_ISE_RANGE) && (weight_ise_range <= astc_helpers::LAST_VALID_WEIGHT_ISE_RANGE)) || (weight_ise_range == astc_helpers::BISE_64_LEVELS));

		if (pBase_ofs_clamped_flag)
			*pBase_ofs_clamped_flag = false;

		bool cem_has_alpha = false, cem_is_base_offset = false;
		switch (cem_index)
		{
		case astc_helpers::CEM_LDR_RGB_DIRECT: break;
		case astc_helpers::CEM_LDR_RGBA_DIRECT: cem_has_alpha = true; break;
		case astc_helpers::CEM_LDR_RGB_BASE_PLUS_OFFSET: cem_is_base_offset = true; break;
		case astc_helpers::CEM_LDR_RGBA_BASE_PLUS_OFFSET: cem_is_base_offset = true; cem_has_alpha = true; break;
		default:
			assert(0);
			return false;
		}

		assert((ccs_index <= 2) || cem_has_alpha);

		const uint32_t total_endpoint_vals = astc_helpers::get_num_cem_values(cem_index);
		const uint32_t total_weights = pixel_stats.m_num_pixels;

		// Remove influence of the 2nd plane's values, recalc principle axis on other values.
		vec4F flattened_pixels[ASTC_LDR_MAX_BLOCK_PIXELS];
		for (uint32_t i = 0; i < pixel_stats.m_num_pixels; i++)
		{
			flattened_pixels[i] = pixel_stats.m_pixels_f[i];
			flattened_pixels[i][ccs_index] = 0.0f;
						
			if (!cem_has_alpha)
				flattened_pixels[i][3] = 0.0f;
		}

		vec4F flattened_pixels_mean(pixel_stats.m_mean_f);
		flattened_pixels_mean[ccs_index] = 0.0f;
		
		if (!cem_has_alpha)
			flattened_pixels_mean[3] = 0.0f;

		vec4F flattened_axis;
		if (!cem_has_alpha)
			flattened_axis = calc_pca_3D(pixel_stats.m_num_pixels, flattened_pixels, flattened_pixels_mean);
		else
			flattened_axis = calc_pca_4D(pixel_stats.m_num_pixels, flattened_pixels, flattened_pixels_mean);

		float best_l = BIG_FLOAT_VAL, best_h = -BIG_FLOAT_VAL;
		//int best_l_index = 0, best_h_index = 0;

		for (uint32_t c = 0; c < pixel_stats.m_num_pixels; c++)
		{
			const vec4F px(flattened_pixels[c] - flattened_pixels_mean);

			float p = px.dot(flattened_axis);
			if (p < best_l)
			{
				best_l = p;
				//best_l_index = c;
			}

			if (p > best_h)
			{
				best_h = p;
				//best_h_index = c;
			}
		} // c

#if 0
		vec4F low_color_f(pixel_stats.m_pixels_f[best_l_index]), high_color_f(pixel_stats.m_pixels_f[best_h_index]);
#else
		vec4F low_color_f, high_color_f;
		low_color_f = flattened_pixels_mean + flattened_axis * best_l;
		high_color_f = flattened_pixels_mean + flattened_axis * best_h;

		low_color_f.clamp(0.0f, 1.0f);
		high_color_f.clamp(0.0f, 1.0f);
#endif

		low_color_f[ccs_index] = pixel_stats.m_min_f[ccs_index];
		high_color_f[ccs_index] = pixel_stats.m_max_f[ccs_index];

		uint8_t trial_blk_endpoints[astc_helpers::MAX_CEM_ENDPOINT_VALS] = { 0 };
		uint8_t trial_blk_weights0[ASTC_LDR_MAX_BLOCK_PIXELS] = { 0 };
		uint8_t trial_blk_weights1[ASTC_LDR_MAX_BLOCK_PIXELS] = { 0 };
		uint64_t trial_blk_error = UINT64_MAX;
		bool trial_used_blue_contraction = false;

		bool tried_used_blue_contraction = false;
		
		if (cem_is_base_offset)
		{
			bool tried_base_ofs_clamped = false;

			try_cem9_13_sp_or_dp(
				cem_index, ccs_index, pixel_stats, enc_params,
				endpoint_ise_range, weight_ise_range,
				low_color_f, high_color_f,
				trial_blk_endpoints, trial_blk_weights0, trial_blk_weights1,
				trial_blk_error, trial_used_blue_contraction, use_blue_contraction, tried_used_blue_contraction, tried_base_ofs_clamped);

			if ((pBase_ofs_clamped_flag) && (tried_base_ofs_clamped))
				*pBase_ofs_clamped_flag = true;

			if (tried_used_blue_contraction)
			{
				try_cem9_13_sp_or_dp(
					cem_index, ccs_index, pixel_stats, enc_params,
					endpoint_ise_range, weight_ise_range,
					low_color_f, high_color_f,
					trial_blk_endpoints, trial_blk_weights0, trial_blk_weights1, trial_blk_error, trial_used_blue_contraction, false, tried_used_blue_contraction, tried_base_ofs_clamped);

				if ((pBase_ofs_clamped_flag) && (tried_base_ofs_clamped))
					*pBase_ofs_clamped_flag = true;
			}
		}
		else
		{
			try_cem8_12_dp(
				cem_index, ccs_index, pixel_stats, enc_params,
				endpoint_ise_range, weight_ise_range,
				low_color_f, high_color_f,
				trial_blk_endpoints, trial_blk_weights0, trial_blk_weights1,
				trial_blk_error, trial_used_blue_contraction, use_blue_contraction, tried_used_blue_contraction);

			if (tried_used_blue_contraction)
			{
				try_cem8_12_dp(
					cem_index, ccs_index, pixel_stats, enc_params,
					endpoint_ise_range, weight_ise_range,
					low_color_f, high_color_f,
					trial_blk_endpoints, trial_blk_weights0, trial_blk_weights1, trial_blk_error, trial_used_blue_contraction, false, tried_used_blue_contraction);
			}
		}

		if (trial_blk_error == UINT64_MAX)
			return cur_blk_error;

		if (trial_blk_error < cur_blk_error)
		{
			cur_blk_error = trial_blk_error;
			memcpy(pEndpoint_vals, trial_blk_endpoints, total_endpoint_vals);
			memcpy(pWeight_vals0, trial_blk_weights0, total_weights);
			memcpy(pWeight_vals1, trial_blk_weights1, total_weights);
		}
				
		vec4F flattened_pixels_min_f(pixel_stats.m_min_f);
		flattened_pixels_min_f[ccs_index] = 0;

		vec4F flattened_pixels_max_f(pixel_stats.m_max_f);
		flattened_pixels_max_f[ccs_index] = 0;

		for (uint32_t pass = 0; pass < enc_params.m_max_ls_passes; pass++)
		{
			vec4F xl, xh;

			// TODO: Switch between 4D or 3D
			if (!compute_least_squares_endpoints_4D(
				pixel_stats.m_num_pixels, trial_blk_weights0, get_ls_weights_ise(weight_ise_range),
				&xl, &xh, flattened_pixels, flattened_pixels_min_f, flattened_pixels_max_f))
			{
				break;
			}

			color_rgba dec_l(0), dec_h(0);
			decode_endpoints(cem_index, trial_blk_endpoints, endpoint_ise_range, dec_l, dec_h);

			xl[ccs_index] = dec_l[ccs_index] * (1.0f / 255.0f);
			xh[ccs_index] = dec_h[ccs_index] * (1.0f / 255.0f);

			if (cem_is_base_offset)
			{
				bool tried_base_ofs_clamped = false;

				try_cem9_13_sp_or_dp(
					cem_index, ccs_index, pixel_stats, enc_params,
					endpoint_ise_range, weight_ise_range,
					xl, xh,
					trial_blk_endpoints, trial_blk_weights0, trial_blk_weights1, trial_blk_error, trial_used_blue_contraction,
					use_blue_contraction, tried_used_blue_contraction, tried_base_ofs_clamped);

				if ((pBase_ofs_clamped_flag) && (tried_base_ofs_clamped))
					*pBase_ofs_clamped_flag = true;

				if (tried_used_blue_contraction)
				{
					// Try without blue contraction for a minor gain.
					try_cem9_13_sp_or_dp(
						cem_index, ccs_index, pixel_stats, enc_params,
						endpoint_ise_range, weight_ise_range,
						xl, xh,
						trial_blk_endpoints, trial_blk_weights0, trial_blk_weights1, trial_blk_error, trial_used_blue_contraction,
						false, tried_used_blue_contraction, tried_base_ofs_clamped);

					if ((pBase_ofs_clamped_flag) && (tried_base_ofs_clamped))
						*pBase_ofs_clamped_flag = true;
				}
			}
			else
			{
				try_cem8_12_dp(
					cem_index, ccs_index, pixel_stats, enc_params,
					endpoint_ise_range, weight_ise_range,
					xl, xh,
					trial_blk_endpoints, trial_blk_weights0, trial_blk_weights1, trial_blk_error, trial_used_blue_contraction,
					use_blue_contraction, tried_used_blue_contraction);

				if (tried_used_blue_contraction)
				{
					// Try without blue contraction for a minor gain.
					try_cem8_12_dp(
						cem_index, ccs_index, pixel_stats, enc_params,
						endpoint_ise_range, weight_ise_range,
						xl, xh,
						trial_blk_endpoints, trial_blk_weights0, trial_blk_weights1, trial_blk_error, trial_used_blue_contraction,
						false, tried_used_blue_contraction);
				}
			}

			if (trial_blk_error >= cur_blk_error)
				break;

			cur_blk_error = trial_blk_error;
			memcpy(pEndpoint_vals, trial_blk_endpoints, total_endpoint_vals);
			memcpy(pWeight_vals0, trial_blk_weights0, total_weights);
			memcpy(pWeight_vals1, trial_blk_weights1, total_weights);

		} // pass

		const float ccs_bounds_min = pixel_stats.m_min_f[ccs_index];
		const float ccs_bounds_max = pixel_stats.m_max_f[ccs_index];
		float ccs_vals[ASTC_LDR_MAX_BLOCK_PIXELS];

		if (ccs_bounds_min != ccs_bounds_max)
		{
			for (uint32_t i = 0; i < pixel_stats.m_num_pixels; i++)
				ccs_vals[i] = pixel_stats.m_pixels_f[i][ccs_index];

			for (uint32_t pass = 0; pass < enc_params.m_max_ls_passes; pass++)
			{
				float xl = 0.0f, xh = 0.0f;

				if (!compute_least_squares_endpoints_1D(
					pixel_stats.m_num_pixels, trial_blk_weights1, get_ls_weights_ise(weight_ise_range),
					&xl, &xh, ccs_vals, ccs_bounds_min, ccs_bounds_max))
				{
					break;
				}

				color_rgba dec_l(0), dec_h(0);
				decode_endpoints(cem_index, trial_blk_endpoints, endpoint_ise_range, dec_l, dec_h);

				vec4F vl, vh;
				for (uint32_t c = 0; c < 4; c++)
				{
					if (c == ccs_index)
					{
						vl[c] = xl;
						vh[c] = xh;
					}
					else
					{
						vl[c] = (float)dec_l[c] * (1.0f / 255.0f);
						vh[c] = (float)dec_h[c] * (1.0f / 255.0f);
					}
				}

				if (cem_is_base_offset)
				{
					bool tried_base_ofs_clamped = false;

					try_cem9_13_sp_or_dp(
						cem_index, ccs_index, pixel_stats, enc_params,
						endpoint_ise_range, weight_ise_range,
						vl, vh,
						trial_blk_endpoints, trial_blk_weights0, trial_blk_weights1, trial_blk_error, trial_used_blue_contraction,
						use_blue_contraction, tried_used_blue_contraction, tried_base_ofs_clamped);

					if ((pBase_ofs_clamped_flag) && (tried_base_ofs_clamped))
						*pBase_ofs_clamped_flag = true;

					if (tried_used_blue_contraction)
					{
						// Try without blue contraction for a minor gain.
						try_cem9_13_sp_or_dp(
							cem_index, ccs_index, pixel_stats, enc_params,
							endpoint_ise_range, weight_ise_range,
							vl, vh,
							trial_blk_endpoints, trial_blk_weights0, trial_blk_weights1, trial_blk_error, trial_used_blue_contraction,
							false, tried_used_blue_contraction, tried_base_ofs_clamped);

						if ((pBase_ofs_clamped_flag) && (tried_base_ofs_clamped))
							*pBase_ofs_clamped_flag = true;
					}
				}
				else
				{
					try_cem8_12_dp(
						cem_index, ccs_index, pixel_stats, enc_params,
						endpoint_ise_range, weight_ise_range,
						vl, vh,
						trial_blk_endpoints, trial_blk_weights0, trial_blk_weights1, trial_blk_error, trial_used_blue_contraction,
						use_blue_contraction, tried_used_blue_contraction);

					if (tried_used_blue_contraction)
					{
						// Try without blue contraction for a minor gain.
						try_cem8_12_dp(
							cem_index, ccs_index, pixel_stats, enc_params,
							endpoint_ise_range, weight_ise_range,
							vl, vh,
							trial_blk_endpoints, trial_blk_weights0, trial_blk_weights1, trial_blk_error, trial_used_blue_contraction,
							false, tried_used_blue_contraction);
					}
				}

				if (trial_blk_error >= cur_blk_error)
					break;

				cur_blk_error = trial_blk_error;
				memcpy(pEndpoint_vals, trial_blk_endpoints, total_endpoint_vals);
				memcpy(pWeight_vals0, trial_blk_weights0, total_weights);
				memcpy(pWeight_vals1, trial_blk_weights1, total_weights);

			} // pass
		}

		if ((enc_params.m_total_weight_refine_passes) && ((weight_ise_range != astc_helpers::BISE_2_LEVELS) && (weight_ise_range != astc_helpers::BISE_64_LEVELS)))
		{
			weight_refiner refiner;
			refiner.init(weight_ise_range, pixel_stats.m_num_pixels, pWeight_vals0);

			for (uint32_t pass = 0; pass < enc_params.m_total_weight_refine_passes; pass++)
			{
				refiner.refine(pass, trial_blk_weights0);

				vec4F xl, xh;

				if (!compute_least_squares_endpoints_4D(
					pixel_stats.m_num_pixels, trial_blk_weights0, get_ls_weights_ise(weight_ise_range),
					&xl, &xh, flattened_pixels, flattened_pixels_min_f, flattened_pixels_max_f))
				{
					break;
				}

				color_rgba dec_l(0), dec_h(0);
				decode_endpoints(cem_index, trial_blk_endpoints, endpoint_ise_range, dec_l, dec_h);

				xl[ccs_index] = dec_l[ccs_index] * (1.0f / 255.0f);
				xh[ccs_index] = dec_h[ccs_index] * (1.0f / 255.0f);

				if (cem_is_base_offset)
				{
					bool tried_base_ofs_clamped = false;

					try_cem9_13_sp_or_dp(
						cem_index, ccs_index, pixel_stats, enc_params,
						endpoint_ise_range, weight_ise_range,
						xl, xh,
						trial_blk_endpoints, trial_blk_weights0, trial_blk_weights1, trial_blk_error, trial_used_blue_contraction,
						use_blue_contraction, tried_used_blue_contraction, tried_base_ofs_clamped);

					if ((pBase_ofs_clamped_flag) && (tried_base_ofs_clamped))
						*pBase_ofs_clamped_flag = true;

					if (tried_used_blue_contraction)
					{
						// Try without blue contraction for a minor gain.
						try_cem9_13_sp_or_dp(
							cem_index, ccs_index, pixel_stats, enc_params,
							endpoint_ise_range, weight_ise_range,
							xl, xh,
							trial_blk_endpoints, trial_blk_weights0, trial_blk_weights1, trial_blk_error, trial_used_blue_contraction,
							false, tried_used_blue_contraction, tried_base_ofs_clamped);

						if ((pBase_ofs_clamped_flag) && (tried_base_ofs_clamped))
							*pBase_ofs_clamped_flag = true;
					}
				}
				else
				{
					try_cem8_12_dp(
						cem_index, ccs_index, pixel_stats, enc_params,
						endpoint_ise_range, weight_ise_range,
						xl, xh,
						trial_blk_endpoints, trial_blk_weights0, trial_blk_weights1, trial_blk_error, trial_used_blue_contraction,
						use_blue_contraction, tried_used_blue_contraction);

					if (tried_used_blue_contraction)
					{
						// Try without blue contraction for a minor gain.
						try_cem8_12_dp(
							cem_index, ccs_index, pixel_stats, enc_params,
							endpoint_ise_range, weight_ise_range,
							xl, xh,
							trial_blk_endpoints, trial_blk_weights0, trial_blk_weights1, trial_blk_error, trial_used_blue_contraction,
							false, tried_used_blue_contraction);
					}
				}

				if (trial_blk_error >= cur_blk_error)
					continue;

				cur_blk_error = trial_blk_error;
				memcpy(pEndpoint_vals, trial_blk_endpoints, total_endpoint_vals);
				memcpy(pWeight_vals0, trial_blk_weights0, total_weights);
				memcpy(pWeight_vals1, trial_blk_weights1, total_weights);

			} // pass

			if (ccs_bounds_min != ccs_bounds_max)
			{
				refiner.init(weight_ise_range, pixel_stats.m_num_pixels, pWeight_vals1);

				for (uint32_t pass = 0; pass < WEIGHT_REFINER_MAX_PASSES; pass++)
				{
					refiner.refine(pass, trial_blk_weights1);

					float xl = 0.0f, xh = 0.0f;

					if (!compute_least_squares_endpoints_1D(
						pixel_stats.m_num_pixels, trial_blk_weights1, get_ls_weights_ise(weight_ise_range),
						&xl, &xh, ccs_vals, ccs_bounds_min, ccs_bounds_max))
					{
						break;
					}
					
					color_rgba dec_l(0), dec_h(0);
					decode_endpoints(cem_index, trial_blk_endpoints, endpoint_ise_range, dec_l, dec_h);

					vec4F vl, vh;
					for (uint32_t c = 0; c < 4; c++)
					{
						if (c == ccs_index)
						{
							vl[c] = xl;
							vh[c] = xh;
						}
						else
						{
							vl[c] = (float)dec_l[c] * (1.0f / 255.0f);
							vh[c] = (float)dec_h[c] * (1.0f / 255.0f);
						}
					}

					bool did_improve_res = false;

					if (cem_is_base_offset)
					{
						bool tried_base_ofs_clamped = false;

						did_improve_res = try_cem9_13_sp_or_dp(
							cem_index, ccs_index, pixel_stats, enc_params,
							endpoint_ise_range, weight_ise_range,
							vl, vh,
							trial_blk_endpoints, trial_blk_weights0, trial_blk_weights1, trial_blk_error, trial_used_blue_contraction,
							use_blue_contraction, tried_used_blue_contraction, tried_base_ofs_clamped);
						BASISU_NOTE_UNUSED(did_improve_res);

						if ((pBase_ofs_clamped_flag) && (tried_base_ofs_clamped))
							*pBase_ofs_clamped_flag = true;

						if (tried_used_blue_contraction)
						{
							// Try without blue contraction for a minor gain.
							did_improve_res = try_cem9_13_sp_or_dp(
								cem_index, ccs_index, pixel_stats, enc_params,
								endpoint_ise_range, weight_ise_range,
								vl, vh,
								trial_blk_endpoints, trial_blk_weights0, trial_blk_weights1, trial_blk_error, trial_used_blue_contraction,
								false, tried_used_blue_contraction, tried_base_ofs_clamped);

							if ((pBase_ofs_clamped_flag) && (tried_base_ofs_clamped))
								*pBase_ofs_clamped_flag = true;
						}
					}
					else
					{
						did_improve_res = try_cem8_12_dp(
							cem_index, ccs_index, pixel_stats, enc_params,
							endpoint_ise_range, weight_ise_range,
							vl, vh,
							trial_blk_endpoints, trial_blk_weights0, trial_blk_weights1, trial_blk_error, trial_used_blue_contraction,
							use_blue_contraction, tried_used_blue_contraction);

						if (tried_used_blue_contraction)
						{
							// Try without blue contraction for a minor gain.
							did_improve_res = try_cem8_12_dp(
								cem_index, ccs_index, pixel_stats, enc_params,
								endpoint_ise_range, weight_ise_range,
								vl, vh,
								trial_blk_endpoints, trial_blk_weights0, trial_blk_weights1, trial_blk_error, trial_used_blue_contraction,
								false, tried_used_blue_contraction);
						}
					}

					if (trial_blk_error >= cur_blk_error)
						continue;

					cur_blk_error = trial_blk_error;
					memcpy(pEndpoint_vals, trial_blk_endpoints, total_endpoint_vals);
					memcpy(pWeight_vals0, trial_blk_weights0, total_weights);
					memcpy(pWeight_vals1, trial_blk_weights1, total_weights);

				} // pass
			} 
		}

		return cur_blk_error;
	}

	// base scale rgb/rgba
	// returns true if improved
	static bool try_cem6_10(
		uint32_t cem_index,
		const pixel_stats_t& pixel_stats, const cem_encode_params& enc_params,
		uint32_t endpoint_ise_range, uint32_t weight_ise_range,
		float scale, float low_a_f, const vec4F& high_color_f,
		uint8_t* pTrial_endpoint_vals, uint8_t* pTrial_weight_vals, uint64_t& trial_blk_error)
	{
		assert(g_initialized);
		assert((cem_index == astc_helpers::CEM_LDR_RGB_BASE_SCALE) || (cem_index == astc_helpers::CEM_LDR_RGB_BASE_SCALE_PLUS_TWO_A));
		assert((pixel_stats.m_num_pixels) && (pixel_stats.m_num_pixels <= ASTC_LDR_MAX_BLOCK_PIXELS));
		assert((endpoint_ise_range >= astc_helpers::FIRST_VALID_ENDPOINT_ISE_RANGE) && (endpoint_ise_range <= astc_helpers::LAST_VALID_ENDPOINT_ISE_RANGE));
		assert(((weight_ise_range >= astc_helpers::FIRST_VALID_WEIGHT_ISE_RANGE) && (weight_ise_range <= astc_helpers::LAST_VALID_WEIGHT_ISE_RANGE)) || (weight_ise_range == astc_helpers::BISE_64_LEVELS));

		uint8_t trial_endpoint_vals[astc_helpers::NUM_MODE10_ENDPOINTS] = { 0 };
		uint8_t trial_weight_vals[ASTC_LDR_MAX_BLOCK_PIXELS] = { 0 };

		cem_encode_ldr_rgb_or_rgba_base_scale(cem_index, endpoint_ise_range, scale, low_a_f, high_color_f, trial_endpoint_vals);

		uint64_t trial_err = eval_solution(
			pixel_stats, cem_index, trial_endpoint_vals, endpoint_ise_range,
			trial_weight_vals, weight_ise_range,
			enc_params);

		bool improved_flag = false;
		if (trial_err < trial_blk_error)
		{
			trial_blk_error = trial_err;
			memcpy(pTrial_endpoint_vals, trial_endpoint_vals, astc_helpers::get_num_cem_values(cem_index));
			memcpy(pTrial_weight_vals, trial_weight_vals, pixel_stats.m_num_pixels);
			improved_flag = true;
		}

		const uint32_t num_endpoint_vals = astc_helpers::get_num_cem_values(cem_index);

		// TODO
		for (int delta = -1; delta <= 1; delta += 1)
		{
			if (!delta)
				continue;

			uint8_t fixed_endpoint_vals[astc_helpers::NUM_MODE10_ENDPOINTS];
			memcpy(fixed_endpoint_vals, trial_endpoint_vals, num_endpoint_vals);

			fixed_endpoint_vals[3] = (uint8_t)astc_helpers::apply_delta_to_bise_endpoint_val(endpoint_ise_range, fixed_endpoint_vals[3], delta);

			trial_err = eval_solution(
				pixel_stats, cem_index, fixed_endpoint_vals, endpoint_ise_range,
				trial_weight_vals, weight_ise_range,
				enc_params);

			if (trial_err < trial_blk_error)
			{
				trial_blk_error = trial_err;
				memcpy(pTrial_endpoint_vals, fixed_endpoint_vals, astc_helpers::get_num_cem_values(cem_index));
				memcpy(pTrial_weight_vals, trial_weight_vals, pixel_stats.m_num_pixels);
				improved_flag = true;
			}
		}

		return improved_flag;
	}

	static bool try_cem6_10_dp(
		uint32_t cem_index, uint32_t ccs_index,
		const pixel_stats_t& pixel_stats, const cem_encode_params& enc_params,
		uint32_t endpoint_ise_range, uint32_t weight_ise_range,
		float scale, float low_a_f, const vec4F& high_color_f,
		uint8_t* pTrial_endpoint_vals, uint8_t* pTrial_weight_vals0, uint8_t* pTrial_weight_vals1, uint64_t& trial_blk_error)
	{
		assert(g_initialized);
		assert((cem_index == astc_helpers::CEM_LDR_RGB_BASE_SCALE) || (cem_index == astc_helpers::CEM_LDR_RGB_BASE_SCALE_PLUS_TWO_A));
		assert(ccs_index <= 3);
		assert((pixel_stats.m_num_pixels) && (pixel_stats.m_num_pixels <= ASTC_LDR_MAX_BLOCK_PIXELS));
		assert((endpoint_ise_range >= astc_helpers::FIRST_VALID_ENDPOINT_ISE_RANGE) && (endpoint_ise_range <= astc_helpers::LAST_VALID_ENDPOINT_ISE_RANGE));
		assert(((weight_ise_range >= astc_helpers::FIRST_VALID_WEIGHT_ISE_RANGE) && (weight_ise_range <= astc_helpers::LAST_VALID_WEIGHT_ISE_RANGE)) || (weight_ise_range == astc_helpers::BISE_64_LEVELS));
		assert(pTrial_weight_vals0 && pTrial_weight_vals1);
		
		uint8_t trial_endpoint_vals[astc_helpers::NUM_MODE10_ENDPOINTS] = { 0 };
		uint8_t trial_weight_vals0[ASTC_LDR_MAX_BLOCK_PIXELS] = { 0 };
		uint8_t trial_weight_vals1[ASTC_LDR_MAX_BLOCK_PIXELS] = { 0 };

		cem_encode_ldr_rgb_or_rgba_base_scale(cem_index, endpoint_ise_range, scale, low_a_f, high_color_f, trial_endpoint_vals);

		uint64_t trial_err = eval_solution_dp(
			pixel_stats, cem_index, ccs_index,
			trial_endpoint_vals, endpoint_ise_range,
			trial_weight_vals0, trial_weight_vals1, weight_ise_range,
			enc_params);

		bool improved_flag = false;
		if (trial_err < trial_blk_error)
		{
			trial_blk_error = trial_err;
			memcpy(pTrial_endpoint_vals, trial_endpoint_vals, astc_helpers::get_num_cem_values(cem_index));
			memcpy(pTrial_weight_vals0, trial_weight_vals0, pixel_stats.m_num_pixels);
			memcpy(pTrial_weight_vals1, trial_weight_vals1, pixel_stats.m_num_pixels);
			improved_flag = true;
		}

		const uint32_t num_endpoint_vals = astc_helpers::get_num_cem_values(cem_index);

		for (int delta = -1; delta <= 1; delta += 1)
		{
			if (!delta)
				continue;

			uint8_t fixed_endpoint_vals[astc_helpers::NUM_MODE10_ENDPOINTS];
			memcpy(fixed_endpoint_vals, trial_endpoint_vals, num_endpoint_vals);

			fixed_endpoint_vals[3] = (uint8_t)astc_helpers::apply_delta_to_bise_endpoint_val(endpoint_ise_range, fixed_endpoint_vals[3], delta);

			trial_err = eval_solution_dp(
				pixel_stats, cem_index, ccs_index,
				fixed_endpoint_vals, endpoint_ise_range,
				trial_weight_vals0, trial_weight_vals1, weight_ise_range,
				enc_params);

			if (trial_err < trial_blk_error)
			{
				trial_blk_error = trial_err;
				memcpy(pTrial_endpoint_vals, fixed_endpoint_vals, astc_helpers::get_num_cem_values(cem_index));
				memcpy(pTrial_weight_vals0, trial_weight_vals0, pixel_stats.m_num_pixels);
				memcpy(pTrial_weight_vals1, trial_weight_vals1, pixel_stats.m_num_pixels);
				improved_flag = true;
			}
		}

		return improved_flag;
	}

	// rgb/rgba base+scale
	static uint64_t encode_cem6_10(
		uint32_t cem_index,
		const pixel_stats_t& pixel_stats, const cem_encode_params& enc_params,
		uint32_t endpoint_ise_range, uint32_t weight_ise_range,
		uint8_t* pEndpoint_vals, uint8_t* pWeight_vals, uint64_t cur_blk_error)
	{
		assert(g_initialized);
		assert((cem_index == astc_helpers::CEM_LDR_RGB_BASE_SCALE) || (cem_index == astc_helpers::CEM_LDR_RGB_BASE_SCALE_PLUS_TWO_A));
		assert((pixel_stats.m_num_pixels) && (pixel_stats.m_num_pixels <= ASTC_LDR_MAX_BLOCK_PIXELS));
		assert((endpoint_ise_range >= astc_helpers::FIRST_VALID_ENDPOINT_ISE_RANGE) && (endpoint_ise_range <= astc_helpers::LAST_VALID_ENDPOINT_ISE_RANGE));
		assert(((weight_ise_range >= astc_helpers::FIRST_VALID_WEIGHT_ISE_RANGE) && (weight_ise_range <= astc_helpers::LAST_VALID_WEIGHT_ISE_RANGE)) || (weight_ise_range == astc_helpers::BISE_64_LEVELS));

		const bool cem_has_alpha = (cem_index == astc_helpers::CEM_LDR_RGB_BASE_SCALE_PLUS_TWO_A);

		const uint32_t total_endpoint_vals = astc_helpers::get_num_cem_values(cem_index);
		const uint32_t total_weights = pixel_stats.m_num_pixels;

		float best_l = BIG_FLOAT_VAL, best_h = -BIG_FLOAT_VAL;
		//int best_l_index = 0, best_h_index = 0;

		for (uint32_t c = 0; c < pixel_stats.m_num_pixels; c++)
		{
			const vec3F px(pixel_stats.m_pixels_f[c]);

			float p = px.dot(pixel_stats.m_zero_rel_axis3);

			if (p < best_l)
			{
				best_l = p;
				//best_l_index = c;
			}

			if (p > best_h)
			{
				best_h = p;
				//best_h_index = c;
			}
		} // c

		const float MAX_S = 255.0f / 256.0f;
		const float EPS = 1e-6f;

		uint64_t trial_blk_error = UINT64_MAX;
		uint8_t trial_blk_endpoints[astc_helpers::NUM_MODE10_ENDPOINTS] = { 0 };
		uint8_t trial_blk_weights[ASTC_LDR_MAX_BLOCK_PIXELS] = { 0 };

		uint64_t best_blk_error = UINT64_MAX;
		uint8_t best_blk_endpoints[astc_helpers::NUM_MODE10_ENDPOINTS] = { 0 };
		uint8_t best_blk_weights[ASTC_LDR_MAX_BLOCK_PIXELS] = { 0 };

		vec3F low_color3_f(best_l * pixel_stats.m_zero_rel_axis3);
		low_color3_f.clamp(0.0f, 1.0f);

		vec3F high_color3_f(best_h * pixel_stats.m_zero_rel_axis3);
		high_color3_f.clamp(0.0f, 1.0f);

		float scale = MAX_S;

		float d = low_color3_f.dot(high_color3_f);
		float nrm = high_color3_f.norm();
		if (nrm > 0.0f)
			scale = saturate(d / nrm);
		scale = minimum(scale, MAX_S);

		vec4F low_color_f(low_color3_f[0], low_color3_f[1], low_color3_f[2], pixel_stats.m_min_f[3]);
		vec4F high_color_f(high_color3_f[0], high_color3_f[1], high_color3_f[2], pixel_stats.m_max_f[3]);

		try_cem6_10(
			cem_index,
			pixel_stats, enc_params,
			endpoint_ise_range, weight_ise_range,
			scale, low_color_f[3], high_color_f,
			trial_blk_endpoints, trial_blk_weights, trial_blk_error);

		best_blk_error = trial_blk_error;
		memcpy(best_blk_endpoints, trial_blk_endpoints, total_endpoint_vals);
		memcpy(best_blk_weights, trial_blk_weights, total_weights);

		const uint32_t NUM_PASSES = 2;
		for (uint32_t pass = 0; pass < NUM_PASSES; pass++)
		{
			color_rgba actual_l(0), actual_h(0);
			float actual_scale = 0;
			decode_endpoints(cem_index, trial_blk_endpoints, endpoint_ise_range, actual_l, actual_h, &actual_scale);

			vec3F actual_high_f((float)actual_h[0], (float)actual_h[1], (float)actual_h[2]);
			actual_high_f *= (1.0f / 255.0f);

			// invalid on raw weights
			const auto& dequant_weights_tab = astc_helpers::g_dequant_tables.get_weight_tab(minimum<uint32_t>(astc_helpers::BISE_32_LEVELS, weight_ise_range)).m_ISE_to_val;

			vec3F Pa(0.0f), Pb(0.0f);
			float A = 0.0f, B = 0.0f, C = 0.0f;

			for (uint32_t i = 0; i < pixel_stats.m_num_pixels; i++)
			{
				const vec3F px(pixel_stats.m_pixels_f[i]);

				const int iw = (weight_ise_range == astc_helpers::BISE_64_LEVELS) ? trial_blk_weights[i] : dequant_weights_tab[trial_blk_weights[i]];
				float t = (float)iw * (1.0f / 64.0f);
				float bi = t, ai = 1.0f - t;

				Pa += px * ai;
				Pb += px * bi;

				A += ai * ai;
				B += ai * bi;
				C += bi * bi;
			}

			vec3F new_high = actual_high_f;
			float new_scale = actual_scale;

			float h2 = actual_high_f.norm();
			if ((h2 > EPS) && (A > EPS))
			{
				new_scale = (Pa.dot(actual_high_f) / h2 - B) / A;
				new_scale = clamp(new_scale, 0.0f, MAX_S);
			}

			const float den = A * new_scale * new_scale + 2.0f * B * new_scale + C;
			if (den > EPS)
			{
				new_high = (Pb + Pa * new_scale) / den;
			}

			h2 = new_high.norm();
			if ((h2 > EPS) && (A > EPS))
			{
				new_scale = (Pa.dot(new_high) / h2 - B) / A;
				new_scale = clamp(new_scale, 0.0f, MAX_S);
			}

			try_cem6_10(
				cem_index,
				pixel_stats, enc_params,
				endpoint_ise_range, weight_ise_range,
				new_scale, (float)actual_l[3] * (1.0f / 255.0f), vec4F(new_high[0], new_high[1], new_high[2], (float)actual_h[3] * (1.0f / 255.0f)),
				trial_blk_endpoints, trial_blk_weights, trial_blk_error);

			if (trial_blk_error >= best_blk_error)
				break;

			best_blk_error = trial_blk_error;
			memcpy(best_blk_endpoints, trial_blk_endpoints, total_endpoint_vals);
			memcpy(best_blk_weights, trial_blk_weights, total_weights);

		} // pass

		if (cem_has_alpha)
		{
			// Try to refine low a/high given the current selectors.
			float bounds_min = pixel_stats.m_min_f[3];
			float bounds_max = pixel_stats.m_max_f[3];
			if (bounds_min != bounds_max)
			{
				float a_vals[ASTC_LDR_MAX_BLOCK_PIXELS];
				for (uint32_t i = 0; i < pixel_stats.m_num_pixels; i++)
					a_vals[i] = pixel_stats.m_pixels_f[i][3];

				const uint32_t TOTAL_PASSES = 1;
				for (uint32_t pass = 0; pass < TOTAL_PASSES; pass++)
				{
					float xl = 0.0f, xh = 0.0f;

					if (compute_least_squares_endpoints_1D(
						pixel_stats.m_num_pixels, best_blk_weights, get_ls_weights_ise(weight_ise_range),
						&xl, &xh, a_vals, bounds_min, bounds_max))
					{
						color_rgba actual_l(0), actual_h(0);
						float actual_scale = 0;
						decode_endpoints(cem_index, trial_blk_endpoints, endpoint_ise_range, actual_l, actual_h, &actual_scale);

						try_cem6_10(
							cem_index,
							pixel_stats, enc_params,
							endpoint_ise_range, weight_ise_range,
							actual_scale, xl, vec4F(actual_h[0], actual_h[1], actual_h[2], xh),
							trial_blk_endpoints, trial_blk_weights, trial_blk_error);

						if (trial_blk_error < best_blk_error)
						{
							best_blk_error = trial_blk_error;
							memcpy(best_blk_endpoints, trial_blk_endpoints, total_endpoint_vals);
							memcpy(best_blk_weights, trial_blk_weights, total_weights);
						}
						else
						{
							break;
						}
					}
					else
					{
						break;
					}
				} // pass
			}
		}

		if (best_blk_error < cur_blk_error)
		{
			cur_blk_error = best_blk_error;
			memcpy(pEndpoint_vals, trial_blk_endpoints, total_endpoint_vals);
			memcpy(pWeight_vals, trial_blk_weights, total_weights);
		}

		return cur_blk_error;
	}

	// rgba base+scale, dual plane a, ccs_index must be 3
	static uint64_t encode_cem10_dp_a(
		uint32_t cem_index,
		const pixel_stats_t& pixel_stats, const cem_encode_params& enc_params,
		uint32_t endpoint_ise_range, uint32_t weight_ise_range,
		uint8_t* pEndpoint_vals, uint8_t* pWeight_vals0, uint8_t* pWeight_vals1, uint64_t cur_blk_error)
	{
		assert(g_initialized);
		assert(cem_index == astc_helpers::CEM_LDR_RGB_BASE_SCALE_PLUS_TWO_A);
		assert((pixel_stats.m_num_pixels) && (pixel_stats.m_num_pixels <= ASTC_LDR_MAX_BLOCK_PIXELS));
		assert((endpoint_ise_range >= astc_helpers::FIRST_VALID_ENDPOINT_ISE_RANGE) && (endpoint_ise_range <= astc_helpers::LAST_VALID_ENDPOINT_ISE_RANGE));
		assert(((weight_ise_range >= astc_helpers::FIRST_VALID_WEIGHT_ISE_RANGE) && (weight_ise_range <= astc_helpers::LAST_VALID_WEIGHT_ISE_RANGE)) || (weight_ise_range == astc_helpers::BISE_64_LEVELS));

		// RGB uses plane0, alpha plane1. So solve RGB first.
		uint8_t rgba_endpoint_vals[astc_helpers::NUM_MODE10_ENDPOINTS] = { 0 };
		uint8_t rgb_weight_vals[ASTC_LDR_MAX_BLOCK_PIXELS] = { 0 };
		uint8_t a_weight_vals[ASTC_LDR_MAX_BLOCK_PIXELS] = { 0 };

		// First just solve RGB, single plane.
		uint64_t rgb_blk_error = encode_cem6_10(
			astc_helpers::CEM_LDR_RGB_BASE_SCALE,
			pixel_stats, enc_params,
			endpoint_ise_range, weight_ise_range,
			rgba_endpoint_vals, rgb_weight_vals, UINT64_MAX);

		assert(rgb_blk_error != UINT64_MAX);

		if (rgb_blk_error == UINT64_MAX)
			return cur_blk_error;

		const auto& endpoint_quant_tab = astc_helpers::g_dequant_tables.get_endpoint_tab(endpoint_ise_range).m_val_to_ise;

		rgba_endpoint_vals[4] = endpoint_quant_tab[pixel_stats.m_min[3]];
		rgba_endpoint_vals[5] = endpoint_quant_tab[pixel_stats.m_max[3]];

		uint64_t rgba_blk_error = eval_solution_dp(
			pixel_stats,
			cem_index, 3,
			rgba_endpoint_vals, endpoint_ise_range,
			rgb_weight_vals, a_weight_vals, weight_ise_range,
			enc_params);

		assert(rgba_blk_error != UINT64_MAX);

		if (rgba_blk_error < cur_blk_error)
		{
			cur_blk_error = rgba_blk_error;
			memcpy(pEndpoint_vals, rgba_endpoint_vals, astc_helpers::NUM_MODE10_ENDPOINTS);
			memcpy(pWeight_vals0, rgb_weight_vals, pixel_stats.m_num_pixels);
			memcpy(pWeight_vals1, a_weight_vals, pixel_stats.m_num_pixels);

			if (!cur_blk_error)
				return cur_blk_error;
		}

		float bounds_min = pixel_stats.m_min_f[3], bounds_max = pixel_stats.m_max_f[3];
		if (bounds_min != bounds_max)
		{
			float a_vals[ASTC_LDR_MAX_BLOCK_PIXELS];
			for (uint32_t i = 0; i < pixel_stats.m_num_pixels; i++)
				a_vals[i] = pixel_stats.m_pixels_f[i][3];

			const uint32_t TOTAL_PASSES = 2;

			uint8_t trial_rgba_endpoint_vals[astc_helpers::NUM_MODE10_ENDPOINTS] = { 0 };
			uint8_t trial_rgb_weight_vals[ASTC_LDR_MAX_BLOCK_PIXELS] = { 0 };
			uint8_t trial_a_weight_vals[ASTC_LDR_MAX_BLOCK_PIXELS] = { 0 };

			for (uint32_t pass = 0; pass < TOTAL_PASSES; pass++)
			{
				float xl = 0.0f, xh = 0.0f;

				if (compute_least_squares_endpoints_1D(
					pixel_stats.m_num_pixels, pass ? trial_a_weight_vals : a_weight_vals, get_ls_weights_ise(weight_ise_range),
					&xl, &xh, a_vals, bounds_min, bounds_max))
				{
					memcpy(trial_rgba_endpoint_vals, rgba_endpoint_vals, astc_helpers::NUM_MODE10_ENDPOINTS);

					trial_rgba_endpoint_vals[4] = precise_round_bise_endpoint_val(xl, endpoint_ise_range);
					trial_rgba_endpoint_vals[5] = precise_round_bise_endpoint_val(xh, endpoint_ise_range);

					uint64_t trial_rgba_blk_error = eval_solution_dp(
						pixel_stats,
						cem_index, 3,
						trial_rgba_endpoint_vals, endpoint_ise_range,
						trial_rgb_weight_vals, trial_a_weight_vals, weight_ise_range,
						enc_params);

					assert(trial_rgba_blk_error != UINT64_MAX);

					if (trial_rgba_blk_error < cur_blk_error)
					{
						cur_blk_error = trial_rgba_blk_error;
						memcpy(pEndpoint_vals, trial_rgba_endpoint_vals, astc_helpers::NUM_MODE10_ENDPOINTS);
						memcpy(pWeight_vals0, trial_rgb_weight_vals, pixel_stats.m_num_pixels);
						memcpy(pWeight_vals1, trial_a_weight_vals, pixel_stats.m_num_pixels);
					}
					else
					{
						break;
					}
				}
				else
				{
					break;
				}
			} // pass
		}

		return cur_blk_error;
	}

	// rgb/rgba base+scale, dual plane rgb (not a!)
	static uint64_t encode_cem6_10_dp_rgb(
		uint32_t cem_index, uint32_t ccs_index,
		const pixel_stats_t& pixel_stats, const cem_encode_params& enc_params,
		uint32_t endpoint_ise_range, uint32_t weight_ise_range,
		uint8_t* pEndpoint_vals, uint8_t* pWeight_vals0, uint8_t* pWeight_vals1, uint64_t cur_blk_error)
	{
		assert(g_initialized);
		assert((cem_index == astc_helpers::CEM_LDR_RGB_BASE_SCALE) || (cem_index == astc_helpers::CEM_LDR_RGB_BASE_SCALE_PLUS_TWO_A));
		assert(ccs_index <= 2);
		assert((pixel_stats.m_num_pixels) && (pixel_stats.m_num_pixels <= ASTC_LDR_MAX_BLOCK_PIXELS));
		assert((endpoint_ise_range >= astc_helpers::FIRST_VALID_ENDPOINT_ISE_RANGE) && (endpoint_ise_range <= astc_helpers::LAST_VALID_ENDPOINT_ISE_RANGE));
		assert(((weight_ise_range >= astc_helpers::FIRST_VALID_WEIGHT_ISE_RANGE) && (weight_ise_range <= astc_helpers::LAST_VALID_WEIGHT_ISE_RANGE)) || (weight_ise_range == astc_helpers::BISE_64_LEVELS));
		assert(pWeight_vals0 && pWeight_vals1);

		// First solve using a single plane, then we'll introduce the other plane's weights and tune the encoded H/s values
		uint8_t sp_endpoint_vals[astc_helpers::NUM_MODE10_ENDPOINTS] = { 0 };
		uint8_t sp_weight_vals[ASTC_LDR_MAX_BLOCK_PIXELS] = { 0 };

		uint64_t sp_block_err = encode_cem6_10(
			cem_index,
			pixel_stats, enc_params,
			endpoint_ise_range, weight_ise_range,
			sp_endpoint_vals, sp_weight_vals, UINT64_MAX);

		assert(sp_block_err != UINT64_MAX);
		BASISU_NOTE_UNUSED(sp_block_err);

		// Now compute both plane's weights using the initial H/s values
		uint8_t trial_weights0_vals[ASTC_LDR_MAX_BLOCK_PIXELS] = { 0 };
		uint8_t trial_weights1_vals[ASTC_LDR_MAX_BLOCK_PIXELS] = { 0 };
		uint64_t dp_blk_error = eval_solution_dp(
			pixel_stats,
			cem_index, ccs_index,
			sp_endpoint_vals, endpoint_ise_range,
			trial_weights0_vals, trial_weights1_vals, weight_ise_range,
			enc_params);

		if (dp_blk_error < cur_blk_error)
		{
			cur_blk_error = dp_blk_error;
			memcpy(pEndpoint_vals, sp_endpoint_vals, astc_helpers::NUM_MODE10_ENDPOINTS);
			memcpy(pWeight_vals0, trial_weights0_vals, pixel_stats.m_num_pixels);
			memcpy(pWeight_vals1, trial_weights1_vals, pixel_stats.m_num_pixels);

			if (!cur_blk_error)
				return cur_blk_error;
		}

		// Compute refined H/s values using the current weights.
		const float MAX_S = 255.0f / 256.0f;
		const float EPS = 1e-6f;

		vec3F Pa(0.0f); // (Pa_r,Pa_g,Pa_b)
		vec3F Pb(0.0f); // (Pb_r,Pb_g,Pb_b)
		float A[3] = { 0 }, B[3] = { 0 }, C[3] = { 0 }; // per-channel

		// invalid on raw weights
		const auto& dequant_weights_tab = astc_helpers::g_dequant_tables.get_weight_tab(minimum<uint32_t>(astc_helpers::BISE_32_LEVELS, weight_ise_range)).m_ISE_to_val;

		for (uint32_t i = 0; i < pixel_stats.m_num_pixels; i++)
		{
			float w0, w1;
			if (weight_ise_range == astc_helpers::BISE_64_LEVELS)
			{
				w0 = (float)trial_weights0_vals[i] * (1.0f / 64.0f);
				w1 = (float)trial_weights1_vals[i] * (1.0f / 64.0f);
			}
			else
			{
				w0 = dequant_weights_tab[trial_weights0_vals[i]] * (1.0f / 64.0f);
				w1 = dequant_weights_tab[trial_weights1_vals[i]] * (1.0f / 64.0f);
			}

			float w[3] = { w0, w0, w0 };
			w[ccs_index] = w1;

			const vec3F& p = pixel_stats.m_pixels_f[i];

			for (int c = 0; c < 3; ++c)
			{
				const float a = 1.0f - w[c];
				const float b = w[c];

				Pa[c] += a * p[c];
				Pb[c] += b * p[c];
				A[c] += a * a;
				B[c] += a * b;
				C[c] += b * b;
			} // c
		} // i

		color_rgba actual_l(0), actual_h(0);
		float actual_scale = 0;
		decode_endpoints(cem_index, sp_endpoint_vals, endpoint_ise_range, actual_l, actual_h, &actual_scale);

		vec3F H((float)actual_h[0], (float)actual_h[1], (float)actual_h[2]);
		H *= (1.0f / 255.0f);

		const float S1 = H[0] * Pa[0] + H[1] * Pa[1] + H[2] * Pa[2];
		float S2 = 0.0f, S3 = 0.0f;
		for (int c = 0; c < 3; c++)
		{
			const float H2 = H[c] * H[c];
			S2 += H2 * A[c];
			S3 += H2 * B[c];
		}

		float new_s = actual_scale;
		if (S2 > EPS)
			new_s = (S1 - S3) / S2;

		new_s = clamp(new_s, 0.0f, MAX_S);

		vec3F new_H(0.0f);
		for (int c = 0; c < 3; ++c)
		{
			const float den = A[c] * new_s * new_s + 2.0f * B[c] * new_s + C[c];

			float Hc = 0.0f;
			if (den > EPS)
			{
				const float num = Pb[c] + new_s * Pa[c];
				Hc = num / den;
			}
			new_H[c] = Hc;
		}

		bool improved_flag = try_cem6_10_dp(
			cem_index, ccs_index,
			pixel_stats, enc_params,
			endpoint_ise_range, weight_ise_range,
			new_s, (float)actual_l[3] * (1.0f / 255.0f), vec4F(new_H[0], new_H[1], new_H[2], (float)actual_h[3] * (1.0f / 255.0f)),
			pEndpoint_vals, pWeight_vals0, pWeight_vals1, cur_blk_error);
		(void)improved_flag;

		return cur_blk_error;
	}

	// dispatcher
	uint64_t cem_encode_pixels(
		uint32_t cem_index, int ccs_index,
		const pixel_stats_t& pixel_stats, const cem_encode_params& enc_params,
		uint32_t endpoint_ise_range, uint32_t weight_ise_range,
		uint8_t* pEndpoint_vals, uint8_t* pWeight_vals0, uint8_t* pWeight_vals1, uint64_t cur_blk_error,
		bool use_blue_contraction, bool *pBase_ofs_clamped_flag)
	{
		assert(g_initialized);
		assert((ccs_index >= -1) && (ccs_index <= 3));
		assert(astc_helpers::is_cem_ldr(cem_index));
		assert(pEndpoint_vals);
		assert(pWeight_vals0);

		const bool dual_plane = (ccs_index >= 0);
		
		if (pBase_ofs_clamped_flag)
			*pBase_ofs_clamped_flag = false;

		uint64_t blk_error = UINT64_MAX;

		switch (cem_index)
		{
		case astc_helpers::CEM_LDR_LUM_DIRECT:
		{
			assert(!dual_plane);

			blk_error = encode_cem0_4(
				cem_index,
				pixel_stats, enc_params,
				endpoint_ise_range, weight_ise_range,
				pEndpoint_vals, pWeight_vals0, cur_blk_error);

			break;
		}
		case astc_helpers::CEM_LDR_LUM_ALPHA_DIRECT:
		{
			if (dual_plane)
			{
				assert(ccs_index == 3);
				assert(pWeight_vals1);

				blk_error = encode_cem4_dp_a(
					cem_index,
					pixel_stats, enc_params,
					endpoint_ise_range, weight_ise_range,
					pEndpoint_vals, pWeight_vals0, pWeight_vals1, cur_blk_error);
			}
			else
			{
				blk_error = encode_cem0_4(
					cem_index,
					pixel_stats, enc_params,
					endpoint_ise_range, weight_ise_range,
					pEndpoint_vals, pWeight_vals0, cur_blk_error);
			}
			break;
		}

		case astc_helpers::CEM_LDR_RGB_DIRECT:
		case astc_helpers::CEM_LDR_RGBA_DIRECT:
		case astc_helpers::CEM_LDR_RGB_BASE_PLUS_OFFSET:
		case astc_helpers::CEM_LDR_RGBA_BASE_PLUS_OFFSET:
		{
			if (dual_plane)
			{
				assert(pWeight_vals1);
				blk_error = encode_cem8_12_9_13_dp(
					cem_index, ccs_index,
					pixel_stats, enc_params,
					endpoint_ise_range, weight_ise_range,
					pEndpoint_vals, pWeight_vals0, pWeight_vals1, cur_blk_error, use_blue_contraction, pBase_ofs_clamped_flag);
			}
			else
			{
				blk_error = encode_cem8_12_9_13(
					cem_index,
					pixel_stats, enc_params,
					endpoint_ise_range, weight_ise_range,
					pEndpoint_vals, pWeight_vals0, cur_blk_error, use_blue_contraction, pBase_ofs_clamped_flag);
			}
			break;
		}
		case astc_helpers::CEM_LDR_RGB_BASE_SCALE:
		{
			if (dual_plane)
			{
				assert(ccs_index <= 2);
				assert(pWeight_vals1);

				blk_error = encode_cem6_10_dp_rgb(
					cem_index, ccs_index,
					pixel_stats, enc_params,
					endpoint_ise_range, weight_ise_range,
					pEndpoint_vals, pWeight_vals0, pWeight_vals1, cur_blk_error);
			}
			else
			{
				blk_error = encode_cem6_10(
					cem_index,
					pixel_stats, enc_params,
					endpoint_ise_range, weight_ise_range,
					pEndpoint_vals, pWeight_vals0, cur_blk_error);
			}
			break;
		}
		case astc_helpers::CEM_LDR_RGB_BASE_SCALE_PLUS_TWO_A:
		{
			if (dual_plane)
			{
				assert(pWeight_vals1);

				if (ccs_index == 3)
				{
					blk_error = encode_cem10_dp_a(
						cem_index,
						pixel_stats, enc_params,
						endpoint_ise_range, weight_ise_range,
						pEndpoint_vals, pWeight_vals0, pWeight_vals1, cur_blk_error);
				}
				else
				{
					blk_error = encode_cem6_10_dp_rgb(
						cem_index, ccs_index,
						pixel_stats, enc_params,
						endpoint_ise_range, weight_ise_range,
						pEndpoint_vals, pWeight_vals0, pWeight_vals1, cur_blk_error);
				}
			}
			else
			{
				blk_error = encode_cem6_10(
					cem_index,
					pixel_stats, enc_params,
					endpoint_ise_range, weight_ise_range,
					pEndpoint_vals, pWeight_vals0, cur_blk_error);
			}
			break;
		}
		default:
		{
			assert(0);
			break;
		}
		}

		return blk_error;
	}

	//---------------------------------------------------------------------------------------------

	float surrogate_evaluate_rgba_sp(const pixel_stats_t& ps, const vec4F& l, const vec4F& h, float* pWeights0, uint32_t num_weight_levels, 
		const cem_encode_params& enc_params, uint32_t flags)
	{
		assert(g_initialized);
		assert((ps.m_num_pixels) && (ps.m_num_pixels <= ASTC_LDR_MAX_BLOCK_PIXELS));
		assert(pWeights0);

		const float wr = (float)enc_params.m_comp_weights[0], wg = (float)enc_params.m_comp_weights[1],
			wb = (float)enc_params.m_comp_weights[2], wa = (float)enc_params.m_comp_weights[3];

		float total_err = 0;
				
		const bool compute_error = ((flags & cFlagNoError) == 0);

		float lr = l[0], lg = l[1], lb = l[2], la = l[3];
		float dr = h[0] - lr, dg = h[1] - lg, db = h[2] - lb, da = h[3] - la;
		float delta_col_nrm = dr * dr + dg * dg + db * db + da * da;

		if (flags & cFlagDisableQuant)
		{
			float f = (float)1.0f / (delta_col_nrm + REALLY_SMALL_FLOAT_VAL);

			lr *= -dr; lg *= -dg; lb *= -db; la *= -da;

			dr *= f; dg *= f; db *= f; da *= f;
			float l_sum = (lr + lg + lb + la) * f;

			for (uint32_t i = 0; i < ps.m_num_pixels; i++)
			{
				const vec4F& p = ps.m_pixels_f[i];
				const float r = p[0], g = p[1], b = p[2], a = p[3];

				float w = r * dr + g * dg + b * db + a * da + l_sum;

				if (w < 0.0f)
					w = 0.0f;
				else if (w > 1.0f)
					w = 1.0f;

				pWeights0[i] = w;

				if (compute_error)
				{
					float one_minus_w = 1.0f - w;

					float dec_r = l[0] * one_minus_w + h[0] * w;
					float dec_g = l[1] * one_minus_w + h[1] * w;
					float dec_b = l[2] * one_minus_w + h[2] * w;
					float dec_a = l[3] * one_minus_w + h[3] * w;

					float diff_r = r - dec_r;
					float diff_g = g - dec_g;
					float diff_b = b - dec_b;
					float diff_a = a - dec_a;

					total_err += (wr * diff_r * diff_r) + (wg * diff_g * diff_g) + (wb * diff_b * diff_b) + (wa * diff_a * diff_a);
				}

			} // i
		}
		else
		{
			const float inv_weight_levels = 1.0f / (float)(num_weight_levels - 1);
						
			float f = (float)(num_weight_levels - 1) / (delta_col_nrm + REALLY_SMALL_FLOAT_VAL);

			lr *= -dr; lg *= -dg; lb *= -db; la *= -da;

			dr *= f; dg *= f; db *= f; da *= f;
			float l_sum_biased = (lr + lg + lb + la) * f + .5f;
						
			for (uint32_t i = 0; i < ps.m_num_pixels; i++)
			{
				const vec4F& p = ps.m_pixels_f[i];
				const float r = p[0], g = p[1], b = p[2], a = p[3];

				float w = (float)fast_floorf_int(r * dr + g * dg + b * db + a * da + l_sum_biased) * inv_weight_levels;

				if (w < 0.0f)
					w = 0.0f;
				else if (w > 1.0f)
					w = 1.0f;

				pWeights0[i] = w;

				if (compute_error)
				{
					float one_minus_w = 1.0f - w;

					float dec_r = l[0] * one_minus_w + h[0] * w;
					float dec_g = l[1] * one_minus_w + h[1] * w;
					float dec_b = l[2] * one_minus_w + h[2] * w;
					float dec_a = l[3] * one_minus_w + h[3] * w;

					float diff_r = r - dec_r;
					float diff_g = g - dec_g;
					float diff_b = b - dec_b;
					float diff_a = a - dec_a;

					total_err += (wr * diff_r * diff_r) + (wg * diff_g * diff_g) + (wb * diff_b * diff_b) + (wa * diff_a * diff_a);
				}

			} // i
		}

		return total_err;

	}

	float surrogate_evaluate_rgba_dp(uint32_t ccs_index, const pixel_stats_t& ps, const vec4F& l, const vec4F& h, float* pWeights0, float* pWeights1, uint32_t num_weight_levels, 
		const cem_encode_params& enc_params, uint32_t flags)
	{
		assert(g_initialized);
		assert((ccs_index >= 0) && (ccs_index <= 3));
		assert((ps.m_num_pixels) && (ps.m_num_pixels <= ASTC_LDR_MAX_BLOCK_PIXELS));
		assert(pWeights0 && pWeights1);

		const float inv_weight_levels = 1.0f / (float)(num_weight_levels - 1);

		const uint32_t c0 = (ccs_index + 1) & 3, c1 = (ccs_index + 2) & 3, c2 = (ccs_index + 3) & 3;

		const float orig_lx = l[c0], orig_ly = l[c1], orig_lz = l[c2], orig_lw = l[ccs_index];
		const float orig_hx = h[c0], orig_hy = h[c1], orig_hz = h[c2], orig_hw = h[ccs_index];
		
		const float wx = (float)enc_params.m_comp_weights[c0], wy = (float)enc_params.m_comp_weights[c1],
			wz = (float)enc_params.m_comp_weights[c2], ww = (float)enc_params.m_comp_weights[ccs_index];
		
		float total_err = 0;

		const bool compute_error = ((flags & cFlagNoError) == 0);
				
		if (flags & cFlagDisableQuant)
		{
			// Plane 0
			{
				float dx = orig_hx - orig_lx, dy = orig_hy - orig_ly, dz = orig_hz - orig_lz;

				float delta_col_nrm = dx * dx + dy * dy + dz * dz;

				float f = (float)1.0f / (delta_col_nrm + REALLY_SMALL_FLOAT_VAL);

				float lx = orig_lx, ly = orig_ly, lz = orig_lz;
				lx *= -dx; ly *= -dy; lz *= -dz;

				dx *= f; dy *= f; dz *= f;
				float l_sum = (lx + ly + lz) * f;

				for (uint32_t i = 0; i < ps.m_num_pixels; i++)
				{
					const vec4F& p = ps.m_pixels_f[i];
					const float x = p[c0], y = p[c1], z = p[c2];

					float weight = x * dx + y * dy + z * dz + l_sum;

					if (weight < 0.0f)
						weight = 0.0f;
					else if (weight > 1.0f)
						weight = 1.0f;

					pWeights0[i] = weight;

					if (compute_error)
					{
						float one_minus_weight = 1.0f - weight;

						float dec_x = orig_lx * one_minus_weight + orig_hx * weight;
						float dec_y = orig_ly * one_minus_weight + orig_hy * weight;
						float dec_z = orig_lz * one_minus_weight + orig_hz * weight;

						float diff_x = x - dec_x;
						float diff_y = y - dec_y;
						float diff_z = z - dec_z;

						total_err += (wx * diff_x * diff_x) + (wy * diff_y * diff_y) + (wz * diff_z * diff_z);
					}

				} // i
			}

			// Plane 1
			{
				const float delta_w = orig_hw - orig_lw;
				const float f = (fabsf(delta_w) > REALLY_SMALL_FLOAT_VAL) ? (1.0f / delta_w) : 0.0f;

				for (uint32_t i = 0; i < ps.m_num_pixels; i++)
				{
					const vec4F& p = ps.m_pixels_f[i];
					const float w = p[ccs_index];

					float weight = (w - orig_lw) * f;

					if (weight < 0.0f)
						weight = 0.0f;
					else if (weight > 1.0f)
						weight = 1.0f;

					pWeights1[i] = weight;

					if (compute_error)
					{
						// Error for DP here is 0 if there's no quant and L/H are sufficient to cover the entire span.
						if ((w < orig_lw) || (w > orig_hw))
						{
							float one_minus_weight = 1.0f - weight;

							float dec_w = orig_lw * one_minus_weight + orig_hw * weight;

							float diff_w = w - dec_w;

							total_err += (ww * diff_w * diff_w);
						}
					}

				} // i
			}
		}
		else
		{
			// Plane 0
			{
				float dx = orig_hx - orig_lx, dy = orig_hy - orig_ly, dz = orig_hz - orig_lz;

				float delta_col_nrm = dx * dx + dy * dy + dz * dz;

				float f = (float)(num_weight_levels - 1) / (delta_col_nrm + REALLY_SMALL_FLOAT_VAL);

				float lx = orig_lx, ly = orig_ly, lz = orig_lz;
				lx *= -dx; ly *= -dy; lz *= -dz;

				dx *= f; dy *= f; dz *= f;
				float l_sum_biased = (lx + ly + lz) * f + .5f;

				for (uint32_t i = 0; i < ps.m_num_pixels; i++)
				{
					const vec4F& p = ps.m_pixels_f[i];
					const float x = p[c0], y = p[c1], z = p[c2];

					float weight = (float)fast_floorf_int(x * dx + y * dy + z * dz + l_sum_biased) * inv_weight_levels;

					if (weight < 0.0f)
						weight = 0.0f;
					else if (weight > 1.0f)
						weight = 1.0f;

					pWeights0[i] = weight;

					if (compute_error)
					{
						float one_minus_weight = 1.0f - weight;

						float dec_x = orig_lx * one_minus_weight + orig_hx * weight;
						float dec_y = orig_ly * one_minus_weight + orig_hy * weight;
						float dec_z = orig_lz * one_minus_weight + orig_hz * weight;

						float diff_x = x - dec_x;
						float diff_y = y - dec_y;
						float diff_z = z - dec_z;

						total_err += (wx * diff_x * diff_x) + (wy * diff_y * diff_y) + (wz * diff_z * diff_z);
					}

				} // i
			}

			// Plane 1
			{
				const float delta_w = orig_hw - orig_lw;
				const float f = (fabs(delta_w) > REALLY_SMALL_FLOAT_VAL) ? ((float)(num_weight_levels - 1) / delta_w) : 0.0f;

				for (uint32_t i = 0; i < ps.m_num_pixels; i++)
				{
					const vec4F& p = ps.m_pixels_f[i];
					const float w = p[ccs_index];

					float weight = (float)fast_floorf_int((w - orig_lw) * f + .5f) * inv_weight_levels;

					if (weight < 0.0f)
						weight = 0.0f;
					else if (weight > 1.0f)
						weight = 1.0f;

					pWeights1[i] = weight;

					if (compute_error)
					{
						float one_minus_weight = 1.0f - weight;

						float dec_w = orig_lw * one_minus_weight + orig_hw * weight;

						float diff_w = w - dec_w;

						total_err += (ww * diff_w * diff_w);
					}

				} // i
			}
		}

		return total_err;
	}

	//---------------------------------------------------------------------------------------------

	float surrogate_quant_endpoint_val(float e, uint32_t num_endpoint_levels, uint32_t flags)
	{
		assert((e >= 0.0f) && (e <= 1.0f));

		if (flags & cFlagDisableQuant)
			return e;

		const float endpoint_levels_minus_1 = (float)(num_endpoint_levels - 1);
		const float inv_endpoint_levels = 1.0f / endpoint_levels_minus_1;
		return (float)fast_roundf_pos_int(e * endpoint_levels_minus_1) * inv_endpoint_levels;
	}

	vec4F surrogate_quant_endpoint(const vec4F& e, uint32_t num_endpoint_levels, uint32_t flags)
	{
		if (flags & cFlagDisableQuant)
			return e;

		const float endpoint_levels_minus_1 = (float)(num_endpoint_levels - 1);
		const float inv_endpoint_levels = 1.0f / endpoint_levels_minus_1;

		assert((e[0] >= 0.0f) && (e[0] <= 1.0f));
		assert((e[1] >= 0.0f) && (e[1] <= 1.0f));
		assert((e[2] >= 0.0f) && (e[2] <= 1.0f));
		assert((e[3] >= 0.0f) && (e[3] <= 1.0f));

		vec4F res;
		res[0] = (float)fast_roundf_pos_int(e[0] * endpoint_levels_minus_1) * inv_endpoint_levels;
		res[1] = (float)fast_roundf_pos_int(e[1] * endpoint_levels_minus_1) * inv_endpoint_levels;
		res[2] = (float)fast_roundf_pos_int(e[2] * endpoint_levels_minus_1) * inv_endpoint_levels;
		res[3] = (float)fast_roundf_pos_int(e[3] * endpoint_levels_minus_1) * inv_endpoint_levels;

		return res;
	}

	static uint32_t get_num_weight_levels(uint32_t weight_ise_range)
	{
		// astc_helpers::BISE_64_LEVELS=raw weights ([0,64], NOT [0,63])
		const uint32_t num_weight_levels = (weight_ise_range == astc_helpers::BISE_64_LEVELS) ? 65 : astc_helpers::get_ise_levels(weight_ise_range);
		return num_weight_levels;
	}

	//---------------------------------------------------------------------------------------------

	static float cem_surrogate_encode_cem6_10_sp(
		uint32_t cem_index,
		const pixel_stats_t& ps, const cem_encode_params& enc_params,
		uint32_t endpoint_ise_range, uint32_t weight_ise_range,
		vec4F& low_endpoint, vec4F& high_endpoint, float &s, float* pWeights0, uint32_t flags)
	{
		const uint32_t num_endpoint_levels = astc_helpers::get_ise_levels(endpoint_ise_range);

		// astc_helpers::BISE_64_LEVELS=raw weights ([0,64], NOT [0,63])
		const uint32_t num_weight_levels = get_num_weight_levels(weight_ise_range);

		const bool cem_has_alpha = (cem_index == astc_helpers::CEM_LDR_RGB_BASE_SCALE_PLUS_TWO_A);

		float d_min = BIG_FLOAT_VAL, d_max = -BIG_FLOAT_VAL;
		
		for (uint32_t i = 0; i < ps.m_num_pixels; i++)
		{
			const vec4F p(ps.m_pixels_f[i]);

			float dot = p.dot3(ps.m_zero_rel_axis3);

			if (dot < d_min)
				d_min = dot;

			if (dot > d_max)
				d_max = dot;
		}

		vec3F low_color3_f(d_min * ps.m_zero_rel_axis3);
		low_color3_f.clamp(0.0f, 1.0f);

		vec3F high_color3_f(d_max * ps.m_zero_rel_axis3);
		high_color3_f.clamp(0.0f, 1.0f);

		const float MAX_S = 255.0f / 256.0f;
		
		float scale = MAX_S;

		float d = low_color3_f.dot(high_color3_f);
		float nrm = high_color3_f.norm();
		if (nrm > 0.0f)
			scale = d / nrm;
				
		scale = clamp(scale, 0.0f, MAX_S);
		
		scale = surrogate_quant_endpoint_val(scale * (256.0f / 255.0f), num_endpoint_levels, flags);

		s = scale;

		high_endpoint = surrogate_quant_endpoint(vec4F(high_color3_f[0], high_color3_f[1], high_color3_f[2], cem_has_alpha ? ps.m_max_f[3] : 1.0f), num_endpoint_levels, flags);

		low_endpoint = vec4F(high_endpoint[0] * scale, high_endpoint[1] * scale, high_endpoint[2] * scale, cem_has_alpha ? ps.m_min_f[3] : 1.0f);

		return surrogate_evaluate_rgba_sp(ps, low_endpoint, high_endpoint, pWeights0, num_weight_levels, enc_params, flags);
	}

	static float cem_surrogate_encode_cem6_10_dp(
		uint32_t cem_index, uint32_t ccs_index,
		const pixel_stats_t& ps, const cem_encode_params& enc_params,
		uint32_t endpoint_ise_range, uint32_t weight_ise_range,
		vec4F& low_endpoint, vec4F& high_endpoint, float& s, float* pWeights0, float* pWeights1, uint32_t flags)
	{
		const bool cem_has_alpha = (cem_index == astc_helpers::CEM_LDR_RGB_BASE_SCALE_PLUS_TWO_A);
		BASISU_NOTE_UNUSED(cem_has_alpha);
		
		// astc_helpers::BISE_64_LEVELS=raw weights ([0,64], NOT [0,63])
		const uint32_t num_weight_levels = get_num_weight_levels(weight_ise_range);

		assert(cem_has_alpha || (ccs_index <= 2));
				
		float temp_weights[astc_ldr::ASTC_LDR_MAX_BLOCK_PIXELS];
		cem_surrogate_encode_cem6_10_sp(
			(ccs_index == 3) ? (uint32_t)astc_helpers::CEM_LDR_RGB_BASE_SCALE : cem_index,
			ps, enc_params, endpoint_ise_range, weight_ise_range, low_endpoint, high_endpoint, s, temp_weights, flags);

		if (ccs_index == 3)
		{
			low_endpoint[3] = ps.m_min_f[3];
			high_endpoint[3] = ps.m_max_f[3];
		}

		return surrogate_evaluate_rgba_dp(ccs_index, ps, low_endpoint, high_endpoint, pWeights0, pWeights1, num_weight_levels, enc_params, flags);
	}

	static float cem_surrogate_encode_cem8_12_sp(
		uint32_t cem_index,
		const pixel_stats_t& ps, const cem_encode_params& enc_params,
		uint32_t endpoint_ise_range, uint32_t weight_ise_range,
		vec4F& low_endpoint, vec4F& high_endpoint, float* pWeights0, uint32_t flags)
	{
		const uint32_t num_endpoint_levels = astc_helpers::get_ise_levels(endpoint_ise_range);
		
		// astc_helpers::BISE_64_LEVELS=raw weights ([0,64], NOT [0,63])
		const uint32_t num_weight_levels = get_num_weight_levels(weight_ise_range);

		const bool cem_has_alpha = (cem_index == astc_helpers::CEM_LDR_RGBA_DIRECT);
		const uint32_t num_comps = cem_has_alpha ? 4 : 3;

		float d_min = BIG_FLOAT_VAL, d_max = -BIG_FLOAT_VAL;
		uint32_t l_idx = 0, h_idx = 0;

		for (uint32_t i = 0; i < ps.m_num_pixels; i++)
		{
			const vec4F p(ps.m_pixels_f[i] - ps.m_mean_f);

			float dot = cem_has_alpha ? p.dot(ps.m_mean_rel_axis4) : p.dot3(ps.m_mean_rel_axis3);

			if (dot < d_min)
			{
				d_min = dot;
				l_idx = i;
			}

			if (dot > d_max)
			{
				d_max = dot;
				h_idx = i;
			}
		}
								
		low_endpoint = surrogate_quant_endpoint(ps.m_pixels_f[l_idx], num_endpoint_levels, flags);
		high_endpoint = surrogate_quant_endpoint(ps.m_pixels_f[h_idx], num_endpoint_levels, flags);

		if (!cem_has_alpha)
		{
			low_endpoint[3] = 1.0f;
			high_endpoint[3] = 1.0f;
		}

		if (low_endpoint.dot(vec4F(1.0f)) > high_endpoint.dot(vec4F(1.0f)))
			std::swap(low_endpoint, high_endpoint);
		
		if ((flags & cFlagDisableQuant) == 0)
		{
			for (uint32_t i = 0; i < num_comps; i++)
			{
				if ((low_endpoint[i] == high_endpoint[i]) && (ps.m_min_f[i] != ps.m_max_f[i]))
				{
					const float inv_endpoint_levels = 1.0f / (float)(num_endpoint_levels - 1);

					float best_dist = BIG_FLOAT_VAL;
					float best_l = 0.0f, best_h = 0.0f;

					for (int ld = -2; ld <= 0; ld++)
					{
						float actual_l = saturate(low_endpoint[i] + (float)ld * inv_endpoint_levels);

						for (int hd = 0; hd <= 2; hd++)
						{
							float actual_h = saturate(high_endpoint[i] + (float)hd * inv_endpoint_levels);

							float v0 = lerp(actual_l, actual_h, 1.0f / 3.0f);
							float v1 = lerp(actual_l, actual_h, 2.0f / 3.0f);
							assert(v0 <= v1);

							float dist0 = v0 - ps.m_min_f[0];
							float dist1 = v1 - ps.m_max_f[0];

							float total_dist = dist0 * dist0 + dist1 * dist1;
							if (total_dist < best_dist)
							{
								best_dist = total_dist;
								best_l = actual_l;
								best_h = actual_h;
							}
						} // hd
					} // ld

					low_endpoint[i] = best_l;
					high_endpoint[i] = best_h;
				}
			}
		}
						
		return surrogate_evaluate_rgba_sp(ps, low_endpoint, high_endpoint, pWeights0, num_weight_levels, enc_params, flags);
	}

	static float cem_surrogate_encode_cem8_12_dp(
		uint32_t cem_index, uint32_t ccs_index,
		const pixel_stats_t& ps, const cem_encode_params& enc_params,
		uint32_t endpoint_ise_range, uint32_t weight_ise_range,
		vec4F& low_endpoint, vec4F& high_endpoint, float* pWeights0, float *pWeights1, uint32_t flags)
	{
		assert((ccs_index >= 0) && (ccs_index <= 3));
		const uint32_t num_endpoint_levels = astc_helpers::get_ise_levels(endpoint_ise_range);

		// astc_helpers::BISE_64_LEVELS=raw weights ([0,64], NOT [0,63])
		const uint32_t num_weight_levels = get_num_weight_levels(weight_ise_range);

		const bool cem_has_alpha = (cem_index == astc_helpers::CEM_LDR_RGBA_DIRECT);
		const uint32_t num_comps = cem_has_alpha ? 4 : 3;

		assert(cem_has_alpha || (ccs_index <= 2));

		vec4F flattened_pixels[ASTC_LDR_MAX_BLOCK_PIXELS];
		for (uint32_t i = 0; i < ps.m_num_pixels; i++)
		{
			flattened_pixels[i] = ps.m_pixels_f[i];
			
			flattened_pixels[i][ccs_index] = 0.0f;
			
			if (!cem_has_alpha)
				flattened_pixels[i][3] = 0.0f;
		}

		vec4F flattened_pixels_mean(ps.m_mean_f);
		flattened_pixels_mean[ccs_index] = 0.0f;
		
		if (!cem_has_alpha)
			flattened_pixels_mean[3] = 0.0f;

		// suppress bogus gcc warning on flattened_pixels
#ifndef __clang__
#if defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmaybe-uninitialized"
#endif
#endif
		const vec4F flattened_axis(calc_pca_4D(ps.m_num_pixels, flattened_pixels, flattened_pixels_mean));

#ifndef __clang__
#if defined(__GNUC__)
#pragma GCC diagnostic pop
#endif
#endif

		float best_dl = BIG_FLOAT_VAL, best_dh = -BIG_FLOAT_VAL;
		int best_l_index = 0, best_h_index = 0;

		for (uint32_t c = 0; c < ps.m_num_pixels; c++)
		{
			const vec4F px(flattened_pixels[c] - flattened_pixels_mean);

			float p = px.dot(flattened_axis);
			if (p < best_dl)
			{
				best_dl = p;
				best_l_index = c;
			}

			if (p > best_dh)
			{
				best_dh = p;
				best_h_index = c;
			}
		} // c
				
		vec4F low_color_f(ps.m_pixels_f[best_l_index]), high_color_f(ps.m_pixels_f[best_h_index]);

		low_color_f[ccs_index] = 0.0f;
		high_color_f[ccs_index] = 0.0f;

		if (!cem_has_alpha)
		{
			low_color_f[3] = 1.0f;
			high_color_f[3] = 1.0f;
		}

		if (low_color_f.dot(vec4F(1.0f)) > high_color_f.dot(vec4F(1.0f)))
			std::swap(low_color_f, high_color_f);
				
		low_color_f[ccs_index] = ps.m_min_f[ccs_index];
		high_color_f[ccs_index] = ps.m_max_f[ccs_index];

		if (!cem_has_alpha)
		{
			low_color_f[3] = 1.0f;
			high_color_f[3] = 1.0f;
		}
						
		low_endpoint = surrogate_quant_endpoint(low_color_f, num_endpoint_levels, flags);
		high_endpoint = surrogate_quant_endpoint(high_color_f, num_endpoint_levels, flags);

		if ((flags & cFlagDisableQuant) == 0)
		{
			for (uint32_t i = 0; i < num_comps; i++)
			{
				if ((low_endpoint[i] == high_endpoint[i]) && (ps.m_min_f[i] != ps.m_max_f[i]))
				{
					const float inv_endpoint_levels = 1.0f / (float)(num_endpoint_levels - 1);

					float best_dist = BIG_FLOAT_VAL;
					float best_l = 0.0f, best_h = 0.0f;

					for (int ld = -2; ld <= 0; ld++)
					{
						float actual_l = saturate(low_endpoint[i] + (float)ld * inv_endpoint_levels);

						for (int hd = 0; hd <= 2; hd++)
						{
							float actual_h = saturate(high_endpoint[i] + (float)hd * inv_endpoint_levels);

							float v0 = lerp(actual_l, actual_h, 1.0f / 3.0f);
							float v1 = lerp(actual_l, actual_h, 2.0f / 3.0f);
							assert(v0 <= v1);

							//if (v0 > v1)
							//	std::swap(v0, v1);

							float dist0 = v0 - ps.m_min_f[0];
							float dist1 = v1 - ps.m_max_f[0];

							float total_dist = dist0 * dist0 + dist1 * dist1;
							if (total_dist < best_dist)
							{
								best_dist = total_dist;
								best_l = actual_l;
								best_h = actual_h;
							}
						} // hd
					} // ld

					low_endpoint[i] = best_l;
					high_endpoint[i] = best_h;
				}
			}
		}

		return surrogate_evaluate_rgba_dp(ccs_index, ps, low_endpoint, high_endpoint, pWeights0, pWeights1, num_weight_levels, enc_params, flags);
	}

	static float cem_surrogate_encode_cem0_4_sp_or_dp(
		uint32_t cem_index, int ccs_index, 
		const pixel_stats_t& ps, const cem_encode_params& enc_params,
		uint32_t endpoint_ise_range, uint32_t weight_ise_range,
		vec4F& low_endpoint, vec4F& high_endpoint, float* pWeights0, float *pWeights1, uint32_t flags)
	{
		const bool cem_has_alpha = (cem_index == astc_helpers::CEM_LDR_LUM_ALPHA_DIRECT);
		const bool dual_plane = (ccs_index == 3);

		if (cem_index == astc_helpers::CEM_LDR_LUM_ALPHA_DIRECT)
		{
			assert((ccs_index == -1) || (ccs_index == 3));
		}
		else
		{
			assert(cem_index == astc_helpers::CEM_LDR_LUM_DIRECT);
			assert(ccs_index == -1);
		}

		const uint32_t num_endpoint_levels = astc_helpers::get_ise_levels(endpoint_ise_range);
		const uint32_t num_weight_levels = get_num_weight_levels(weight_ise_range);

		float lum_l = BIG_FLOAT_VAL, lum_h = -BIG_FLOAT_VAL;

		for (uint32_t i = 0; i < ps.m_num_pixels; i++)
		{
			const vec4F& px = ps.m_pixels_f[i];

			float l = (px[0] + px[1] + px[2]) * (1.0f / 3.0f);

			lum_l = minimum(lum_l, l);
			lum_h = maximum(lum_h, l);
		}

		const float a_l = cem_has_alpha ? ps.m_min_f[3] : 1.0f;
		const float a_h = cem_has_alpha ? ps.m_max_f[3] : 1.0f;

		low_endpoint.set(lum_l, lum_l, lum_l, a_l);
		high_endpoint.set(lum_h, lum_h, lum_h, a_h);

		low_endpoint = surrogate_quant_endpoint(low_endpoint, num_endpoint_levels, flags);
		high_endpoint = surrogate_quant_endpoint(high_endpoint, num_endpoint_levels, flags);

		if (dual_plane)
			return surrogate_evaluate_rgba_dp(ccs_index, ps, low_endpoint, high_endpoint, pWeights0, pWeights1, num_weight_levels, enc_params, flags);
		else
			return surrogate_evaluate_rgba_sp(ps, low_endpoint, high_endpoint, pWeights0, num_weight_levels, enc_params, flags);
	}
	
	float cem_surrogate_encode_pixels(
		uint32_t cem_index, int ccs_index,
		const pixel_stats_t& ps, const cem_encode_params& enc_params,
		uint32_t endpoint_ise_range, uint32_t weight_ise_range,
		vec4F &low_endpoint, vec4F &high_endpoint, float &s, float* pWeights0, float* pWeights1, uint32_t flags)
	{
		assert(g_initialized);
		assert((ccs_index >= -1) && (ccs_index <= 3));
		assert(astc_helpers::is_cem_ldr(cem_index));
		assert(pWeights0 && pWeights1);

		const bool dual_plane = (ccs_index >= 0);

		switch (cem_index)
		{
		case astc_helpers::CEM_LDR_LUM_DIRECT:
		case astc_helpers::CEM_LDR_LUM_ALPHA_DIRECT:
		{
			return cem_surrogate_encode_cem0_4_sp_or_dp(
				cem_index, ccs_index,
				ps, enc_params,
				endpoint_ise_range, weight_ise_range,
				low_endpoint, high_endpoint, pWeights0, pWeights1, flags);
		}
		case astc_helpers::CEM_LDR_RGB_BASE_SCALE:
		case astc_helpers::CEM_LDR_RGB_BASE_SCALE_PLUS_TWO_A:
		{
			if (dual_plane)
			{
				return cem_surrogate_encode_cem6_10_dp(
					cem_index, ccs_index,
					ps, enc_params,
					endpoint_ise_range, weight_ise_range,
					low_endpoint, high_endpoint, s, pWeights0, pWeights1, flags);
			}
			else
			{
				return cem_surrogate_encode_cem6_10_sp(
					cem_index,
					ps, enc_params,
					endpoint_ise_range, weight_ise_range,
					low_endpoint, high_endpoint, s, pWeights0, flags);
			}
			break;
		}
		case astc_helpers::CEM_LDR_RGB_DIRECT:
		case astc_helpers::CEM_LDR_RGBA_DIRECT:
		{
			if (dual_plane)
			{
				return cem_surrogate_encode_cem8_12_dp(
					cem_index, ccs_index,
					ps, enc_params,
					endpoint_ise_range, weight_ise_range,
					low_endpoint, high_endpoint, pWeights0, pWeights1, flags);
			}
			else
			{
				return cem_surrogate_encode_cem8_12_sp(
					cem_index,
					ps, enc_params,
					endpoint_ise_range, weight_ise_range,
					low_endpoint, high_endpoint, pWeights0, flags);
			}
			
			break;
		}
		default:
			assert(0);
			break;
		}

		return BIG_FLOAT_VAL;
	}

	//---------------------------------------------------------------------------------------------

	uint8_t g_part3_mapping[NUM_PART3_MAPPINGS][3] =
	{
		{ 0, 1, 2 },
		{ 1, 2, 0 },
		{ 2, 0, 1 },
		{ 0, 2, 1 },
		{ 1, 0, 2 },
		{ 2, 1, 0 }
	};

	partition_pattern_vec::partition_pattern_vec()
	{
		clear();
	}

	partition_pattern_vec::partition_pattern_vec(const partition_pattern_vec& other)
	{
		*this = other;
	}

	partition_pattern_vec::partition_pattern_vec(uint32_t width, uint32_t height, const uint8_t *pParts) :
		m_width(width), m_height(height)
	{
		if (pParts)
		{
			memcpy(m_parts, pParts, get_total());
		}
	}
		
	void partition_pattern_vec::init(uint32_t width, uint32_t height, const uint8_t* pParts)
	{
		m_width = width;
		m_height = height;
		if (pParts)
		{
			const uint32_t num_texels = get_total();
			memcpy(m_parts, pParts, num_texels);
		}
	}

	void partition_pattern_vec::clear()
	{
		m_width = 0;
		m_height = 0;
		memset(m_parts, 0, sizeof(m_parts));
	}

	partition_pattern_vec& partition_pattern_vec::operator= (const partition_pattern_vec& rhs)
	{
		if (this == &rhs)
			return *this;

		m_width = rhs.m_width;
		m_height = rhs.m_height;
		memcpy(m_parts, rhs.m_parts, get_total());
		
		return *this;
	}
			
	// misnamed- just SAD distance, not square
	int partition_pattern_vec::get_squared_distance(const partition_pattern_vec& other) const
	{
		const uint32_t total_pixels = get_total();

		int total_dist = 0;
		for (uint32_t i = 0; i < total_pixels; i++)
			total_dist += iabs((int)m_parts[i] - (int)other.m_parts[i]);

		return total_dist;
	}

	partition_pattern_vec partition_pattern_vec::get_permuted2(uint32_t permute_index) const
	{
		assert(permute_index <= 1);
		const uint32_t total_pixels = get_total();

		partition_pattern_vec res(m_width, m_height);
		for (uint32_t i = 0; i < total_pixels; i++)
		{
			assert(m_parts[i] <= 1);
			res.m_parts[i] = (uint8_t)(m_parts[i] ^ permute_index);
		}

		return res;
	}
		
	partition_pattern_vec partition_pattern_vec::get_permuted3(uint32_t permute_index) const
	{
		assert(permute_index <= 5);
		const uint32_t total_pixels = get_total();

		partition_pattern_vec res(m_width, m_height);
		for (uint32_t i = 0; i < total_pixels; i++)
		{
			assert(m_parts[i] <= 2);
			res.m_parts[i] = g_part3_mapping[permute_index][m_parts[i]];
		}
				
		return res;
	}

	partition_pattern_vec partition_pattern_vec::get_canonicalized() const
	{
		partition_pattern_vec res(m_width, m_height);
			
		const uint32_t total_pixels = get_total();
						
		int new_labels[4] = { -1, -1, -1, -1 };

		uint32_t next_index = 0;
		for (uint32_t i = 0; i < total_pixels; i++)
		{
			uint32_t p = m_parts[i];
			assert(p <= 3);

			if (new_labels[p] == -1)
				new_labels[p] = next_index++;

			res.m_parts[i] = (uint8_t)new_labels[p];
		}
				
		return res;
	}

	// This requires no redundant patterns, i.e. all must be unique.
	bool vp_tree::init(uint32_t n, const partition_pattern_vec* pUnique_pats)
	{
		clear();

		uint_vec pat_indices(n);
		for (uint32_t i = 0; i < n; i++)
			pat_indices[i] = i;

		std::pair<int, float> root_idx = find_best_vantage_point(n, pUnique_pats, pat_indices);

		if (root_idx.first == -1)
			return false;

		m_nodes.resize(1);
		m_nodes[0].m_vantage_point = pUnique_pats[root_idx.first];
		m_nodes[0].m_point_index = root_idx.first;
		m_nodes[0].m_dist = root_idx.second;
		m_nodes[0].m_inner_node = -1;
		m_nodes[0].m_outer_node = -1;

		uint_vec inner_list, outer_list;

		inner_list.reserve(n / 2);
		outer_list.reserve(n / 2);

		for (uint32_t pat_index = 0; pat_index < n; pat_index++)
		{
			if ((int)pat_index == root_idx.first)
				continue;

			const float dist = m_nodes[0].m_vantage_point.get_distance(pUnique_pats[pat_index]);

			if (dist <= root_idx.second)
				inner_list.push_back(pat_index);
			else
				outer_list.push_back(pat_index);
		}

		if (inner_list.size())
		{
			m_nodes[0].m_inner_node = create_node(n, pUnique_pats, inner_list);
			if (m_nodes[0].m_inner_node < 0)
				return false;
		}

		if (outer_list.size())
		{
			m_nodes[0].m_outer_node = create_node(n, pUnique_pats, outer_list);
			if (m_nodes[0].m_outer_node < 0)
				return false;
		}

		return true;
	}

	void vp_tree::find_nearest(uint32_t num_subsets, const partition_pattern_vec& desired_pat, result_queue& results, uint32_t max_results) const
	{
		assert((num_subsets >= 2) && (num_subsets <= 3));

		results.clear();

		if (!m_nodes.size())
			return;

		uint32_t num_desired_pats;
		partition_pattern_vec desired_pats[NUM_PART3_MAPPINGS];

		if (num_subsets == 2)
		{
			num_desired_pats = 2;
			for (uint32_t i = 0; i < 2; i++)
				desired_pats[i] = desired_pat.get_permuted2(i);
		}
		else
		{
			num_desired_pats = NUM_PART3_MAPPINGS;
			for (uint32_t i = 0; i < NUM_PART3_MAPPINGS; i++)
				desired_pats[i] = desired_pat.get_permuted3(i);
		}

#if 0
		find_nearest_at_node(0, num_desired_pats, desired_pats, results, max_results);
#else
		find_nearest_at_node_non_recursive(0, num_desired_pats, desired_pats, results, max_results);
#endif
	}

	void vp_tree::find_nearest_at_node(int node_index, uint32_t num_desired_pats, const partition_pattern_vec* pDesired_pats, result_queue& results, uint32_t max_results) const
	{
		float best_dist_to_vantage = BIG_FLOAT_VAL;
		uint32_t best_mapping = 0;
		for (uint32_t i = 0; i < num_desired_pats; i++)
		{
			float dist = pDesired_pats[i].get_distance(m_nodes[node_index].m_vantage_point);
			if (dist < best_dist_to_vantage)
			{
				best_dist_to_vantage = dist;
				best_mapping = i;
			}
		}

		result r;
		r.m_dist = best_dist_to_vantage;
		r.m_mapping_index = best_mapping;
		r.m_pat_index = m_nodes[node_index].m_point_index;

		results.insert(r, max_results);

		if (best_dist_to_vantage <= m_nodes[node_index].m_dist)
		{
			// inner first
			if (m_nodes[node_index].m_inner_node >= 0)
				find_nearest_at_node(m_nodes[node_index].m_inner_node, num_desired_pats, pDesired_pats, results, max_results);

			if (m_nodes[node_index].m_outer_node >= 0)
			{
				if ((results.get_size() < max_results) ||
					((m_nodes[node_index].m_dist - best_dist_to_vantage) <= results.get_highest_dist())
					)
				{
					find_nearest_at_node(m_nodes[node_index].m_outer_node, num_desired_pats, pDesired_pats, results, max_results);
				}
			}
		}
		else
		{
			// outer first
			if (m_nodes[node_index].m_outer_node >= 0)
				find_nearest_at_node(m_nodes[node_index].m_outer_node, num_desired_pats, pDesired_pats, results, max_results);

			if (m_nodes[node_index].m_inner_node >= 0)
			{
				if ((results.get_size() < max_results) ||
					((best_dist_to_vantage - m_nodes[node_index].m_dist) <= results.get_highest_dist())
					)
				{
					find_nearest_at_node(m_nodes[node_index].m_inner_node, num_desired_pats, pDesired_pats, results, max_results);
				}
			}
		}
	}

	void vp_tree::find_nearest_at_node_non_recursive(int init_node_index, uint32_t num_desired_pats, const partition_pattern_vec* pDesired_pats, result_queue& results, uint32_t max_results) const
	{
		uint_vec node_stack;
		node_stack.reserve(16);
		node_stack.push_back(init_node_index);

		do
		{
			const uint32_t node_index = node_stack.back();
			node_stack.pop_back();

			float best_dist_to_vantage = BIG_FLOAT_VAL;
			uint32_t best_mapping = 0;
			for (uint32_t i = 0; i < num_desired_pats; i++)
			{
				float dist = pDesired_pats[i].get_distance(m_nodes[node_index].m_vantage_point);
				if (dist < best_dist_to_vantage)
				{
					best_dist_to_vantage = dist;
					best_mapping = i;
				}
			}

			result r;
			r.m_dist = best_dist_to_vantage;
			r.m_mapping_index = best_mapping;
			r.m_pat_index = m_nodes[node_index].m_point_index;

			results.insert(r, max_results);

			if (best_dist_to_vantage <= m_nodes[node_index].m_dist)
			{
				if (m_nodes[node_index].m_outer_node >= 0)
				{
					if ((results.get_size() < max_results) ||
						((m_nodes[node_index].m_dist - best_dist_to_vantage) <= results.get_highest_dist())
						)
					{
						node_stack.push_back(m_nodes[node_index].m_outer_node);
					}
				}

				// inner first
				if (m_nodes[node_index].m_inner_node >= 0)
				{
					node_stack.push_back(m_nodes[node_index].m_inner_node);
				}
			}
			else
			{
				if (m_nodes[node_index].m_inner_node >= 0)
				{
					if ((results.get_size() < max_results) ||
						((best_dist_to_vantage - m_nodes[node_index].m_dist) <= results.get_highest_dist())
						)
					{
						node_stack.push_back(m_nodes[node_index].m_inner_node);
					}
				}

				// outer first
				if (m_nodes[node_index].m_outer_node >= 0)
				{
					node_stack.push_back(m_nodes[node_index].m_outer_node);
				}
			}

		} while (!node_stack.empty());
	}

	// returns the index of the new node, or -1 on error
	int vp_tree::create_node(uint32_t n, const partition_pattern_vec* pUnique_pats, const uint_vec& pat_indices)
	{
		std::pair<int, float> root_idx = find_best_vantage_point(n, pUnique_pats, pat_indices);

		if (root_idx.first < 0)
			return -1;

		m_nodes.resize(m_nodes.size() + 1);
		const uint32_t new_node_index = m_nodes.size_u32() - 1;

		m_nodes[new_node_index].m_vantage_point = pUnique_pats[root_idx.first];
		m_nodes[new_node_index].m_point_index = root_idx.first;
		m_nodes[new_node_index].m_dist = root_idx.second;
		m_nodes[new_node_index].m_inner_node = -1;
		m_nodes[new_node_index].m_outer_node = -1;

		uint_vec inner_list, outer_list;

		inner_list.reserve(pat_indices.size_u32() / 2);
		outer_list.reserve(pat_indices.size_u32() / 2);

		for (uint32_t pat_indices_iter = 0; pat_indices_iter < pat_indices.size(); pat_indices_iter++)
		{
			const uint32_t pat_index = pat_indices[pat_indices_iter];

			if ((int)pat_index == root_idx.first)
				continue;

			const float dist = m_nodes[new_node_index].m_vantage_point.get_distance(pUnique_pats[pat_index]);

			if (dist <= root_idx.second)
				inner_list.push_back(pat_index);
			else
				outer_list.push_back(pat_index);
		}

		if (inner_list.size())
			m_nodes[new_node_index].m_inner_node = create_node(n, pUnique_pats, inner_list);

		if (outer_list.size())
			m_nodes[new_node_index].m_outer_node = create_node(n, pUnique_pats, outer_list);

		return new_node_index;
	}

	// returns the pattern index of the vantage point (-1 on error), and the optimal split distance
	std::pair<int, float> vp_tree::find_best_vantage_point(uint32_t num_unique_pats, const partition_pattern_vec* pUnique_pats, const uint_vec& pat_indices)
	{
		BASISU_NOTE_UNUSED(num_unique_pats);

		const uint32_t n = pat_indices.size_u32();

		assert(n);
		if (n == 1)
			return std::pair(pat_indices[0], 0.0f);

		float best_split_metric = -1.0f;
		int best_split_pat = -1;
		float best_split_dist = 0.0f;
		float best_split_var = 0.0f;

		basisu::vector< std::pair<float, uint32_t> > dists;
		dists.reserve(n);

		float_vec float_dists;
		float_dists.reserve(n);

		for (uint32_t pat_indices_iter = 0; pat_indices_iter < n; pat_indices_iter++)
		{
			const uint32_t split_pat_index = pat_indices[pat_indices_iter];
			assert(split_pat_index < num_unique_pats);

			const partition_pattern_vec& trial_vantage = pUnique_pats[split_pat_index];

			dists.resize(0);
			float_dists.resize(0);

			for (uint32_t j = 0; j < n; j++)
			{
				const uint32_t pat_index = pat_indices[j];
				assert(pat_index < num_unique_pats);

				if (pat_index == split_pat_index)
					continue;

				float dist = trial_vantage.get_distance(pUnique_pats[pat_index]);
				dists.emplace_back(std::pair(dist, pat_index));

				float_dists.push_back(dist);
			}

			stats<double> s;
			s.calc(float_dists.size_u32(), float_dists.data());

			std::sort(dists.begin(), dists.end(), [](const auto& a, const auto& b) {
				return a.first < b.first;
				});

			const uint32_t num_dists = dists.size_u32();
			float split_dist = dists[num_dists / 2].first;
			if ((num_dists & 1) == 0)
				split_dist = (split_dist + dists[(num_dists / 2) - 1].first) * .5f;

			uint32_t total_inner = 0, total_outer = 0;

			for (uint32_t j = 0; j < n; j++)
			{
				const uint32_t pat_index = pat_indices[j];
				if (pat_index == split_pat_index)
					continue;

				float dist = trial_vantage.get_distance(pUnique_pats[pat_index]);

				if (dist <= split_dist)
					total_inner++;
				else
					total_outer++;
			}

			float split_metric = (float)minimum(total_inner, total_outer) / (float)maximum(total_inner, total_outer);

			if ((split_metric > best_split_metric) ||
				((split_metric == best_split_metric) && (s.m_var > best_split_var)))
			{
				best_split_metric = split_metric;
				best_split_dist = split_dist;
				best_split_pat = split_pat_index;
				best_split_var = (float)s.m_var;
			}
		}

		return std::pair(best_split_pat, best_split_dist);
	}

	void partitions_data::init(uint32_t num_partitions, uint32_t block_width, uint32_t block_height, bool init_vp_tree)
	{
		assert((num_partitions >= 2) && (num_partitions <= 4));

		//const uint32_t total_texels = block_width * block_height;

		m_width = block_width;
		m_height = block_height;
		m_num_partitions = num_partitions;
						
		m_part_vp_tree.clear();

		for (uint32_t i = 0; i < 1024; i++)
		{
			m_part_seed_to_unique_index[i] = -1;
			m_unique_index_to_part_seed[i] = -1;
		}

		//const bool is_small_block = astc_helpers::is_small_block(block_width, block_height);
	
		partition_hash_map part_hash;
		part_hash.reserve(1024);
		m_total_unique_patterns = 0;

		clear_obj(m_partition_pat_histograms);

		for (uint32_t seed_index = 0; seed_index < astc_helpers::NUM_PARTITION_PATTERNS; seed_index++)
		{
			partition_pattern_vec pat;
			uint32_t part_hist[4] = { 0 };

			pat.init(block_width, block_height);

			for (uint32_t y = 0; y < block_height; y++)
			{
				for (uint32_t x = 0; x < block_width; x++)
				{
					//const uint8_t p = (uint8_t)astc_helpers::compute_texel_partition(seed_index, x, y, 0, m_num_partitions, is_small_block);
					const uint8_t p = (uint8_t)astc_helpers::get_precomputed_texel_partition(block_width, block_height, seed_index, x, y, num_partitions);
					
					assert((p < m_num_partitions) && (p < 4));
										
					pat(x, y) = p;

					part_hist[p]++;
				} // x
			} // y
						
			bool skip_pat = false;
			for (uint32_t i = 0; i < m_num_partitions; i++)
			{
				if (!part_hist[i])
				{
					skip_pat = true;
					break;
				}
			}
			if (skip_pat)
				continue;

			partition_pattern_vec std_pat(pat.get_canonicalized());

			if (part_hash.contains(std_pat))
				continue;

			if (num_partitions == 2)
			{
				assert(!part_hash.contains(pat));
				assert(!part_hash.contains(pat.get_permuted2(1)));
			}
			else if (num_partitions == 3)
			{
				for (uint32_t i = 0; i < partition_pattern_vec::cMaxPermute3Index; i++)
				{
					assert(!part_hash.contains(pat.get_permuted3(i)));
				}
			}

			for (uint32_t c = 0; c < 4; c++)
				m_partition_pat_histograms[m_total_unique_patterns].m_hist[c] = (uint8_t)part_hist[c];

			part_hash.insert(std_pat, std::make_pair(seed_index, m_total_unique_patterns));

			m_part_seed_to_unique_index[seed_index] = (int16_t)m_total_unique_patterns;
			m_unique_index_to_part_seed[m_total_unique_patterns] = (int16_t)seed_index;
															
			m_partition_pats[m_total_unique_patterns] = pat;
						
			m_total_unique_patterns++;

		} // seed_index

		if (init_vp_tree)
			m_part_vp_tree.init(m_total_unique_patterns, m_partition_pats);
	}
		
} //  namespace astc_ldr

} // namespace basisu
