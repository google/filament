// basisu_gpu_texture.cpp
// Copyright (C) 2019-2026 Binomial LLC. All Rights Reserved.
//
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
#include "basisu_gpu_texture.h"
#include "basisu_enc.h"
#include "basisu_pvrtc1_4.h"
#include "3rdparty/android_astc_decomp.h"
#include "basisu_bc7enc.h"
#include "../transcoder/basisu_astc_hdr_core.h"

#define TINYDDS_IMPLEMENTATION
#include "3rdparty/tinydds.h"

#define BASISU_USE_GOOGLE_ASTC_DECODER (1)

namespace basisu
{
	//------------------------------------------------------------------------------------------------
	// ETC2 EAC

	void unpack_etc2_eac(const void* pBlock_bits, color_rgba* pPixels)
	{
		static_assert(sizeof(eac_a8_block) == 8, "sizeof(eac_a8_block) == 8");

		const eac_a8_block* pBlock = static_cast<const eac_a8_block*>(pBlock_bits);

		const int8_t* pTable = g_etc2_eac_tables[pBlock->m_table];

		const uint64_t selector_bits = pBlock->get_selector_bits();

		const int32_t base = pBlock->m_base;
		const int32_t mul = pBlock->m_multiplier;

		pPixels[0].a = clamp255(base + pTable[pBlock->get_selector(0, 0, selector_bits)] * mul);
		pPixels[1].a = clamp255(base + pTable[pBlock->get_selector(1, 0, selector_bits)] * mul);
		pPixels[2].a = clamp255(base + pTable[pBlock->get_selector(2, 0, selector_bits)] * mul);
		pPixels[3].a = clamp255(base + pTable[pBlock->get_selector(3, 0, selector_bits)] * mul);

		pPixels[4].a = clamp255(base + pTable[pBlock->get_selector(0, 1, selector_bits)] * mul);
		pPixels[5].a = clamp255(base + pTable[pBlock->get_selector(1, 1, selector_bits)] * mul);
		pPixels[6].a = clamp255(base + pTable[pBlock->get_selector(2, 1, selector_bits)] * mul);
		pPixels[7].a = clamp255(base + pTable[pBlock->get_selector(3, 1, selector_bits)] * mul);

		pPixels[8].a = clamp255(base + pTable[pBlock->get_selector(0, 2, selector_bits)] * mul);
		pPixels[9].a = clamp255(base + pTable[pBlock->get_selector(1, 2, selector_bits)] * mul);
		pPixels[10].a = clamp255(base + pTable[pBlock->get_selector(2, 2, selector_bits)] * mul);
		pPixels[11].a = clamp255(base + pTable[pBlock->get_selector(3, 2, selector_bits)] * mul);

		pPixels[12].a = clamp255(base + pTable[pBlock->get_selector(0, 3, selector_bits)] * mul);
		pPixels[13].a = clamp255(base + pTable[pBlock->get_selector(1, 3, selector_bits)] * mul);
		pPixels[14].a = clamp255(base + pTable[pBlock->get_selector(2, 3, selector_bits)] * mul);
		pPixels[15].a = clamp255(base + pTable[pBlock->get_selector(3, 3, selector_bits)] * mul);
	}

	//------------------------------------------------------------------------------------------------
	// BC1
	struct bc1_block
	{
		enum { cTotalEndpointBytes = 2, cTotalSelectorBytes = 4 };

		uint8_t m_low_color[cTotalEndpointBytes];
		uint8_t m_high_color[cTotalEndpointBytes];
		uint8_t m_selectors[cTotalSelectorBytes];

		inline uint32_t get_high_color() const { return m_high_color[0] | (m_high_color[1] << 8U); }
		inline uint32_t get_low_color() const { return m_low_color[0] | (m_low_color[1] << 8U); }

		static void unpack_color(uint32_t c, uint32_t& r, uint32_t& g, uint32_t& b)
		{
			r = (c >> 11) & 31;
			g = (c >> 5) & 63;
			b = c & 31;

			r = (r << 3) | (r >> 2);
			g = (g << 2) | (g >> 4);
			b = (b << 3) | (b >> 2);
		}

		inline uint32_t get_selector(uint32_t x, uint32_t y) const { assert((x < 4U) && (y < 4U)); return (m_selectors[y] >> (x * 2)) & 3; }
	};

	// Returns true if the block uses 3 color punchthrough alpha mode.
	bool unpack_bc1(const void* pBlock_bits, color_rgba* pPixels, bool set_alpha)
	{
		static_assert(sizeof(bc1_block) == 8, "sizeof(bc1_block) == 8");

		const bc1_block* pBlock = static_cast<const bc1_block*>(pBlock_bits);

		const uint32_t l = pBlock->get_low_color();
		const uint32_t h = pBlock->get_high_color();

		color_rgba c[4];

		uint32_t r0, g0, b0, r1, g1, b1;
		bc1_block::unpack_color(l, r0, g0, b0);
		bc1_block::unpack_color(h, r1, g1, b1);

		c[0].set_noclamp_rgba(r0, g0, b0, 255);
		c[1].set_noclamp_rgba(r1, g1, b1, 255);

		bool used_punchthrough = false;

		if (l > h)
		{
			c[2].set_noclamp_rgba((r0 * 2 + r1) / 3, (g0 * 2 + g1) / 3, (b0 * 2 + b1) / 3, 255);
			c[3].set_noclamp_rgba((r1 * 2 + r0) / 3, (g1 * 2 + g0) / 3, (b1 * 2 + b0) / 3, 255);
		}
		else
		{
			c[2].set_noclamp_rgba((r0 + r1) / 2, (g0 + g1) / 2, (b0 + b1) / 2, 255);
			c[3].set_noclamp_rgba(0, 0, 0, 0);
			used_punchthrough = true;
		}

		if (set_alpha)
		{
			for (uint32_t y = 0; y < 4; y++, pPixels += 4)
			{
				pPixels[0] = c[pBlock->get_selector(0, y)];
				pPixels[1] = c[pBlock->get_selector(1, y)];
				pPixels[2] = c[pBlock->get_selector(2, y)];
				pPixels[3] = c[pBlock->get_selector(3, y)];
			}
		}
		else
		{
			for (uint32_t y = 0; y < 4; y++, pPixels += 4)
			{
				pPixels[0].set_rgb(c[pBlock->get_selector(0, y)]);
				pPixels[1].set_rgb(c[pBlock->get_selector(1, y)]);
				pPixels[2].set_rgb(c[pBlock->get_selector(2, y)]);
				pPixels[3].set_rgb(c[pBlock->get_selector(3, y)]);
			}
		}

		return used_punchthrough;
	}

	bool unpack_bc1_nv(const void* pBlock_bits, color_rgba* pPixels, bool set_alpha)
	{
		static_assert(sizeof(bc1_block) == 8, "sizeof(bc1_block) == 8");

		const bc1_block* pBlock = static_cast<const bc1_block*>(pBlock_bits);

		const uint32_t l = pBlock->get_low_color();
		const uint32_t h = pBlock->get_high_color();

		color_rgba c[4];

		int r0 = (l >> 11) & 31;
		int g0 = (l >> 5) & 63;
		int b0 = l & 31;
		int r1 = (h >> 11) & 31;
		int g1 = (h >> 5) & 63;
		int b1 = h & 31;

		c[0].b = (uint8_t)((3 * b0 * 22) / 8);
		c[0].g = (uint8_t)((g0 << 2) | (g0 >> 4));
		c[0].r = (uint8_t)((3 * r0 * 22) / 8);
		c[0].a = 0xFF;

		c[1].r = (uint8_t)((3 * r1 * 22) / 8);
		c[1].g = (uint8_t)((g1 << 2) | (g1 >> 4));
		c[1].b = (uint8_t)((3 * b1 * 22) / 8);
		c[1].a = 0xFF;

		int gdiff = c[1].g - c[0].g;

		bool used_punchthrough = false;

		if (l > h)
		{
			c[2].r = (uint8_t)(((2 * r0 + r1) * 22) / 8);
			c[2].g = (uint8_t)(((256 * c[0].g + gdiff / 4 + 128 + gdiff * 80) / 256));
			c[2].b = (uint8_t)(((2 * b0 + b1) * 22) / 8);
			c[2].a = 0xFF;

			c[3].r = (uint8_t)(((2 * r1 + r0) * 22) / 8);
			c[3].g = (uint8_t)((256 * c[1].g - gdiff / 4 + 128 - gdiff * 80) / 256);
			c[3].b = (uint8_t)(((2 * b1 + b0) * 22) / 8);
			c[3].a = 0xFF;
		}
		else
		{
			c[2].r = (uint8_t)(((r0 + r1) * 33) / 8);
			c[2].g = (uint8_t)((256 * c[0].g + gdiff / 4 + 128 + gdiff * 128) / 256);
			c[2].b = (uint8_t)(((b0 + b1) * 33) / 8);
			c[2].a = 0xFF;

			c[3].set_noclamp_rgba(0, 0, 0, 0);
			used_punchthrough = true;
		}

		if (set_alpha)
		{
			for (uint32_t y = 0; y < 4; y++, pPixels += 4)
			{
				pPixels[0] = c[pBlock->get_selector(0, y)];
				pPixels[1] = c[pBlock->get_selector(1, y)];
				pPixels[2] = c[pBlock->get_selector(2, y)];
				pPixels[3] = c[pBlock->get_selector(3, y)];
			}
		}
		else
		{
			for (uint32_t y = 0; y < 4; y++, pPixels += 4)
			{
				pPixels[0].set_rgb(c[pBlock->get_selector(0, y)]);
				pPixels[1].set_rgb(c[pBlock->get_selector(1, y)]);
				pPixels[2].set_rgb(c[pBlock->get_selector(2, y)]);
				pPixels[3].set_rgb(c[pBlock->get_selector(3, y)]);
			}
		}

		return used_punchthrough;
	}

	static inline int interp_5_6_amd(int c0, int c1) { assert(c0 < 256 && c1 < 256); return (c0 * 43 + c1 * 21 + 32) >> 6; }
	static inline int interp_half_5_6_amd(int c0, int c1) { assert(c0 < 256 && c1 < 256); return (c0 + c1 + 1) >> 1; }

	bool unpack_bc1_amd(const void* pBlock_bits, color_rgba* pPixels, bool set_alpha)
	{
		const bc1_block* pBlock = static_cast<const bc1_block*>(pBlock_bits);

		const uint32_t l = pBlock->get_low_color();
		const uint32_t h = pBlock->get_high_color();

		color_rgba c[4];

		uint32_t r0, g0, b0, r1, g1, b1;
		bc1_block::unpack_color(l, r0, g0, b0);
		bc1_block::unpack_color(h, r1, g1, b1);

		c[0].set_noclamp_rgba(r0, g0, b0, 255);
		c[1].set_noclamp_rgba(r1, g1, b1, 255);

		bool used_punchthrough = false;

		if (l > h)
		{
			c[2].set_noclamp_rgba(interp_5_6_amd(r0, r1), interp_5_6_amd(g0, g1), interp_5_6_amd(b0, b1), 255);
			c[3].set_noclamp_rgba(interp_5_6_amd(r1, r0), interp_5_6_amd(g1, g0), interp_5_6_amd(b1, b0), 255);
		}
		else
		{
			c[2].set_noclamp_rgba(interp_half_5_6_amd(r0, r1), interp_half_5_6_amd(g0, g1), interp_half_5_6_amd(b0, b1), 255);
			c[3].set_noclamp_rgba(0, 0, 0, 0);
			used_punchthrough = true;
		}

		if (set_alpha)
		{
			for (uint32_t y = 0; y < 4; y++, pPixels += 4)
			{
				pPixels[0] = c[pBlock->get_selector(0, y)];
				pPixels[1] = c[pBlock->get_selector(1, y)];
				pPixels[2] = c[pBlock->get_selector(2, y)];
				pPixels[3] = c[pBlock->get_selector(3, y)];
			}
		}
		else
		{
			for (uint32_t y = 0; y < 4; y++, pPixels += 4)
			{
				pPixels[0].set_rgb(c[pBlock->get_selector(0, y)]);
				pPixels[1].set_rgb(c[pBlock->get_selector(1, y)]);
				pPixels[2].set_rgb(c[pBlock->get_selector(2, y)]);
				pPixels[3].set_rgb(c[pBlock->get_selector(3, y)]);
			}
		}

		return used_punchthrough;
	}

	//------------------------------------------------------------------------------------------------
	// BC3-5

	struct bc4_block
	{
		enum { cBC4SelectorBits = 3, cTotalSelectorBytes = 6, cMaxSelectorValues = 8 };
		uint8_t m_endpoints[2];

		uint8_t m_selectors[cTotalSelectorBytes];

		inline uint32_t get_low_alpha() const { return m_endpoints[0]; }
		inline uint32_t get_high_alpha() const { return m_endpoints[1]; }
		inline bool is_alpha6_block() const { return get_low_alpha() <= get_high_alpha(); }

		inline uint64_t get_selector_bits() const
		{
			return ((uint64_t)((uint32_t)m_selectors[0] | ((uint32_t)m_selectors[1] << 8U) | ((uint32_t)m_selectors[2] << 16U) | ((uint32_t)m_selectors[3] << 24U))) |
				(((uint64_t)m_selectors[4]) << 32U) |
				(((uint64_t)m_selectors[5]) << 40U);
		}

		inline uint32_t get_selector(uint32_t x, uint32_t y, uint64_t selector_bits) const
		{
			assert((x < 4U) && (y < 4U));
			return (selector_bits >> (((y * 4) + x) * cBC4SelectorBits)) & (cMaxSelectorValues - 1);
		}

		static inline uint32_t get_block_values6(uint8_t* pDst, uint32_t l, uint32_t h)
		{
			pDst[0] = static_cast<uint8_t>(l);
			pDst[1] = static_cast<uint8_t>(h);
			pDst[2] = static_cast<uint8_t>((l * 4 + h) / 5);
			pDst[3] = static_cast<uint8_t>((l * 3 + h * 2) / 5);
			pDst[4] = static_cast<uint8_t>((l * 2 + h * 3) / 5);
			pDst[5] = static_cast<uint8_t>((l + h * 4) / 5);
			pDst[6] = 0;
			pDst[7] = 255;
			return 6;
		}

		static inline uint32_t get_block_values8(uint8_t* pDst, uint32_t l, uint32_t h)
		{
			pDst[0] = static_cast<uint8_t>(l);
			pDst[1] = static_cast<uint8_t>(h);
			pDst[2] = static_cast<uint8_t>((l * 6 + h) / 7);
			pDst[3] = static_cast<uint8_t>((l * 5 + h * 2) / 7);
			pDst[4] = static_cast<uint8_t>((l * 4 + h * 3) / 7);
			pDst[5] = static_cast<uint8_t>((l * 3 + h * 4) / 7);
			pDst[6] = static_cast<uint8_t>((l * 2 + h * 5) / 7);
			pDst[7] = static_cast<uint8_t>((l + h * 6) / 7);
			return 8;
		}

		static inline uint32_t get_block_values(uint8_t* pDst, uint32_t l, uint32_t h)
		{
			if (l > h)
				return get_block_values8(pDst, l, h);
			else
				return get_block_values6(pDst, l, h);
		}
	};

	void unpack_bc4(const void* pBlock_bits, uint8_t* pPixels, uint32_t stride)
	{
		static_assert(sizeof(bc4_block) == 8, "sizeof(bc4_block) == 8");

		const bc4_block* pBlock = static_cast<const bc4_block*>(pBlock_bits);

		uint8_t sel_values[8];
		bc4_block::get_block_values(sel_values, pBlock->get_low_alpha(), pBlock->get_high_alpha());

		const uint64_t selector_bits = pBlock->get_selector_bits();

		for (uint32_t y = 0; y < 4; y++, pPixels += (stride * 4U))
		{
			pPixels[0] = sel_values[pBlock->get_selector(0, y, selector_bits)];
			pPixels[stride * 1] = sel_values[pBlock->get_selector(1, y, selector_bits)];
			pPixels[stride * 2] = sel_values[pBlock->get_selector(2, y, selector_bits)];
			pPixels[stride * 3] = sel_values[pBlock->get_selector(3, y, selector_bits)];
		}
	}

	// Returns false if the block uses 3-color punchthrough alpha mode, which isn't supported on some GPU's for BC3.
	bool unpack_bc3(const void* pBlock_bits, color_rgba* pPixels)
	{
		bool success = true;

		if (unpack_bc1((const uint8_t*)pBlock_bits + sizeof(bc4_block), pPixels, true))
			success = false;

		unpack_bc4(pBlock_bits, &pPixels[0].a, sizeof(color_rgba));

		return success;
	}

	// writes RG
	void unpack_bc5(const void* pBlock_bits, color_rgba* pPixels)
	{
		unpack_bc4(pBlock_bits, &pPixels[0].r, sizeof(color_rgba));
		unpack_bc4((const uint8_t*)pBlock_bits + sizeof(bc4_block), &pPixels[0].g, sizeof(color_rgba));
	}

	//------------------------------------------------------------------------------------------------
	// ATC isn't officially documented, so I'm assuming these references:
	// http://www.guildsoftware.com/papers/2012.Converting.DXTC.to.ATC.pdf
	// https://github.com/Triang3l/S3TConv/blob/master/s3tconv_atitc.c
	// The paper incorrectly says the ATC lerp factors are 1/3 and 2/3, but they are actually 3/8 and 5/8.
	void unpack_atc(const void* pBlock_bits, color_rgba* pPixels)
	{
		const uint8_t* pBytes = static_cast<const uint8_t*>(pBlock_bits);

		const uint16_t color0 = pBytes[0] | (pBytes[1] << 8U);
		const uint16_t color1 = pBytes[2] | (pBytes[3] << 8U);
		uint32_t sels = pBytes[4] | (pBytes[5] << 8U) | (pBytes[6] << 16U) | (pBytes[7] << 24U);

		const bool mode = (color0 & 0x8000) != 0;

		color_rgba c[4];

		c[0].set((color0 >> 10) & 31, (color0 >> 5) & 31, color0 & 31, 255);
		c[0].r = (c[0].r << 3) | (c[0].r >> 2);
		c[0].g = (c[0].g << 3) | (c[0].g >> 2);
		c[0].b = (c[0].b << 3) | (c[0].b >> 2);

		c[3].set((color1 >> 11) & 31, (color1 >> 5) & 63, color1 & 31, 255);
		c[3].r = (c[3].r << 3) | (c[3].r >> 2);
		c[3].g = (c[3].g << 2) | (c[3].g >> 4);
		c[3].b = (c[3].b << 3) | (c[3].b >> 2);

		if (mode)
		{
			c[1].set(basisu::maximum(0, c[0].r - (c[3].r >> 2)), basisu::maximum(0, c[0].g - (c[3].g >> 2)), basisu::maximum(0, c[0].b - (c[3].b >> 2)), 255);
			c[2] = c[0];
			c[0].set(0, 0, 0, 255);
		}
		else
		{
			c[1].r = (c[0].r * 5 + c[3].r * 3) >> 3;
			c[1].g = (c[0].g * 5 + c[3].g * 3) >> 3;
			c[1].b = (c[0].b * 5 + c[3].b * 3) >> 3;

			c[2].r = (c[0].r * 3 + c[3].r * 5) >> 3;
			c[2].g = (c[0].g * 3 + c[3].g * 5) >> 3;
			c[2].b = (c[0].b * 3 + c[3].b * 5) >> 3;
		}

		for (uint32_t i = 0; i < 16; i++)
		{
			const uint32_t s = sels & 3;

			pPixels[i] = c[s];

			sels >>= 2;
		}
	}

	static inline int bc6h_sign_extend(int val, int bits)
	{
		assert((bits >= 1) && (bits < 32));
		assert((val >= 0) && (val < (1 << bits)));
		return (val << (32 - bits)) >> (32 - bits);
	}

	static inline int bc6h_apply_delta(int base, int delta, int num_bits, int is_signed)
	{
		int bitmask = ((1 << num_bits) - 1);
		int v = (base + delta) & bitmask;
		return is_signed ? bc6h_sign_extend(v, num_bits) : v;
	}

	static int bc6h_dequantize(int val, int bits, int is_signed)
	{
		int result;
		if (is_signed)
		{
			if (bits >= 16)
				result = val;
			else
			{
				int s_flag = 0;
				if (val < 0)
				{
					s_flag = 1;
					val = -val;
				}

				if (val == 0)
					result = 0;
				else if (val >= ((1 << (bits - 1)) - 1))
					result = 0x7FFF;
				else
					result = ((val << 15) + 0x4000) >> (bits - 1);

				if (s_flag)
					result = -result;
			}
		}
		else
		{
			if (bits >= 15)
				result = val;
			else if (!val)
				result = 0;
			else if (val == ((1 << bits) - 1))
				result = 0xFFFF;
			else
				result = ((val << 16) + 0x8000) >> bits;
		}
		return result;
	}

	static inline int bc6h_interpolate(int a, int b, const uint8_t* pWeights, int index)
	{
		return (a * (64 - (int)pWeights[index]) + b * (int)pWeights[index] + 32) >> 6;
	}

	static inline basist::half_float bc6h_convert_to_half(int val, int is_signed)
	{
		if (!is_signed)
		{
			// scale by 31/64
			return (basist::half_float)((val * 31) >> 6);
		}

		// scale by 31/32
		val = (val < 0) ? -(((-val) * 31) >> 5) : (val * 31) >> 5;

		int s = 0;
		if (val < 0)
		{
			s = 0x8000;
			val = -val;
		}

		return (basist::half_float)(s | val);
	}

	static inline uint32_t bc6h_get_bits(uint32_t num_bits, uint64_t& l, uint64_t& h, uint32_t& total_bits)
	{
		assert((num_bits) && (num_bits <= 63));

		uint32_t v = (uint32_t)(l & ((1U << num_bits) - 1U));

		l >>= num_bits;
		l |= (h << (64U - num_bits));
		h >>= num_bits;

		total_bits += num_bits;
		assert(total_bits <= 128);

		return v;
	}

	static inline uint32_t bc6h_reverse_bits(uint32_t v, uint32_t num_bits)
	{
		uint32_t res = 0;
		for (uint32_t i = 0; i < num_bits; i++)
		{
			uint32_t bit = (v & (1u << i)) != 0u;
			res |= (bit << (num_bits - 1u - i));
		}
		return res;
	}

	static inline uint64_t bc6h_read_le_qword(const void* p)
	{
		const uint8_t* pSrc = static_cast<const uint8_t*>(p);
		return ((uint64_t)read_le_dword(pSrc)) | (((uint64_t)read_le_dword(pSrc + sizeof(uint32_t))) << 32U);
	}

	bool unpack_bc6h(const void* pSrc_block, void* pDst_block, bool is_signed, uint32_t dest_pitch_in_halfs)
	{
		assert(dest_pitch_in_halfs >= 4 * 3);

		const uint32_t MAX_SUBSETS = 2, MAX_COMPS = 3;

		const uint8_t* pSrc = static_cast<const uint8_t*>(pSrc_block);
		basist::half_float* pDst = static_cast<basist::half_float*>(pDst_block);

		uint64_t blo = bc6h_read_le_qword(pSrc), bhi = bc6h_read_le_qword(pSrc + sizeof(uint64_t));

		// Unpack mode
		const int mode = basist::g_bc6h_mode_lookup[blo & 31];
		if (mode < 0)
		{
			for (int y = 0; y < 4; y++)
			{
				memset(pDst, 0, sizeof(basist::half_float) * 4);
				pDst += dest_pitch_in_halfs;
			}
			return false;
		}

		// Skip mode bits
		uint32_t total_bits_read = 0;
		bc6h_get_bits((mode < 2) ? 2 : 5, blo, bhi, total_bits_read);

		assert(mode < (int)basist::NUM_BC6H_MODES);

		const uint32_t num_subsets = (mode >= 10) ? 1 : 2;
		const bool is_mode_9_or_10 = (mode == 9) || (mode == 10);

		// Unpack endpoint components
		int comps[MAX_SUBSETS][MAX_COMPS][2] = { { { 0 } } };		// [subset][comp][l/h]
		int part_index = 0;

		uint32_t layout_index = 0;
		while (layout_index < basist::MAX_BC6H_LAYOUT_INDEX)
		{
			const basist::bc6h_bit_layout& layout = basist::g_bc6h_bit_layouts[mode][layout_index];

			if (layout.m_comp < 0)
				break;

			const int subset = layout.m_index >> 1, lh_index = layout.m_index & 1;
			assert((layout.m_comp == 3) || ((subset >= 0) && (subset < (int)MAX_SUBSETS)));

			const int last_bit = layout.m_last_bit, first_bit = layout.m_first_bit;
			assert(last_bit >= 0);

			int& res = (layout.m_comp == 3) ? part_index : comps[subset][layout.m_comp][lh_index];

			if (first_bit < 0)
			{
				res |= (bc6h_get_bits(1, blo, bhi, total_bits_read) << last_bit);
			}
			else
			{
				const int total_bits = iabs(last_bit - first_bit) + 1;
				const int bit_shift = basisu::minimum(first_bit, last_bit);

				int b = bc6h_get_bits(total_bits, blo, bhi, total_bits_read);

				if (last_bit < first_bit)
					b = bc6h_reverse_bits(b, total_bits);

				res |= (b << bit_shift);
			}

			layout_index++;
		}
		assert(layout_index != basist::MAX_BC6H_LAYOUT_INDEX);

		// Sign extend/dequantize endpoints
		const int num_sig_bits = basist::g_bc6h_mode_sig_bits[mode][0];
		if (is_signed)
		{
			for (uint32_t comp = 0; comp < 3; comp++)
				comps[0][comp][0] = bc6h_sign_extend(comps[0][comp][0], num_sig_bits);
		}

		if (is_signed || !is_mode_9_or_10)
		{
			for (uint32_t subset = 0; subset < num_subsets; subset++)
				for (uint32_t comp = 0; comp < 3; comp++)
					for (uint32_t lh = (subset ? 0 : 1); lh < 2; lh++)
						comps[subset][comp][lh] = bc6h_sign_extend(comps[subset][comp][lh], basist::g_bc6h_mode_sig_bits[mode][1 + comp]);
		}

		if (!is_mode_9_or_10)
		{
			for (uint32_t subset = 0; subset < num_subsets; subset++)
				for (uint32_t comp = 0; comp < 3; comp++)
					for (uint32_t lh = (subset ? 0 : 1); lh < 2; lh++)
						comps[subset][comp][lh] = bc6h_apply_delta(comps[0][comp][0], comps[subset][comp][lh], num_sig_bits, is_signed);
		}

		for (uint32_t subset = 0; subset < num_subsets; subset++)
			for (uint32_t comp = 0; comp < 3; comp++)
				for (uint32_t lh = 0; lh < 2; lh++)
					comps[subset][comp][lh] = bc6h_dequantize(comps[subset][comp][lh], num_sig_bits, is_signed);

		// Now unpack weights and output texels
		const int weight_bits = (mode >= 10) ? 4 : 3;
		const uint8_t* pWeights = (mode >= 10) ? basist::g_bc6h_weight4 : basist::g_bc6h_weight3;

		dest_pitch_in_halfs -= 4 * 3;

		for (uint32_t y = 0; y < 4; y++)
		{
			for (uint32_t x = 0; x < 4; x++)
			{
				int subset = (num_subsets == 1) ? ((x | y) ? 0 : 0x80) : basist::g_bc6h_2subset_patterns[part_index][y][x];
				const int num_bits = weight_bits + ((subset & 0x80) ? -1 : 0);

				subset &= 1;

				const int weight_index = bc6h_get_bits(num_bits, blo, bhi, total_bits_read);

				pDst[0] = bc6h_convert_to_half(bc6h_interpolate(comps[subset][0][0], comps[subset][0][1], pWeights, weight_index), is_signed);
				pDst[1] = bc6h_convert_to_half(bc6h_interpolate(comps[subset][1][0], comps[subset][1][1], pWeights, weight_index), is_signed);
				pDst[2] = bc6h_convert_to_half(bc6h_interpolate(comps[subset][2][0], comps[subset][2][1], pWeights, weight_index), is_signed);

				pDst += 3;
			}

			pDst += dest_pitch_in_halfs;
		}

		assert(total_bits_read == 128);
		return true;
	}
	//------------------------------------------------------------------------------------------------
	// FXT1 (for fun, and because some modern Intel parts support it, and because a subset is like BC1)

	struct fxt1_block
	{
		union
		{
			struct
			{
				uint64_t m_t00 : 2;
				uint64_t m_t01 : 2;
				uint64_t m_t02 : 2;
				uint64_t m_t03 : 2;
				uint64_t m_t04 : 2;
				uint64_t m_t05 : 2;
				uint64_t m_t06 : 2;
				uint64_t m_t07 : 2;
				uint64_t m_t08 : 2;
				uint64_t m_t09 : 2;
				uint64_t m_t10 : 2;
				uint64_t m_t11 : 2;
				uint64_t m_t12 : 2;
				uint64_t m_t13 : 2;
				uint64_t m_t14 : 2;
				uint64_t m_t15 : 2;
				uint64_t m_t16 : 2;
				uint64_t m_t17 : 2;
				uint64_t m_t18 : 2;
				uint64_t m_t19 : 2;
				uint64_t m_t20 : 2;
				uint64_t m_t21 : 2;
				uint64_t m_t22 : 2;
				uint64_t m_t23 : 2;
				uint64_t m_t24 : 2;
				uint64_t m_t25 : 2;
				uint64_t m_t26 : 2;
				uint64_t m_t27 : 2;
				uint64_t m_t28 : 2;
				uint64_t m_t29 : 2;
				uint64_t m_t30 : 2;
				uint64_t m_t31 : 2;
			} m_lo;
			uint64_t m_lo_bits;
			uint8_t m_sels[8];
		};

		union
		{
			struct
			{
#ifdef BASISU_USE_ORIGINAL_3DFX_FXT1_ENCODING
				// This is the format that 3DFX's DECOMP.EXE tool expects, which I'm assuming is what the actual 3DFX hardware wanted.
				// Unfortunately, color0/color1 and color2/color3 are flipped relative to the official OpenGL extension and Intel's documentation!
				uint64_t m_b1 : 5;
				uint64_t m_g1 : 5;
				uint64_t m_r1 : 5;
				uint64_t m_b0 : 5;
				uint64_t m_g0 : 5;
				uint64_t m_r0 : 5;
				uint64_t m_b3 : 5;
				uint64_t m_g3 : 5;
				uint64_t m_r3 : 5;
				uint64_t m_b2 : 5;
				uint64_t m_g2 : 5;
				uint64_t m_r2 : 5;
#else
				// Intel's encoding, and the encoding in the OpenGL FXT1 spec.
				uint64_t m_b0 : 5;
				uint64_t m_g0 : 5;
				uint64_t m_r0 : 5;
				uint64_t m_b1 : 5;
				uint64_t m_g1 : 5;
				uint64_t m_r1 : 5;
				uint64_t m_b2 : 5;
				uint64_t m_g2 : 5;
				uint64_t m_r2 : 5;
				uint64_t m_b3 : 5;
				uint64_t m_g3 : 5;
				uint64_t m_r3 : 5;
#endif
				uint64_t m_alpha : 1;
				uint64_t m_glsb : 2;
				uint64_t m_mode : 1;
			} m_hi;

			uint64_t m_hi_bits;
		};
	};

	static color_rgba expand_565(const color_rgba& c)
	{
		return color_rgba((c.r << 3) | (c.r >> 2), (c.g << 2) | (c.g >> 4), (c.b << 3) | (c.b >> 2), 255);
	}

	// We only support CC_MIXED non-alpha blocks here because that's the only mode the transcoder uses at the moment.
	bool unpack_fxt1(const void *p, color_rgba *pPixels)
	{
		const fxt1_block* pBlock = static_cast<const fxt1_block*>(p);

		if (pBlock->m_hi.m_mode == 0)
			return false;
		if (pBlock->m_hi.m_alpha == 1)
			return false;
				
		color_rgba colors[4];

		colors[0].r = pBlock->m_hi.m_r0;
		colors[0].g = (uint8_t)((pBlock->m_hi.m_g0 << 1) | ((pBlock->m_lo.m_t00 >> 1) ^ (pBlock->m_hi.m_glsb & 1)));
		colors[0].b = pBlock->m_hi.m_b0;
		colors[0].a = 255;

		colors[1].r = pBlock->m_hi.m_r1;
		colors[1].g = (uint8_t)((pBlock->m_hi.m_g1 << 1) | (pBlock->m_hi.m_glsb & 1));
		colors[1].b = pBlock->m_hi.m_b1;
		colors[1].a = 255;

		colors[2].r = pBlock->m_hi.m_r2;
		colors[2].g = (uint8_t)((pBlock->m_hi.m_g2 << 1) | ((pBlock->m_lo.m_t16 >> 1) ^ (pBlock->m_hi.m_glsb >> 1)));
		colors[2].b = pBlock->m_hi.m_b2;
		colors[2].a = 255;

		colors[3].r = pBlock->m_hi.m_r3;
		colors[3].g = (uint8_t)((pBlock->m_hi.m_g3 << 1) | (pBlock->m_hi.m_glsb >> 1));
		colors[3].b = pBlock->m_hi.m_b3;
		colors[3].a = 255;

		for (uint32_t i = 0; i < 4; i++)
			colors[i] = expand_565(colors[i]);

		color_rgba block0_colors[4];
		block0_colors[0] = colors[0];
		block0_colors[1] = color_rgba((colors[0].r * 2 + colors[1].r + 1) / 3, (colors[0].g * 2 + colors[1].g + 1) / 3, (colors[0].b * 2 + colors[1].b + 1) / 3, 255);
		block0_colors[2] = color_rgba((colors[1].r * 2 + colors[0].r + 1) / 3, (colors[1].g * 2 + colors[0].g + 1) / 3, (colors[1].b * 2 + colors[0].b + 1) / 3, 255);
		block0_colors[3] = colors[1];

		for (uint32_t i = 0; i < 16; i++)
		{
			const uint32_t sel = (pBlock->m_sels[i >> 2] >> ((i & 3) * 2)) & 3;

			const uint32_t x = i & 3;
			const uint32_t y = i >> 2;
			pPixels[x + y * 8] = block0_colors[sel];
		}

		color_rgba block1_colors[4];
		block1_colors[0] = colors[2];
		block1_colors[1] = color_rgba((colors[2].r * 2 + colors[3].r + 1) / 3, (colors[2].g * 2 + colors[3].g + 1) / 3, (colors[2].b * 2 + colors[3].b + 1) / 3, 255);
		block1_colors[2] = color_rgba((colors[3].r * 2 + colors[2].r + 1) / 3, (colors[3].g * 2 + colors[2].g + 1) / 3, (colors[3].b * 2 + colors[2].b + 1) / 3, 255);
		block1_colors[3] = colors[3];

		for (uint32_t i = 0; i < 16; i++)
		{
			const uint32_t sel = (pBlock->m_sels[4 + (i >> 2)] >> ((i & 3) * 2)) & 3;
			
			const uint32_t x = i & 3;
			const uint32_t y = i >> 2;
			pPixels[4 + x + y * 8] = block1_colors[sel];
		}

		return true;
	}

	//------------------------------------------------------------------------------------------------
	// PVRTC2 (non-interpolated, hard_flag=1 modulation=0 subset only!)

	struct pvrtc2_block
	{
		uint8_t m_modulation[4];

		union
		{
			union
			{
				// Opaque mode: RGB colora=554 and colorb=555
				struct
				{
					uint32_t m_mod_flag : 1;
					uint32_t m_blue_a : 4;
					uint32_t m_green_a : 5;
					uint32_t m_red_a : 5;
					uint32_t m_hard_flag : 1;
					uint32_t m_blue_b : 5;
					uint32_t m_green_b : 5;
					uint32_t m_red_b : 5;
					uint32_t m_opaque_flag : 1;

				} m_opaque_color_data;

				// Transparent mode: RGBA colora=4433 and colorb=4443
				struct
				{
					uint32_t m_mod_flag : 1;
					uint32_t m_blue_a : 3;
					uint32_t m_green_a : 4;
					uint32_t m_red_a : 4;
					uint32_t m_alpha_a : 3;
					uint32_t m_hard_flag : 1;
					uint32_t m_blue_b : 4;
					uint32_t m_green_b : 4;
					uint32_t m_red_b : 4;
					uint32_t m_alpha_b : 3;
					uint32_t m_opaque_flag : 1;

				} m_trans_color_data;
			};

			uint32_t m_color_data_bits;
		};
	};

	static color_rgba convert_rgb_555_to_888(const color_rgba& col)
	{
		return color_rgba((col[0] << 3) | (col[0] >> 2), (col[1] << 3) | (col[1] >> 2), (col[2] << 3) | (col[2] >> 2), 255);
	}
	
	static color_rgba convert_rgba_5554_to_8888(const color_rgba& col)
	{
		return color_rgba((col[0] << 3) | (col[0] >> 2), (col[1] << 3) | (col[1] >> 2), (col[2] << 3) | (col[2] >> 2), (col[3] << 4) | col[3]);
	}

	// PVRTC2 is currently limited to only what our transcoder outputs (non-interpolated, hard_flag=1 modulation=0). In this mode, PVRTC2 looks much like BC1/ATC.
	bool unpack_pvrtc2(const void *p, color_rgba *pPixels)
	{
		const pvrtc2_block* pBlock = static_cast<const pvrtc2_block*>(p);

		if ((!pBlock->m_opaque_color_data.m_hard_flag) || (pBlock->m_opaque_color_data.m_mod_flag))
		{
			// This mode isn't supported by the transcoder, so we aren't bothering with it here.
			return false;
		}

		color_rgba colors[4];

		if (pBlock->m_opaque_color_data.m_opaque_flag)
		{
			// colora=554
			color_rgba color_a(pBlock->m_opaque_color_data.m_red_a, pBlock->m_opaque_color_data.m_green_a, (pBlock->m_opaque_color_data.m_blue_a << 1) | (pBlock->m_opaque_color_data.m_blue_a >> 3), 255);
			
			// colora=555
			color_rgba color_b(pBlock->m_opaque_color_data.m_red_b, pBlock->m_opaque_color_data.m_green_b, pBlock->m_opaque_color_data.m_blue_b, 255);
						
			colors[0] = convert_rgb_555_to_888(color_a);
			colors[3] = convert_rgb_555_to_888(color_b);

			colors[1].set((colors[0].r * 5 + colors[3].r * 3) / 8, (colors[0].g * 5 + colors[3].g * 3) / 8, (colors[0].b * 5 + colors[3].b * 3) / 8, 255);
			colors[2].set((colors[0].r * 3 + colors[3].r * 5) / 8, (colors[0].g * 3 + colors[3].g * 5) / 8, (colors[0].b * 3 + colors[3].b * 5) / 8, 255);
		}
		else
		{
			// colora=4433 
			color_rgba color_a(
				(pBlock->m_trans_color_data.m_red_a << 1) | (pBlock->m_trans_color_data.m_red_a >> 3), 
				(pBlock->m_trans_color_data.m_green_a << 1) | (pBlock->m_trans_color_data.m_green_a >> 3),
				(pBlock->m_trans_color_data.m_blue_a << 2) | (pBlock->m_trans_color_data.m_blue_a >> 1), 
				pBlock->m_trans_color_data.m_alpha_a << 1);

			//colorb=4443
			color_rgba color_b(
				(pBlock->m_trans_color_data.m_red_b << 1) | (pBlock->m_trans_color_data.m_red_b >> 3),
				(pBlock->m_trans_color_data.m_green_b << 1) | (pBlock->m_trans_color_data.m_green_b >> 3),
				(pBlock->m_trans_color_data.m_blue_b << 1) | (pBlock->m_trans_color_data.m_blue_b >> 3),
				(pBlock->m_trans_color_data.m_alpha_b << 1) | 1);

			colors[0] = convert_rgba_5554_to_8888(color_a);
			colors[3] = convert_rgba_5554_to_8888(color_b);
		}

		colors[1].set((colors[0].r * 5 + colors[3].r * 3) / 8, (colors[0].g * 5 + colors[3].g * 3) / 8, (colors[0].b * 5 + colors[3].b * 3) / 8, (colors[0].a * 5 + colors[3].a * 3) / 8);
		colors[2].set((colors[0].r * 3 + colors[3].r * 5) / 8, (colors[0].g * 3 + colors[3].g * 5) / 8, (colors[0].b * 3 + colors[3].b * 5) / 8, (colors[0].a * 3 + colors[3].a * 5) / 8);

		for (uint32_t i = 0; i < 16; i++)
		{
			const uint32_t sel = (pBlock->m_modulation[i >> 2] >> ((i & 3) * 2)) & 3;
			pPixels[i] = colors[sel];
		}

		return true;
	}

	//------------------------------------------------------------------------------------------------
	// ETC2 EAC R11 or RG11

	struct etc2_eac_r11
	{
		uint64_t m_base	: 8;
		uint64_t m_table	: 4;
		uint64_t m_mul		: 4;
		uint64_t m_sels_0 : 8;
		uint64_t m_sels_1 : 8;
		uint64_t m_sels_2 : 8;
		uint64_t m_sels_3 : 8;
		uint64_t m_sels_4 : 8;
		uint64_t m_sels_5 : 8;

		uint64_t get_sels() const
		{
			return ((uint64_t)m_sels_0 << 40U) | ((uint64_t)m_sels_1 << 32U) | ((uint64_t)m_sels_2 << 24U) | ((uint64_t)m_sels_3 << 16U) | ((uint64_t)m_sels_4 << 8U) | m_sels_5;
		}

		void set_sels(uint64_t v)
		{
			m_sels_0 = (v >> 40U) & 0xFF;
			m_sels_1 = (v >> 32U) & 0xFF;
			m_sels_2 = (v >> 24U) & 0xFF;
			m_sels_3 = (v >> 16U) & 0xFF;
			m_sels_4 = (v >> 8U) & 0xFF;
			m_sels_5 = v & 0xFF;
		}
	};

	struct etc2_eac_rg11
	{
		etc2_eac_r11 m_c[2];
	};

	void unpack_etc2_eac_r(const void *p, color_rgba* pPixels, uint32_t c)
	{
		const etc2_eac_r11* pBlock = static_cast<const etc2_eac_r11*>(p);
		const uint64_t sels = pBlock->get_sels();

		const int base = (int)pBlock->m_base * 8 + 4;
		const int mul = pBlock->m_mul ? ((int)pBlock->m_mul * 8) : 1;
		const int table = (int)pBlock->m_table;

		for (uint32_t y = 0; y < 4; y++)
		{
			for (uint32_t x = 0; x < 4; x++)
			{
				const uint32_t shift = 45 - ((y + x * 4) * 3);
				
				const uint32_t sel = (uint32_t)((sels >> shift) & 7);
				
				int val = base + g_etc2_eac_tables[table][sel] * mul;
				val = clamp<int>(val, 0, 2047);

				// Convert to 8-bits with rounding
				//pPixels[x + y * 4].m_comps[c] = static_cast<uint8_t>((val * 255 + 1024) / 2047);
				pPixels[x + y * 4].m_comps[c] = static_cast<uint8_t>((val * 255 + 1023) / 2047);

			} // x
		} // y
	}

	void unpack_etc2_eac_rg(const void* p, color_rgba* pPixels)
	{
		for (uint32_t c = 0; c < 2; c++)
		{
			const etc2_eac_r11* pBlock = &static_cast<const etc2_eac_rg11*>(p)->m_c[c];

			unpack_etc2_eac_r(pBlock, pPixels, c);
		}
	}

	//------------------------------------------------------------------------------------------------
	// UASTC

	void unpack_uastc(const void* p, color_rgba* pPixels)
	{
		basist::unpack_uastc(*static_cast<const basist::uastc_block*>(p), (basist::color32 *)pPixels, false);
	}
			
	// Unpacks to RGBA, R, RG, or A. LDR GPU texture formats only.
	// astc_srgb: if true, ASTC LDR formats are decoded in sRGB decode mode, otherwise L8.
	bool unpack_block(texture_format fmt, const void* pBlock, color_rgba* pPixels, bool astc_srgb)
	{
		switch (fmt)
		{
		case texture_format::cBC1:
		{
			unpack_bc1(pBlock, pPixels, true);
			break;
		}
		case texture_format::cBC1_NV:
		{
			unpack_bc1_nv(pBlock, pPixels, true);
			break;
		}
		case texture_format::cBC1_AMD:
		{
			unpack_bc1_amd(pBlock, pPixels, true);
			break;
		}
		case texture_format::cBC3:
		{
			return unpack_bc3(pBlock, pPixels);
		}
		case texture_format::cBC4:
		{
			// Unpack to R
			unpack_bc4(pBlock, &pPixels[0].r, sizeof(color_rgba));
			break;
		}
		case texture_format::cBC5:
		{
			unpack_bc5(pBlock, pPixels);
			break;
		}
		case texture_format::cBC7:
		{
			return basist::bc7u::unpack_bc7(pBlock, reinterpret_cast<basist::color_rgba *>(pPixels));
		}
		// Full ETC2 color blocks (planar/T/H modes) is currently unsupported in basisu, but we do support ETC2 with alpha (using ETC1 for color)
		case texture_format::cETC2_RGB:
		case texture_format::cETC1:
		case texture_format::cETC1S:
		{
			return unpack_etc1(*static_cast<const etc_block*>(pBlock), pPixels);
		}
		case texture_format::cETC2_RGBA:
		{
			if (!unpack_etc1(static_cast<const etc_block*>(pBlock)[1], pPixels))
				return false;
			unpack_etc2_eac(pBlock, pPixels);
			break;
		}
		case texture_format::cETC2_ALPHA:
		{
			// Unpack to A
			unpack_etc2_eac(pBlock, pPixels);
			break;
		}
		case texture_format::cBC6HSigned:
		case texture_format::cBC6HUnsigned:
		case texture_format::cASTC_HDR_4x4:
		case texture_format::cUASTC_HDR_4x4:
		case texture_format::cASTC_HDR_6x6:
		{
			// Can't unpack HDR blocks in unpack_block() because it returns 32bpp pixel data.
			assert(0);
			return false;
		}
		case texture_format::cASTC_LDR_4x4:
		case texture_format::cASTC_LDR_5x4:
		case texture_format::cASTC_LDR_5x5:
		case texture_format::cASTC_LDR_6x5:
		case texture_format::cASTC_LDR_6x6:
		case texture_format::cASTC_LDR_8x5:
		case texture_format::cASTC_LDR_8x6:
		case texture_format::cASTC_LDR_10x5:
		case texture_format::cASTC_LDR_10x6:
		case texture_format::cASTC_LDR_8x8:
		case texture_format::cASTC_LDR_10x8:
		case texture_format::cASTC_LDR_10x10:
		case texture_format::cASTC_LDR_12x10:
		case texture_format::cASTC_LDR_12x12:
		{
			const uint32_t block_width = get_block_width(fmt), block_height = get_block_height(fmt);

			assert(get_astc_ldr_texture_format(block_width, block_height) == fmt);
			assert(astc_helpers::is_valid_block_size(block_width, block_height));
						
			// TODO: Allow caller to use the Android decoder, too.
			bool status = basisu_astc::astc::decompress_ldr(reinterpret_cast<uint8_t*>(pPixels), static_cast<const uint8_t*>(pBlock), astc_srgb, block_width, block_height);
			assert(status);

			if (!status)
				return false;
			
			break;
		}
		case texture_format::cATC_RGB:
		{
			unpack_atc(pBlock, pPixels);
			break;
		}
		case texture_format::cATC_RGBA_INTERPOLATED_ALPHA:
		{
			unpack_atc(static_cast<const uint8_t*>(pBlock) + 8, pPixels);
			unpack_bc4(pBlock, &pPixels[0].a, sizeof(color_rgba));
			break;
		}
		case texture_format::cFXT1_RGB:
		{
			unpack_fxt1(pBlock, pPixels);
			break;
		}
		case texture_format::cPVRTC2_4_RGBA:
		{
			unpack_pvrtc2(pBlock, pPixels);
			break;
		}
		case texture_format::cETC2_R11_EAC:
		{
			unpack_etc2_eac_r(static_cast<const etc2_eac_r11 *>(pBlock), pPixels, 0);
			break;
		}
		case texture_format::cETC2_RG11_EAC:
		{
			unpack_etc2_eac_rg(pBlock, pPixels);
			break;
		}
		case texture_format::cUASTC4x4:
		{
			unpack_uastc(pBlock, pPixels);
			break;
		}
		default:
		{
			assert(0);
			// TODO
			return false;
		}
		}
		return true;
	}

	bool unpack_block_hdr(texture_format fmt, const void* pBlock, vec4F* pPixels)
	{
		switch (fmt)
		{
			case texture_format::cASTC_HDR_6x6:
			{
#if BASISU_USE_GOOGLE_ASTC_DECODER
				bool status = basisu_astc::astc::decompress_hdr(&pPixels[0][0], (uint8_t*)pBlock, 6, 6);
				assert(status);
				if (!status)
					return false;
#else
				// Use our decoder
				basist::half_float half_block[6 * 6][4];

				astc_helpers::log_astc_block log_blk;
				if (!astc_helpers::unpack_block(pBlock, log_blk, 6, 6))
					return false;
				if (!astc_helpers::decode_block(log_blk, half_block, 6, 6, astc_helpers::cDecodeModeHDR16))
					return false;

				for (uint32_t p = 0; p < (6 * 6); p++)
				{
					pPixels[p][0] = basist::half_to_float(half_block[p][0]);
					pPixels[p][1] = basist::half_to_float(half_block[p][1]);
					pPixels[p][2] = basist::half_to_float(half_block[p][2]);
					pPixels[p][3] = basist::half_to_float(half_block[p][3]);
				}
#endif
				return true;
			}
			case texture_format::cASTC_HDR_4x4:
			case texture_format::cUASTC_HDR_4x4:
			{
#if BASISU_USE_GOOGLE_ASTC_DECODER
				// Use Google's decoder
				bool status = basisu_astc::astc::decompress_hdr(&pPixels[0][0], (uint8_t*)pBlock, 4, 4);
				assert(status);
				if (!status)
					return false;
#else
				// Use our decoder
				basist::half_float half_block[16][4];
				
				astc_helpers::log_astc_block log_blk;
				if (!astc_helpers::unpack_block(pBlock, log_blk, 4, 4))
					return false;
				if (!astc_helpers::decode_block(log_blk, half_block, 4, 4, astc_helpers::cDecodeModeHDR16))
					return false;

				for (uint32_t p = 0; p < 16; p++)
				{
					pPixels[p][0] = basist::half_to_float(half_block[p][0]);
					pPixels[p][1] = basist::half_to_float(half_block[p][1]);
					pPixels[p][2] = basist::half_to_float(half_block[p][2]);
					pPixels[p][3] = basist::half_to_float(half_block[p][3]);
				}

				//memset(pPixels, 0, sizeof(vec4F) * 16);
#endif
				return true;
			}
			case texture_format::cBC6HSigned:
			case texture_format::cBC6HUnsigned:
			{
				basist::half_float half_block[16][3];

				unpack_bc6h(pBlock, half_block, fmt == texture_format::cBC6HSigned);

				for (uint32_t p = 0; p < 16; p++)
				{
					pPixels[p][0] = basist::half_to_float(half_block[p][0]);
					pPixels[p][1] = basist::half_to_float(half_block[p][1]);
					pPixels[p][2] = basist::half_to_float(half_block[p][2]);
					pPixels[p][3] = 1.0f;
				}

				return true;
			}
			default:
			{
				break;
			}
		}

		assert(0);
		return false;
	}
		
	bool gpu_image::unpack(image& img, bool astc_srgb) const
	{
		img.resize(get_pixel_width(), get_pixel_height());
		img.set_all(g_black_color);

		if (!img.get_width() || !img.get_height())
			return true;

		if ((m_fmt == texture_format::cPVRTC1_4_RGB) || (m_fmt == texture_format::cPVRTC1_4_RGBA))
		{
			pvrtc4_image pi(m_width, m_height);
			
			if (get_total_blocks() != pi.get_total_blocks())
				return false;
			
			memcpy((void *)&pi.get_blocks()[0], (const void *)get_ptr(), get_size_in_bytes());

			pi.deswizzle();

			pi.unpack_all_pixels(img);

			return true;
		}

		assert((m_block_width <= cMaxBlockSize) && (m_block_height <= cMaxBlockSize));
		color_rgba pixels[cMaxBlockSize * cMaxBlockSize];
		for (uint32_t i = 0; i < cMaxBlockSize * cMaxBlockSize; i++)
			pixels[i] = g_black_color;

		bool success = true;

		for (uint32_t by = 0; by < m_blocks_y; by++)
		{
			for (uint32_t bx = 0; bx < m_blocks_x; bx++)
			{
				const void* pBlock = get_block_ptr(bx, by);

				if (!unpack_block(m_fmt, pBlock, pixels, astc_srgb))
					success = false;

				img.set_block_clipped(pixels, bx * m_block_width, by * m_block_height, m_block_width, m_block_height);
			} // bx
		} // by

		return success;
	}

	bool gpu_image::unpack_hdr(imagef& img) const
	{
		if ((m_fmt != texture_format::cASTC_HDR_4x4) && (m_fmt != texture_format::cUASTC_HDR_4x4) && (m_fmt != texture_format::cASTC_HDR_6x6) &&
			(m_fmt != texture_format::cBC6HUnsigned) &&	(m_fmt != texture_format::cBC6HSigned))
		{
			// Can't call on LDR images, at least currently. (Could unpack the LDR data and convert to float.)
			assert(0);
			return false;
		}

		img.resize(get_pixel_width(), get_pixel_height());
		img.set_all(vec4F(0.0f));

		if (!img.get_width() || !img.get_height())
			return true;

		assert((m_block_width <= cMaxBlockSize) && (m_block_height <= cMaxBlockSize));
		vec4F pixels[cMaxBlockSize * cMaxBlockSize];
		clear_obj(pixels);

		bool success = true;

		for (uint32_t by = 0; by < m_blocks_y; by++)
		{
			for (uint32_t bx = 0; bx < m_blocks_x; bx++)
			{
				const void* pBlock = get_block_ptr(bx, by);

				if (!unpack_block_hdr(m_fmt, pBlock, pixels))
					success = false;

				img.set_block_clipped(pixels, bx * m_block_width, by * m_block_height, m_block_width, m_block_height);
			} // bx
		} // by

		return success;
	}
		
	// KTX1 texture file writing
	static const uint8_t g_ktx_file_id[12] = { 0xAB, 0x4B, 0x54, 0x58, 0x20, 0x31, 0x31, 0xBB, 0x0D, 0x0A, 0x1A, 0x0A };

	// KTX/GL enums
	enum
	{
		KTX_ENDIAN = 0x04030201, 
		KTX_OPPOSITE_ENDIAN = 0x01020304,
		KTX_ETC1_RGB8_OES = 0x8D64,
		KTX_RED = 0x1903,
		KTX_RG = 0x8227,
		KTX_RGB = 0x1907,
		KTX_RGBA = 0x1908,

		KTX_COMPRESSED_RGB_S3TC_DXT1_EXT = 0x83F0,
		KTX_COMPRESSED_RGBA_S3TC_DXT5_EXT = 0x83F3,
		KTX_COMPRESSED_RED_RGTC1_EXT = 0x8DBB,
		KTX_COMPRESSED_RED_GREEN_RGTC2_EXT = 0x8DBD,
		KTX_COMPRESSED_RGB8_ETC2 = 0x9274,
		KTX_COMPRESSED_RGBA8_ETC2_EAC = 0x9278,
		KTX_COMPRESSED_RGBA_BPTC_UNORM = 0x8E8C,
		KTX_COMPRESSED_SRGB_ALPHA_BPTC_UNORM = 0x8E8D,
		KTX_COMPRESSED_RGB_BPTC_SIGNED_FLOAT = 0x8E8E,
		KTX_COMPRESSED_RGB_BPTC_UNSIGNED_FLOAT = 0x8E8F,
		KTX_COMPRESSED_RGB_PVRTC_4BPPV1_IMG = 0x8C00,
		KTX_COMPRESSED_RGBA_PVRTC_4BPPV1_IMG = 0x8C02,
		
		KTX_COMPRESSED_RGBA_ASTC_4x4_KHR = 0x93B0,
		KTX_COMPRESSED_RGBA_ASTC_5x4_KHR = 0x93B1,
		KTX_COMPRESSED_RGBA_ASTC_5x5_KHR = 0x93B2,
		KTX_COMPRESSED_RGBA_ASTC_6x5_KHR = 0x93B3,
		KTX_COMPRESSED_RGBA_ASTC_6x6_KHR = 0x93B4,
		KTX_COMPRESSED_RGBA_ASTC_8x5_KHR = 0x93B5,
		KTX_COMPRESSED_RGBA_ASTC_8x6_KHR = 0x93B6,
		KTX_COMPRESSED_RGBA_ASTC_8x8_KHR = 0x93B7,
		KTX_COMPRESSED_RGBA_ASTC_10x5_KHR = 0x93B8,
		KTX_COMPRESSED_RGBA_ASTC_10x6_KHR = 0x93B9,
		KTX_COMPRESSED_RGBA_ASTC_10x8_KHR = 0x93BA,
		KTX_COMPRESSED_RGBA_ASTC_10x10_KHR = 0x93BB,
		KTX_COMPRESSED_RGBA_ASTC_12x10_KHR = 0x93BC,
		KTX_COMPRESSED_RGBA_ASTC_12x12_KHR = 0x93BD,

		KTX_COMPRESSED_SRGB8_ALPHA8_ASTC_4x4_KHR = 0x93D0,
		KTX_COMPRESSED_SRGB8_ALPHA8_ASTC_5x4_KHR = 0x93D1,
		KTX_COMPRESSED_SRGB8_ALPHA8_ASTC_5x5_KHR = 0x93D2,
		KTX_COMPRESSED_SRGB8_ALPHA8_ASTC_6x5_KHR = 0x93D3,
		KTX_COMPRESSED_SRGB8_ALPHA8_ASTC_6x6_KHR = 0x93D4,
		KTX_COMPRESSED_SRGB8_ALPHA8_ASTC_8x5_KHR = 0x93D5,
		KTX_COMPRESSED_SRGB8_ALPHA8_ASTC_8x6_KHR = 0x93D6,
		KTX_COMPRESSED_SRGB8_ALPHA8_ASTC_8x8_KHR = 0x93D7,
		KTX_COMPRESSED_SRGB8_ALPHA8_ASTC_10x5_KHR = 0x93D8,
		KTX_COMPRESSED_SRGB8_ALPHA8_ASTC_10x6_KHR = 0x93D9,
		KTX_COMPRESSED_SRGB8_ALPHA8_ASTC_10x8_KHR = 0x93DA,
		KTX_COMPRESSED_SRGB8_ALPHA8_ASTC_10x10_KHR = 0x93DB,
		KTX_COMPRESSED_SRGB8_ALPHA8_ASTC_12x10_KHR = 0x93DC,
		KTX_COMPRESSED_SRGB8_ALPHA8_ASTC_12x12_KHR = 0x93DD,

		KTX_COMPRESSED_RGBA_UASTC_4x4_KHR = 0x94CC, // TODO - Use proper value!

		KTX_ATC_RGB_AMD = 0x8C92,
		KTX_ATC_RGBA_INTERPOLATED_ALPHA_AMD = 0x87EE,

		KTX_COMPRESSED_RGB_FXT1_3DFX = 0x86B0,
		KTX_COMPRESSED_RGBA_FXT1_3DFX = 0x86B1,
		KTX_COMPRESSED_RGBA_PVRTC_4BPPV2_IMG = 0x9138,
		KTX_COMPRESSED_R11_EAC = 0x9270,
		KTX_COMPRESSED_RG11_EAC = 0x9272
	};
		
	struct ktx_header
	{
		uint8_t m_identifier[12];
		packed_uint<4> m_endianness;
		packed_uint<4> m_glType;
		packed_uint<4> m_glTypeSize;
		packed_uint<4> m_glFormat;
		packed_uint<4> m_glInternalFormat;
		packed_uint<4> m_glBaseInternalFormat;
		packed_uint<4> m_pixelWidth;
		packed_uint<4> m_pixelHeight;
		packed_uint<4> m_pixelDepth;
		packed_uint<4> m_numberOfArrayElements;
		packed_uint<4> m_numberOfFaces;
		packed_uint<4> m_numberOfMipmapLevels;
		packed_uint<4> m_bytesOfKeyValueData;

		void clear() { clear_obj(*this);	}
	};

	// Input is a texture array of mipmapped gpu_image's: gpu_images[array_index][level_index]
	bool create_ktx_texture_file(uint8_vec &ktx_data, const basisu::vector<gpu_image_vec>& gpu_images, bool cubemap_flag, bool astc_srgb_flag)
	{
		if (!gpu_images.size())
		{
			assert(0);
			return false;
		}

		uint32_t width = 0, height = 0, total_levels = 0;
		basisu::texture_format fmt = texture_format::cInvalidTextureFormat;

		// Sanity check the input
		if (cubemap_flag)
		{
			if ((gpu_images.size() % 6) != 0)
			{
				assert(0);
				return false;
			}
		}
				
		for (uint32_t array_index = 0; array_index < gpu_images.size(); array_index++)
		{
			const gpu_image_vec &levels = gpu_images[array_index];

			if (!levels.size())
			{
				// Empty mip chain
				assert(0);
				return false;
			}

			if (!array_index)
			{
				width = levels[0].get_pixel_width();
				height = levels[0].get_pixel_height();
				total_levels = (uint32_t)levels.size();
				fmt = levels[0].get_format();
			}
			else
			{
				if ((width != levels[0].get_pixel_width()) ||
				    (height != levels[0].get_pixel_height()) ||
				    (total_levels != levels.size()))
				{
					// All cubemap/texture array faces must be the same dimension
					assert(0);
					return false;
				}
			}

			for (uint32_t level_index = 0; level_index < levels.size(); level_index++)
			{
				if (level_index)
				{
					if ( (levels[level_index].get_pixel_width() != maximum<uint32_t>(1, levels[0].get_pixel_width() >> level_index)) ||
							(levels[level_index].get_pixel_height() != maximum<uint32_t>(1, levels[0].get_pixel_height() >> level_index)) )
					{
						// Malformed mipmap chain
						assert(0);
						return false;
					}
				}

				if (fmt != levels[level_index].get_format())
				{
					// All input textures must use the same GPU format
					assert(0);
					return false;
				}
			}
		}

		uint32_t internal_fmt = KTX_ETC1_RGB8_OES, base_internal_fmt = KTX_RGB;

		switch (fmt)
		{
		case texture_format::cBC1:
		case texture_format::cBC1_NV:
		case texture_format::cBC1_AMD:
		{
			internal_fmt = KTX_COMPRESSED_RGB_S3TC_DXT1_EXT;
			break;
		}
		case texture_format::cBC3:
		{
			internal_fmt = KTX_COMPRESSED_RGBA_S3TC_DXT5_EXT;
			base_internal_fmt = KTX_RGBA;
			break;
		}
		case texture_format::cBC4:
		{
			internal_fmt = KTX_COMPRESSED_RED_RGTC1_EXT;// KTX_COMPRESSED_LUMINANCE_LATC1_EXT;
			base_internal_fmt = KTX_RED;
			break;
		}
		case texture_format::cBC5:
		{
			internal_fmt = KTX_COMPRESSED_RED_GREEN_RGTC2_EXT;
			base_internal_fmt = KTX_RG;
			break;
		}
		case texture_format::cETC1:
		case texture_format::cETC1S:
		{
			internal_fmt = KTX_ETC1_RGB8_OES;
			break;
		}
		case texture_format::cETC2_RGB:
		{
			internal_fmt = KTX_COMPRESSED_RGB8_ETC2;
			break;
		}
		case texture_format::cETC2_RGBA:
		{
			internal_fmt = KTX_COMPRESSED_RGBA8_ETC2_EAC;
			base_internal_fmt = KTX_RGBA;
			break;
		}
		case texture_format::cBC6HSigned:
		{
			internal_fmt = KTX_COMPRESSED_RGB_BPTC_SIGNED_FLOAT;
			base_internal_fmt = KTX_RGBA;
			break;
		}
		case texture_format::cBC6HUnsigned:
		{
			internal_fmt = KTX_COMPRESSED_RGB_BPTC_UNSIGNED_FLOAT;
			base_internal_fmt = KTX_RGBA;
			break;
		}
		case texture_format::cBC7:
		{
			internal_fmt = KTX_COMPRESSED_RGBA_BPTC_UNORM;
			base_internal_fmt = KTX_RGBA;
			break;
		}
		case texture_format::cPVRTC1_4_RGB:
		{
			internal_fmt = KTX_COMPRESSED_RGB_PVRTC_4BPPV1_IMG;
			break;
		}
		case texture_format::cPVRTC1_4_RGBA:
		{
			internal_fmt = KTX_COMPRESSED_RGBA_PVRTC_4BPPV1_IMG;
			base_internal_fmt = KTX_RGBA;
			break;
		}
		case texture_format::cASTC_HDR_6x6:
		{
			internal_fmt = KTX_COMPRESSED_RGBA_ASTC_6x6_KHR;
			// TODO: should we write RGB? We don't support generating HDR 6x6 with alpha.
			base_internal_fmt = KTX_RGBA; 
			break;
		}
		// We use different enums for HDR vs. LDR ASTC, but internally they are both just ASTC.
		case texture_format::cASTC_HDR_4x4:
		case texture_format::cUASTC_HDR_4x4: // UASTC_HDR 4x4 is just HDR-only ASTC
		{
			internal_fmt = KTX_COMPRESSED_RGBA_ASTC_4x4_KHR;
			base_internal_fmt = KTX_RGBA;
			break;
		}
		case texture_format::cASTC_LDR_4x4:
		{
			internal_fmt = !astc_srgb_flag ? KTX_COMPRESSED_RGBA_ASTC_4x4_KHR : KTX_COMPRESSED_SRGB8_ALPHA8_ASTC_4x4_KHR;
			base_internal_fmt = KTX_RGBA;
			break;
		}
		case texture_format::cASTC_LDR_5x4:
		{
			internal_fmt = !astc_srgb_flag ? KTX_COMPRESSED_RGBA_ASTC_5x4_KHR : KTX_COMPRESSED_SRGB8_ALPHA8_ASTC_5x4_KHR;
			base_internal_fmt = KTX_RGBA;
			break;
		}
		case texture_format::cASTC_LDR_5x5:
		{
			internal_fmt = !astc_srgb_flag ? KTX_COMPRESSED_RGBA_ASTC_5x5_KHR : KTX_COMPRESSED_SRGB8_ALPHA8_ASTC_5x5_KHR;
			base_internal_fmt = KTX_RGBA;
			break;
		}
		case texture_format::cASTC_LDR_6x5:
		{
			internal_fmt = !astc_srgb_flag ? KTX_COMPRESSED_RGBA_ASTC_6x5_KHR : KTX_COMPRESSED_SRGB8_ALPHA8_ASTC_6x5_KHR;
			base_internal_fmt = KTX_RGBA;
			break;
		}
		case texture_format::cASTC_LDR_6x6:
		{
			internal_fmt = !astc_srgb_flag ? KTX_COMPRESSED_RGBA_ASTC_6x6_KHR : KTX_COMPRESSED_SRGB8_ALPHA8_ASTC_6x6_KHR;
			base_internal_fmt = KTX_RGBA;
			break;
		}
		case texture_format::cASTC_LDR_8x5:
		{
			internal_fmt = !astc_srgb_flag ? KTX_COMPRESSED_RGBA_ASTC_8x5_KHR : KTX_COMPRESSED_SRGB8_ALPHA8_ASTC_8x5_KHR;
			base_internal_fmt = KTX_RGBA;
			break;
		}
		case texture_format::cASTC_LDR_8x6:
		{
			internal_fmt = !astc_srgb_flag ? KTX_COMPRESSED_RGBA_ASTC_8x6_KHR : KTX_COMPRESSED_SRGB8_ALPHA8_ASTC_8x6_KHR;
			base_internal_fmt = KTX_RGBA;
			break;
		}
		case texture_format::cASTC_LDR_10x5:
		{
			internal_fmt = !astc_srgb_flag ? KTX_COMPRESSED_RGBA_ASTC_10x5_KHR : KTX_COMPRESSED_SRGB8_ALPHA8_ASTC_10x5_KHR;
			base_internal_fmt = KTX_RGBA;
			break;
		}
		case texture_format::cASTC_LDR_10x6:
		{
			internal_fmt = !astc_srgb_flag ? KTX_COMPRESSED_RGBA_ASTC_10x6_KHR : KTX_COMPRESSED_SRGB8_ALPHA8_ASTC_10x6_KHR;
			base_internal_fmt = KTX_RGBA;
			break;
		}
		case texture_format::cASTC_LDR_8x8:
		{
			internal_fmt = !astc_srgb_flag ? KTX_COMPRESSED_RGBA_ASTC_8x8_KHR : KTX_COMPRESSED_SRGB8_ALPHA8_ASTC_8x8_KHR;
			base_internal_fmt = KTX_RGBA;
			break;
		}
		case texture_format::cASTC_LDR_10x8:
		{
			internal_fmt = !astc_srgb_flag ? KTX_COMPRESSED_RGBA_ASTC_10x8_KHR : KTX_COMPRESSED_SRGB8_ALPHA8_ASTC_10x8_KHR;
			base_internal_fmt = KTX_RGBA;
			break;
		}
		case texture_format::cASTC_LDR_10x10:
		{
			internal_fmt = !astc_srgb_flag ? KTX_COMPRESSED_RGBA_ASTC_10x10_KHR : KTX_COMPRESSED_SRGB8_ALPHA8_ASTC_10x10_KHR;
			base_internal_fmt = KTX_RGBA;
			break;
		}
		case texture_format::cASTC_LDR_12x10:
		{
			internal_fmt = !astc_srgb_flag ? KTX_COMPRESSED_RGBA_ASTC_12x10_KHR : KTX_COMPRESSED_SRGB8_ALPHA8_ASTC_12x10_KHR;
			base_internal_fmt = KTX_RGBA;
			break;
		}
		case texture_format::cASTC_LDR_12x12:
		{
			internal_fmt = !astc_srgb_flag ? KTX_COMPRESSED_RGBA_ASTC_12x12_KHR : KTX_COMPRESSED_SRGB8_ALPHA8_ASTC_12x12_KHR;
			base_internal_fmt = KTX_RGBA;
			break;
		}
		case texture_format::cATC_RGB:
		{
			internal_fmt = KTX_ATC_RGB_AMD;
			break;
		}
		case texture_format::cATC_RGBA_INTERPOLATED_ALPHA:
		{
			internal_fmt = KTX_ATC_RGBA_INTERPOLATED_ALPHA_AMD;
			base_internal_fmt = KTX_RGBA;
			break;
		}
		case texture_format::cETC2_R11_EAC:
		{
			internal_fmt = KTX_COMPRESSED_R11_EAC;
			base_internal_fmt = KTX_RED;
			break;
		}
		case texture_format::cETC2_RG11_EAC:
		{
			internal_fmt = KTX_COMPRESSED_RG11_EAC;
			base_internal_fmt = KTX_RG;
			break;
		}
		case texture_format::cUASTC4x4:
		{
			internal_fmt = KTX_COMPRESSED_RGBA_UASTC_4x4_KHR;
			base_internal_fmt = KTX_RGBA;
			break;
		}
		case texture_format::cFXT1_RGB:
		{
			internal_fmt = KTX_COMPRESSED_RGB_FXT1_3DFX;
			break;
		}
		case texture_format::cPVRTC2_4_RGBA:
		{
			internal_fmt = KTX_COMPRESSED_RGBA_PVRTC_4BPPV2_IMG;
			base_internal_fmt = KTX_RGBA;
			break;
		}
		default:
		{
			// TODO
			assert(0);
			return false;
		}
		}

		ktx_header header;
		header.clear();
		memcpy(&header.m_identifier, g_ktx_file_id, sizeof(g_ktx_file_id));
		header.m_endianness = KTX_ENDIAN;

		header.m_pixelWidth = width;
		header.m_pixelHeight = height;

		header.m_glTypeSize = 1;

		header.m_glInternalFormat = internal_fmt;
		header.m_glBaseInternalFormat = base_internal_fmt;

		header.m_numberOfArrayElements = (uint32_t)(cubemap_flag ? (gpu_images.size() / 6) : gpu_images.size());
		if (header.m_numberOfArrayElements == 1)
			header.m_numberOfArrayElements = 0;

		header.m_numberOfMipmapLevels = total_levels;
		header.m_numberOfFaces = cubemap_flag ? 6 : 1;

		append_vector(ktx_data, (uint8_t*)&header, sizeof(header));

		fmt_debug_printf("create_ktx_texture_file: {}x{}, astc_srgb_flag: {}, basis::texture_format: {}, internalFormat: {}, baseInternalFormat: {}, arrayElements: {}, faces: {}, mipLevels: {}\n",
			width, height, astc_srgb_flag, (uint32_t)fmt,
			(uint32_t)header.m_glInternalFormat, (uint32_t)header.m_glBaseInternalFormat,
			(uint32_t)header.m_numberOfArrayElements, (uint32_t)header.m_numberOfFaces,
			(uint32_t)header.m_numberOfMipmapLevels);

		for (uint32_t level_index = 0; level_index < total_levels; level_index++)
		{
			uint32_t img_size = gpu_images[0][level_index].get_size_in_bytes();

			if ((header.m_numberOfFaces == 1) || (header.m_numberOfArrayElements > 1))
			{
				img_size = img_size * header.m_numberOfFaces * maximum<uint32_t>(1, header.m_numberOfArrayElements);
			}

			assert(img_size && ((img_size & 3) == 0));

			packed_uint<4> packed_img_size(img_size);
			append_vector(ktx_data, (uint8_t*)&packed_img_size, sizeof(packed_img_size));

			uint32_t bytes_written = 0;
			(void)bytes_written;

			for (uint32_t array_index = 0; array_index < maximum<uint32_t>(1, header.m_numberOfArrayElements); array_index++)
			{
				for (uint32_t face_index = 0; face_index < header.m_numberOfFaces; face_index++)
				{
					const gpu_image& img = gpu_images[cubemap_flag ? (array_index * 6 + face_index) : array_index][level_index];

					append_vector(ktx_data, (uint8_t*)img.get_ptr(), img.get_size_in_bytes());

					bytes_written += img.get_size_in_bytes();
				}

			} // array_index

		} // level_index

		return true;
	}

	bool does_dds_support_format(texture_format fmt)
	{
		switch (fmt)
		{
		case texture_format::cBC1_NV:
		case texture_format::cBC1_AMD:
		case texture_format::cBC1:
		case texture_format::cBC3:
		case texture_format::cBC4:
		case texture_format::cBC5:
		case texture_format::cBC6HSigned:
		case texture_format::cBC6HUnsigned:
		case texture_format::cBC7:
			return true;
		default:
			break;
		}
		return false;
	}

	// Only supports the basic DirectX BC texture formats.
	// gpu_images array is: [face/layer][mipmap level]
	// For cubemap arrays, # of face/layers must be a multiple of 6.
	// Accepts 2D, 2D mipmapped, 2D array, 2D array mipmapped
	// and cubemap, cubemap mipmapped, and cubemap array mipmapped.
	bool write_dds_file(uint8_vec &dds_data, const basisu::vector<gpu_image_vec>& gpu_images, bool cubemap_flag, bool use_srgb_format)
	{
		if (!gpu_images.size())
		{
			assert(0);
			return false;
		}

		// Sanity check the input
		uint32_t slices = 1;
		if (cubemap_flag)
		{
			if ((gpu_images.size() % 6) != 0)
			{
				assert(0);
				return false;
			}
			slices = gpu_images.size_u32() / 6;
		}
		else
		{
			slices = gpu_images.size_u32();
		}

		uint32_t width = 0, height = 0, total_levels = 0;
		basisu::texture_format fmt = texture_format::cInvalidTextureFormat;

		// Sanity check the input for consistent # of dimensions and mip levels
		for (uint32_t array_index = 0; array_index < gpu_images.size(); array_index++)
		{
			const gpu_image_vec& levels = gpu_images[array_index];

			if (!levels.size())
			{
				// Empty mip chain
				assert(0);
				return false;
			}

			if (!array_index)
			{
				width = levels[0].get_pixel_width();
				height = levels[0].get_pixel_height();
				total_levels = (uint32_t)levels.size();
				fmt = levels[0].get_format();
			}
			else
			{
				if ((width != levels[0].get_pixel_width()) ||
					(height != levels[0].get_pixel_height()) ||
					(total_levels != levels.size()))
				{
					// All cubemap/texture array faces must be the same dimension
					assert(0);
					return false;
				}
			}

			for (uint32_t level_index = 0; level_index < levels.size(); level_index++)
			{
				if (level_index)
				{
					if ((levels[level_index].get_pixel_width() != maximum<uint32_t>(1, levels[0].get_pixel_width() >> level_index)) ||
						(levels[level_index].get_pixel_height() != maximum<uint32_t>(1, levels[0].get_pixel_height() >> level_index)))
					{
						// Malformed mipmap chain
						assert(0);
						return false;
					}
				}

				if (fmt != levels[level_index].get_format())
				{
					// All input textures must use the same GPU format
					assert(0);
					return false;
				}
			}
		}

		// No mipmap levels
		if (!total_levels)
		{
			assert(0);
			return false;
		}

		// Create the DDS mipmap level data
		uint8_vec mipmaps[32];

		// See https://learn.microsoft.com/en-us/windows/win32/direct3ddds/dds-file-layout-for-cubic-environment-maps
		// DDS cubemap organization is cubemap face 0 followed by all mips, then cubemap face 1 followed by all mips, etc.
		// Unfortunately tinydds.h's writer doesn't handle this case correctly, so we work around it here.
		// This also applies with 2D texture arrays, too. RenderDoc and ddsview (DirectXTex) views each type (cubemap array and 2D texture array) correctly.
		// Also see "Using Texture Arrays in Direct3D 10/11":
		// https://learn.microsoft.com/en-us/windows/win32/direct3ddds/dx-graphics-dds-pguide
		for (uint32_t array_index = 0; array_index < gpu_images.size(); array_index++)
		{
			const gpu_image_vec& levels = gpu_images[array_index];

			for (uint32_t level_index = 0; level_index < levels.size(); level_index++)
			{
				append_vector(mipmaps[0], (uint8_t*)levels[level_index].get_ptr(), levels[level_index].get_size_in_bytes());

			} // level_index
		} // array_index

#if 0
		// This organization, required by tinydds.h's API, is wrong.
		{
			for (uint32_t array_index = 0; array_index < gpu_images.size(); array_index++)
			{
				const gpu_image_vec& levels = gpu_images[array_index];

				for (uint32_t level_index = 0; level_index < levels.size(); level_index++)
				{
					append_vector(mipmaps[level_index], (uint8_t*)levels[level_index].get_ptr(), levels[level_index].get_size_in_bytes());

				} // level_index
			} // array_index
		}
#endif
		
		// Write DDS file using tinydds
		TinyDDS_WriteCallbacks cbs;
		cbs.error = [](void* user, char const* msg) { BASISU_NOTE_UNUSED(user);  fprintf(stderr, "tinydds: %s\n", msg); };
		cbs.alloc = [](void* user, size_t size) -> void* { BASISU_NOTE_UNUSED(user); return malloc(size); };
		cbs.free = [](void* user, void* memory) { BASISU_NOTE_UNUSED(user); free(memory); };
		cbs.write = [](void* user, void const* buffer, size_t byteCount) { BASISU_NOTE_UNUSED(user); uint8_vec* pVec = (uint8_vec*)user; append_vector(*pVec, (const uint8_t*)buffer, byteCount); };

		uint32_t mipmap_sizes[32];
		const void* mipmap_ptrs[32];
		
		clear_obj(mipmap_sizes);
		clear_obj(mipmap_ptrs);

		assert(total_levels < 32);
		for (uint32_t i = 0; i < total_levels; i++)
		{
			mipmap_sizes[i] = mipmaps[i].size_in_bytes_u32();
			mipmap_ptrs[i] = mipmaps[i].get_ptr();
		}

		// Select tinydds texture format
		uint32_t tinydds_fmt = 0;

		switch (fmt)
		{
			case texture_format::cBC1_NV:
			case texture_format::cBC1_AMD:
			case texture_format::cBC1: 
				tinydds_fmt = use_srgb_format ? TDDS_BC1_RGBA_SRGB_BLOCK : TDDS_BC1_RGBA_UNORM_BLOCK;
				break;
			case texture_format::cBC3:
				tinydds_fmt = use_srgb_format ? TDDS_BC3_SRGB_BLOCK : TDDS_BC3_UNORM_BLOCK;
				break;
			case texture_format::cBC4:
				tinydds_fmt = TDDS_BC4_UNORM_BLOCK;
				break;
			case texture_format::cBC5:
				tinydds_fmt = TDDS_BC5_UNORM_BLOCK;
				break;
			case texture_format::cBC6HSigned:
				tinydds_fmt = TDDS_BC6H_SFLOAT_BLOCK;
				break;
			case texture_format::cBC6HUnsigned:
				tinydds_fmt = TDDS_BC6H_UFLOAT_BLOCK;
				break;
			case texture_format::cBC7:
				tinydds_fmt = use_srgb_format ? TDDS_BC7_SRGB_BLOCK : TDDS_BC7_UNORM_BLOCK;
				break;
			default:
			{
				fprintf(stderr, "Warning: Unsupported format in write_dds_file().\n");
				return false;
			}
		}

		// Note DirectXTex's DDSView doesn't handle odd sizes textures correctly. RenderDoc loads them fine, however.
		
		fmt_debug_printf("write_dds_file: {}x{}, basis::texture_format: {}, tinydds_fmt: {}, slices: {}, mipLevels: {}, cubemap_flag: {}, use_srgb_format: {}\n",
			width, height, (uint32_t)fmt, tinydds_fmt, slices, total_levels, cubemap_flag, use_srgb_format);

		bool status = TinyDDS_WriteImage(&cbs,
			&dds_data,
			width,
			height,
			1,
			slices,
			total_levels,
			(TinyDDS_Format)tinydds_fmt,
			cubemap_flag,
			true,
			mipmap_sizes,
			mipmap_ptrs);

		if (!status)
		{
			fprintf(stderr, "write_dds_file: Failed creating DDS file\n");
			return false;
		}
								
		return true;
	}

	bool write_dds_file(const char* pFilename, const basisu::vector<gpu_image_vec>& gpu_images, bool cubemap_flag, bool use_srgb_format)
	{
		uint8_vec dds_data;

		if (!write_dds_file(dds_data, gpu_images, cubemap_flag, use_srgb_format))
			return false;

		if (!write_vec_to_file(pFilename, dds_data))
		{
			fprintf(stderr, "write_dds_file: Failed writing DDS file data\n");
			return false;
		}

		return true;
	}
		
	bool read_uncompressed_dds_file(const char* pFilename, basisu::vector<image> &ldr_mips,	basisu::vector<imagef>& hdr_mips)
	{
		const uint32_t MAX_IMAGE_DIM = 16384;

		TinyDDS_Callbacks cbs;

		cbs.errorFn = [](void* user, char const* msg) { BASISU_NOTE_UNUSED(user); fprintf(stderr, "tinydds: %s\n", msg); };
		cbs.allocFn = [](void* user, size_t size) -> void* { BASISU_NOTE_UNUSED(user); return malloc(size); };
		cbs.freeFn = [](void* user, void* memory) { BASISU_NOTE_UNUSED(user); free(memory); };
		cbs.readFn = [](void* user, void* buffer, size_t byteCount) -> size_t { return (size_t)fread(buffer, 1, byteCount, (FILE*)user); };
		
#ifdef _MSC_VER
		cbs.seekFn = [](void* user, int64_t ofs) -> bool { return _fseeki64((FILE*)user, ofs, SEEK_SET) == 0; };
		cbs.tellFn = [](void* user) -> int64_t { return _ftelli64((FILE*)user); };
#else
		cbs.seekFn = [](void* user, int64_t ofs) -> bool { return fseek((FILE*)user, (long)ofs, SEEK_SET) == 0; };
		cbs.tellFn = [](void* user) -> int64_t { return (int64_t)ftell((FILE*)user); };
#endif

		FILE* pFile = fopen_safe(pFilename, "rb");
		if (!pFile)
		{
			error_printf("Can't open .DDS file \"%s\"\n", pFilename);
			return false;
		}

		// These are the formats AMD Compressonator supports in its UI.
		enum dds_fmt
		{
			cRGBA32,
			cRGBA_HALF,
			cRGBA_FLOAT
		};

		bool status = false;
		dds_fmt fmt = cRGBA32;
		uint32_t width = 0, height = 0;
		bool hdr_flag = false;
		TinyDDS_Format tfmt = TDDS_UNDEFINED;

		TinyDDS_ContextHandle ctx = TinyDDS_CreateContext(&cbs, pFile);
		if (!ctx)
			goto failure;

		status = TinyDDS_ReadHeader(ctx);
		if (!status)
		{
			error_printf("Failed parsing DDS header in file \"%s\"\n", pFilename);
			goto failure;
		}
				
		if ((!TinyDDS_Is2D(ctx)) || (TinyDDS_ArraySlices(ctx) > 1) || (TinyDDS_IsCubemap(ctx)))
		{
			error_printf("Unsupported DDS texture type in file \"%s\"\n", pFilename);
			goto failure;
		}

		width = TinyDDS_Width(ctx);
		height = TinyDDS_Height(ctx);
						
		if (!width || !height)
		{
			error_printf("DDS texture dimensions invalid in file \"%s\"\n", pFilename);
			goto failure;
		}

		if ((width > MAX_IMAGE_DIM) || (height > MAX_IMAGE_DIM))
		{
			error_printf("DDS texture dimensions too large in file \"%s\"\n", pFilename);
			goto failure;
		}
		
		tfmt = TinyDDS_GetFormat(ctx);
		switch (tfmt)
		{
		case TDDS_R8G8B8A8_SRGB:
		case TDDS_R8G8B8A8_UNORM:
		case TDDS_B8G8R8A8_SRGB:
		case TDDS_B8G8R8A8_UNORM:
			fmt = cRGBA32;
			break;
		case TDDS_R16G16B16A16_SFLOAT:
			fmt = cRGBA_HALF;
			hdr_flag = true;
			break;
		case TDDS_R32G32B32A32_SFLOAT:
			fmt = cRGBA_FLOAT;
			hdr_flag = true;
			break;
		default:
			error_printf("File \"%s\" has an unsupported DDS texture format (only supports RGBA/BGRA 32bpp, RGBA HALF float, or RGBA FLOAT)\n", pFilename);
			goto failure;
		}

		if (hdr_flag)
			hdr_mips.resize(TinyDDS_NumberOfMipmaps(ctx));
		else
			ldr_mips.resize(TinyDDS_NumberOfMipmaps(ctx));

		for (uint32_t level = 0; level < TinyDDS_NumberOfMipmaps(ctx); level++)
		{
			const uint32_t level_width = TinyDDS_MipMapReduce(width, level);
			const uint32_t level_height = TinyDDS_MipMapReduce(height, level);
			const uint32_t total_level_texels = level_width * level_height;

			const void* pImage = TinyDDS_ImageRawData(ctx, level);
			const uint32_t image_size = TinyDDS_ImageSize(ctx, level);

			if (fmt == cRGBA32)
			{
				ldr_mips[level].resize(level_width, level_height);

				if ((ldr_mips[level].get_total_pixels() * sizeof(uint32_t) != image_size))
				{
					assert(0);
					goto failure;
				}

				memcpy(ldr_mips[level].get_ptr(), pImage, image_size);
								
				if ((tfmt == TDDS_B8G8R8A8_SRGB) || (tfmt == TDDS_B8G8R8A8_UNORM))
				{
					// Swap R and B components.
					uint32_t *pTexels = (uint32_t *)ldr_mips[level].get_ptr();
					for (uint32_t i = 0; i < total_level_texels; i++)
					{
						const uint32_t v = pTexels[i];
						const uint32_t r = (v >> 16) & 0xFF;
						const uint32_t b = v & 0xFF;
						pTexels[i] = r | (b << 16) | (v & 0xFF00FF00);
					}
				}
			}
			else if (fmt == cRGBA_FLOAT)
			{
				hdr_mips[level].resize(level_width, level_height);

				if ((hdr_mips[level].get_total_pixels() * sizeof(float) * 4 != image_size))
				{
					assert(0);
					goto failure;
				}

				memcpy((void *)hdr_mips[level].get_ptr(), pImage, image_size);
			}
			else if (fmt == cRGBA_HALF)
			{
				hdr_mips[level].resize(level_width, level_height);
				
				if ((hdr_mips[level].get_total_pixels() * sizeof(basist::half_float) * 4 != image_size))
				{
					assert(0);
					goto failure;
				}

				// Unpack half to float.
				const basist::half_float* pSrc_comps = static_cast<const basist::half_float*>(pImage);
				vec4F* pDst_texels = hdr_mips[level].get_ptr();
				
				for (uint32_t i = 0; i < total_level_texels; i++)
				{
					(*pDst_texels)[0] = basist::half_to_float(pSrc_comps[0]);
					(*pDst_texels)[1] = basist::half_to_float(pSrc_comps[1]);
					(*pDst_texels)[2] = basist::half_to_float(pSrc_comps[2]);
					(*pDst_texels)[3] = basist::half_to_float(pSrc_comps[3]);

					pSrc_comps += 4;
					pDst_texels++;
				} // y
			}
		} // level

		TinyDDS_DestroyContext(ctx);
		fclose(pFile);

		return true;

	failure:
		if (ctx)
			TinyDDS_DestroyContext(ctx);

		if (pFile)
			fclose(pFile);

		return false;
	}

	bool write_compressed_texture_file(const char* pFilename, const basisu::vector<gpu_image_vec>& g, bool cubemap_flag, bool use_srgb_format)
	{
		std::string extension(string_tolower(string_get_extension(pFilename)));

		uint8_vec filedata;
		if (extension == "ktx")
		{
			if (!create_ktx_texture_file(filedata, g, cubemap_flag, use_srgb_format))
				return false;
		}
		else if (extension == "pvr")
		{
			// TODO
			return false;
		}
		else if (extension == "dds")
		{
			if (!write_dds_file(filedata, g, cubemap_flag, use_srgb_format))
				return false;
		}
		else
		{
			// unsupported texture format
			assert(0);
			return false;
		}

		return basisu::write_vec_to_file(pFilename, filedata);
	}

	bool write_compressed_texture_file(const char* pFilename, const gpu_image_vec& g, bool use_srgb_format)
	{
		basisu::vector<gpu_image_vec> a;
		a.push_back(g);
		return write_compressed_texture_file(pFilename, a, false, use_srgb_format);
	}

	bool write_compressed_texture_file(const char* pFilename, const gpu_image& g, bool use_srgb_format)
	{
		basisu::vector<gpu_image_vec> v;
		enlarge_vector(v, 1)->push_back(g);
		return write_compressed_texture_file(pFilename, v, false, use_srgb_format);
	}

	//const uint32_t OUT_FILE_MAGIC = 'TEXC';
	struct out_file_header 
	{
		packed_uint<4> m_magic;
		packed_uint<4> m_pad;
		packed_uint<4> m_width;
		packed_uint<4> m_height;
	};

	// As no modern tool supports FXT1 format .KTX files, let's write .OUT files and make sure 3DFX's original tools shipped in 1999 can decode our encoded output.
	bool write_3dfx_out_file(const char* pFilename, const gpu_image& gi)
	{
		out_file_header hdr;
		//hdr.m_magic = OUT_FILE_MAGIC;
		hdr.m_magic.m_bytes[0] = 67;
		hdr.m_magic.m_bytes[1] = 88;
		hdr.m_magic.m_bytes[2] = 69;
		hdr.m_magic.m_bytes[3] = 84;
		hdr.m_pad = 0;
		hdr.m_width = gi.get_blocks_x() * 8;
		hdr.m_height = gi.get_blocks_y() * 4;

		FILE* pFile = nullptr;
#ifdef _WIN32
		fopen_s(&pFile, pFilename, "wb");
#else
		pFile = fopen(pFilename, "wb");
#endif
		if (!pFile)
			return false;

		fwrite(&hdr, sizeof(hdr), 1, pFile);
		fwrite(gi.get_ptr(), gi.get_size_in_bytes(), 1, pFile);
		
		return fclose(pFile) != EOF;
	}

#pragma pack(push, 1)
	struct astc_file_header
	{
		uint8_t m_sig[4];
		uint8_t m_block_dim[3];
		uint8_t m_width[3];
		uint8_t m_height[3];
		uint8_t m_depth[3];
	};
#pragma pack(pop)

	bool read_astc_file(const uint8_t *pImage_data, size_t image_data_size, vector2D<astc_helpers::astc_block>& blocks, uint32_t &block_width, uint32_t &block_height, uint32_t &width, uint32_t &height)
	{
		block_width = 0;
		block_height = 0;
		width = 0;
		height = 0;
		blocks.resize(0, 0);

		if (image_data_size < (sizeof(astc_file_header) + sizeof(astc_helpers::astc_block)))
			return false;

		const astc_file_header* pHeader = reinterpret_cast<const astc_file_header*>(pImage_data);

		if ((pHeader->m_sig[0] != 0x13) || (pHeader->m_sig[1] != 0xAB) || (pHeader->m_sig[2] != 0xA1) || (pHeader->m_sig[3] != 0x5C))
			return false;

		const uint32_t block_depth = pHeader->m_block_dim[2];
		if (block_depth != 1)
			return false;

		if ((pHeader->m_depth[0] != 1) || (pHeader->m_depth[1] != 0) || (pHeader->m_depth[2] != 0))
			return false;

		block_width = pHeader->m_block_dim[0];
		block_height = pHeader->m_block_dim[1];
				
		if (!astc_helpers::is_valid_block_size(block_width, block_height))
			return false;
				
		width = pHeader->m_width[0] | ((uint32_t)pHeader->m_width[1] << 8u) | ((uint32_t)pHeader->m_width[2] << 16u);
		height = pHeader->m_height[0] | ((uint32_t)pHeader->m_height[1] << 8u) | ((uint32_t)pHeader->m_height[2] << 16u);
		
		const uint32_t MAX_DIM = 32768;
		if ((!width) || (width > MAX_DIM) || (!height) || (height > MAX_DIM))
			return false;

		const uint32_t num_blocks_x = (width + block_width - 1) / block_width;
		const uint32_t num_blocks_y = (height + block_height - 1) / block_height;
		const uint32_t total_blocks = num_blocks_x * num_blocks_y;
		
		size_t total_expected_size = sizeof(astc_file_header) + (size_t)total_blocks * sizeof(astc_helpers::astc_block);
		if (image_data_size < total_expected_size)
			return false;

		if (!blocks.try_resize(num_blocks_x, num_blocks_y))
			return false;

		memcpy(blocks.get_ptr(), pImage_data + sizeof(astc_file_header), (size_t)total_blocks * sizeof(astc_helpers::astc_block));

		return true;
	}

	bool read_astc_file(const char* pFilename, vector2D<astc_helpers::astc_block>& blocks, uint32_t& block_width, uint32_t& block_height, uint32_t& width, uint32_t& height)
	{
		uint8_vec file_data;
		if (!read_file_to_vec(pFilename, file_data))
			return false;

		if (!file_data.size())
			return false;

		return read_astc_file(file_data.get_ptr(), file_data.size(), blocks, block_width, block_height, width, height);
	}

	// The .astc texture format is readable using ARM's astcenc, AMD Compressonator, and other engines/tools. It oddly doesn't support mipmaps, limiting 
	// its usefulness/relevance.
	// https://github.com/ARM-software/astc-encoder/blob/main/Docs/FileFormat.md
	bool write_astc_file(const char* pFilename, const void* pBlocks, uint32_t block_width, uint32_t block_height, uint32_t dim_x, uint32_t dim_y)
	{
		assert(pBlocks && (dim_x > 0) && (dim_y > 0));
		assert(astc_helpers::is_valid_block_size(block_width, block_height));

		uint8_vec file_data;
		file_data.push_back(0x13);
		file_data.push_back(0xAB);
		file_data.push_back(0xA1);
		file_data.push_back(0x5C);

		file_data.push_back((uint8_t)block_width);
		file_data.push_back((uint8_t)block_height);
		file_data.push_back(1);

		file_data.push_back((uint8_t)dim_x);
		file_data.push_back((uint8_t)(dim_x >> 8));
		file_data.push_back((uint8_t)(dim_x >> 16));

		file_data.push_back((uint8_t)dim_y);
		file_data.push_back((uint8_t)(dim_y >> 8));
		file_data.push_back((uint8_t)(dim_y >> 16));

		file_data.push_back((uint8_t)1);
		file_data.push_back((uint8_t)0);
		file_data.push_back((uint8_t)0);

		const uint32_t num_blocks_x = (dim_x + block_width - 1) / block_width;
		const uint32_t num_blocks_y = (dim_y + block_height - 1) / block_height;

		const uint32_t total_bytes = num_blocks_x * num_blocks_y * 16;

		const size_t cur_size = file_data.size();

		file_data.resize(cur_size + total_bytes);

		memcpy(&file_data[cur_size], pBlocks, total_bytes);

		return write_vec_to_file(pFilename, file_data);
	}
		
} // basisu

