// basisu_transcoder_internal.h - Universal texture format transcoder library.
// Copyright (C) 2019-2026 Binomial LLC. All Rights Reserved.
//
// Important: If compiling with gcc, be sure strict aliasing is disabled: -fno-strict-aliasing
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
#pragma once

#ifdef _MSC_VER
#pragma warning (disable: 4127) //  conditional expression is constant
#endif

// v1.50: Added UASTC HDR 4x4 support
// v1.60: Added RDO ASTC HDR 6x6 and intermediate support
// v1.65: Added ASTC LDR 4x4-12x12 and XUASTC LDR 4x4-12x12 (not publically released)
// v2.00: Added unified effort/quality options across all formats, fast direct transcoding of XUASTC 4x4/6x6/8x6 to BC7, adaptive deblocking, ZStd or arithmetic profiles, weight grid DCT
// v2.10: Khronos modifications to KTX2 file format for UASTC HDR 6x6i support for KTX-Software compatiblity (we're also modifying how XUASTC LDR files use KTX2 to be compatible)
#define BASISD_LIB_VERSION 210
#define BASISD_VERSION_STRING "02.10"

#ifdef _DEBUG
#define BASISD_BUILD_DEBUG
#else
#define BASISD_BUILD_RELEASE
#endif

#include "basisu.h"
#include "basisu_astc_helpers.h"

#define BASISD_znew (z = 36969 * (z & 65535) + (z >> 16))

namespace basisu
{
	extern bool g_debug_printf;
}

namespace basist
{
	// Low-level formats directly supported by the transcoder (other supported texture formats are combinations of these low-level block formats).
	// You probably don't care about these enum's unless you are going pretty low-level and calling the transcoder to decode individual slices.
	enum class block_format
	{
		cETC1,								// ETC1S RGB 
		cETC2_RGBA,							// full ETC2 EAC RGBA8 block
		cBC1,								// DXT1 RGB 
		cBC3,								// BC4 block followed by a four color BC1 block
		cBC4,								// DXT5A (alpha block only)
		cBC5,								// two BC4 blocks
		cPVRTC1_4_RGB,						// opaque-only PVRTC1 4bpp
		cPVRTC1_4_RGBA,						// PVRTC1 4bpp RGBA
		cBC7,								// Full BC7 block, any mode
		cBC7_M5_COLOR,						// RGB BC7 mode 5 color (writes an opaque mode 5 block)
		cBC7_M5_ALPHA,						// alpha portion of BC7 mode 5 (cBC7_M5_COLOR output data must have been written to the output buffer first to set the mode/rot fields etc.)
		cETC2_EAC_A8,						// alpha block of ETC2 EAC (first 8 bytes of the 16-bit ETC2 EAC RGBA format)
		cASTC_LDR_4x4,						// ASTC LDR 4x4 (either color-only or color+alpha). Note that the transcoder always currently assumes sRGB decode mode is not enabled when outputting ASTC LDR for ETC1S/UASTC LDR 4x4.
											// data. If you use a sRGB ASTC format you'll get ~1 LSB of additional error, because of the different way ASTC decoders scale 8-bit endpoints to 16-bits during unpacking.
		
		cATC_RGB,
		cATC_RGBA_INTERPOLATED_ALPHA,
		cFXT1_RGB,							// Opaque-only, has oddball 8x4 pixel block size

		cPVRTC2_4_RGB,
		cPVRTC2_4_RGBA,

		cETC2_EAC_R11,
		cETC2_EAC_RG11,
												
		cIndices,							// Used internally: Write 16-bit endpoint and selector indices directly to output (output block must be at least 32-bits)

		cRGB32,								// Writes RGB components to 32bpp output pixels
		cRGBA32,							// Writes RGB255 components to 32bpp output pixels
		cA32,								// Writes alpha component to 32bpp output pixels
				
		cRGB565,
		cBGR565,
		
		cRGBA4444_COLOR,
		cRGBA4444_ALPHA,
		cRGBA4444_COLOR_OPAQUE,
		cRGBA4444,
		cRGBA_HALF,
		cRGB_HALF,
		cRGB_9E5,

		cUASTC_4x4,							// LDR, universal
		cUASTC_HDR_4x4,						// HDR, transcodes only to 4x4 HDR ASTC, BC6H, or uncompressed
		cBC6H,
		
		cASTC_HDR_4x4,
		cASTC_HDR_6x6,

		// The remaining ASTC LDR block sizes.
		cASTC_LDR_5x4,
		cASTC_LDR_5x5,
		cASTC_LDR_6x5,
		cASTC_LDR_6x6,
		cASTC_LDR_8x5,
		cASTC_LDR_8x6,
		cASTC_LDR_10x5,
		cASTC_LDR_10x6,
		cASTC_LDR_8x8,
		cASTC_LDR_10x8,
		cASTC_LDR_10x10,
		cASTC_LDR_12x10,
		cASTC_LDR_12x12,
										
		cTotalBlockFormats
	};

	inline bool block_format_is_hdr(block_format fmt)
	{
		switch (fmt)
		{
		case block_format::cUASTC_HDR_4x4:
		case block_format::cBC6H:
		case block_format::cASTC_HDR_4x4:
		case block_format::cASTC_HDR_6x6:
			return true;
		default:
			break;
		}

		return false;
	}

	// LDR or HDR ASTC?
	inline bool block_format_is_astc(block_format fmt)
	{
		switch (fmt)
		{
		case block_format::cASTC_LDR_4x4:
		case block_format::cASTC_LDR_5x4:
		case block_format::cASTC_LDR_5x5:
		case block_format::cASTC_LDR_6x5:
		case block_format::cASTC_LDR_6x6:
		case block_format::cASTC_LDR_8x5:
		case block_format::cASTC_LDR_8x6:
		case block_format::cASTC_LDR_10x5:
		case block_format::cASTC_LDR_10x6:
		case block_format::cASTC_LDR_8x8:
		case block_format::cASTC_LDR_10x8:
		case block_format::cASTC_LDR_10x10:
		case block_format::cASTC_LDR_12x10:
		case block_format::cASTC_LDR_12x12:
		case block_format::cASTC_HDR_4x4:
		case block_format::cASTC_HDR_6x6:
			return true;
		default:
			break;
		}

		return false;
	}

	inline uint32_t get_block_width(block_format fmt)
	{
		switch (fmt)
		{
		case block_format::cFXT1_RGB:
			return 8;
		case block_format::cASTC_HDR_6x6:
			return 6;

		case block_format::cASTC_LDR_5x4: return 5;
		case block_format::cASTC_LDR_5x5: return 5;
		case block_format::cASTC_LDR_6x5: return 6;
		case block_format::cASTC_LDR_6x6: return 6;
		case block_format::cASTC_LDR_8x5: return 8;
		case block_format::cASTC_LDR_8x6: return 8;
		case block_format::cASTC_LDR_10x5: return 10;
		case block_format::cASTC_LDR_10x6: return 10;
		case block_format::cASTC_LDR_8x8: return 8;
		case block_format::cASTC_LDR_10x8: return 10;
		case block_format::cASTC_LDR_10x10: return 10;
		case block_format::cASTC_LDR_12x10: return 12;
		case block_format::cASTC_LDR_12x12: return 12;

		default:
			break;
		}
		return 4;
	}

	inline uint32_t get_block_height(block_format fmt)
	{
		switch (fmt)
		{
		case block_format::cASTC_HDR_6x6:
			return 6;
					
		case block_format::cASTC_LDR_5x5: return 5;
		case block_format::cASTC_LDR_6x5: return 5;
		case block_format::cASTC_LDR_6x6: return 6;
		case block_format::cASTC_LDR_8x5: return 5;
		case block_format::cASTC_LDR_8x6: return 6;
		case block_format::cASTC_LDR_10x5: return 5;
		case block_format::cASTC_LDR_10x6: return 6;
		case block_format::cASTC_LDR_8x8: return 8;
		case block_format::cASTC_LDR_10x8: return 8;
		case block_format::cASTC_LDR_10x10: return 10;
		case block_format::cASTC_LDR_12x10: return 10;
		case block_format::cASTC_LDR_12x12: return 12;

		default:
			break;
		}
		return 4;
	}

	const int COLOR5_PAL0_PREV_HI = 9, COLOR5_PAL0_DELTA_LO = -9, COLOR5_PAL0_DELTA_HI = 31;
	const int COLOR5_PAL1_PREV_HI = 21, COLOR5_PAL1_DELTA_LO = -21, COLOR5_PAL1_DELTA_HI = 21;
	const int COLOR5_PAL2_PREV_HI = 31, COLOR5_PAL2_DELTA_LO = -31, COLOR5_PAL2_DELTA_HI = 9;
	const int COLOR5_PAL_MIN_DELTA_B_RUNLEN = 3, COLOR5_PAL_DELTA_5_RUNLEN_VLC_BITS = 3;

	const uint32_t ENDPOINT_PRED_TOTAL_SYMBOLS = (4 * 4 * 4 * 4) + 1;
	const uint32_t ENDPOINT_PRED_REPEAT_LAST_SYMBOL = ENDPOINT_PRED_TOTAL_SYMBOLS - 1;
	const uint32_t ENDPOINT_PRED_MIN_REPEAT_COUNT = 3;
	const uint32_t ENDPOINT_PRED_COUNT_VLC_BITS = 4;

	const uint32_t NUM_ENDPOINT_PREDS = 3;// BASISU_ARRAY_SIZE(g_endpoint_preds);
	const uint32_t CR_ENDPOINT_PRED_INDEX = NUM_ENDPOINT_PREDS - 1;
	const uint32_t NO_ENDPOINT_PRED_INDEX = 3;//NUM_ENDPOINT_PREDS;
	const uint32_t MAX_SELECTOR_HISTORY_BUF_SIZE = 64;
	const uint32_t SELECTOR_HISTORY_BUF_RLE_COUNT_THRESH = 3;
	const uint32_t SELECTOR_HISTORY_BUF_RLE_COUNT_BITS = 6;
	const uint32_t SELECTOR_HISTORY_BUF_RLE_COUNT_TOTAL = (1 << SELECTOR_HISTORY_BUF_RLE_COUNT_BITS);
		
	uint16_t crc16(const void *r, size_t size, uint16_t crc);

	uint32_t hash_hsieh(const uint8_t* pBuf, size_t len);

	template <typename Key>
	struct bit_hasher
	{
		inline std::size_t operator()(const Key& k) const
		{
			return hash_hsieh(reinterpret_cast<const uint8_t*>(&k), sizeof(k));
		}
	};

	struct string_hasher
	{
		inline std::size_t operator()(const std::string& k) const
		{
			size_t l = k.size();
			if (!l)
				return 0;
			return hash_hsieh(reinterpret_cast<const uint8_t*>(k.c_str()), l);
		}
	};

	class huffman_decoding_table
	{
		friend class bitwise_decoder;

	public:
		huffman_decoding_table()
		{
		}

		void clear()
		{
			basisu::clear_vector(m_code_sizes);
			basisu::clear_vector(m_lookup);
			basisu::clear_vector(m_tree);
		}

		bool init(uint32_t total_syms, const uint8_t *pCode_sizes, uint32_t fast_lookup_bits = basisu::cHuffmanFastLookupBits)
		{
			if (!total_syms)
			{
				clear();
				return true;
			}

			m_code_sizes.resize(total_syms);
			memcpy(&m_code_sizes[0], pCode_sizes, total_syms);

			const uint32_t huffman_fast_lookup_size = 1 << fast_lookup_bits;

			m_lookup.resize(0);
			m_lookup.resize(huffman_fast_lookup_size);

			m_tree.resize(0);
			m_tree.resize(total_syms * 2);

			uint32_t syms_using_codesize[basisu::cHuffmanMaxSupportedInternalCodeSize + 1];
			basisu::clear_obj(syms_using_codesize);
			for (uint32_t i = 0; i < total_syms; i++)
			{
				if (pCode_sizes[i] > basisu::cHuffmanMaxSupportedInternalCodeSize)
					return false;
				syms_using_codesize[pCode_sizes[i]]++;
			}

			uint32_t next_code[basisu::cHuffmanMaxSupportedInternalCodeSize + 1];
			next_code[0] = next_code[1] = 0;

			uint32_t used_syms = 0, total = 0;
			for (uint32_t i = 1; i < basisu::cHuffmanMaxSupportedInternalCodeSize; i++)
			{
				used_syms += syms_using_codesize[i];
				next_code[i + 1] = (total = ((total + syms_using_codesize[i]) << 1));
			}

			if (((1U << basisu::cHuffmanMaxSupportedInternalCodeSize) != total) && (used_syms != 1U))
				return false;

			for (int tree_next = -1, sym_index = 0; sym_index < (int)total_syms; ++sym_index)
			{
				uint32_t rev_code = 0, l, cur_code, code_size = pCode_sizes[sym_index];
				if (!code_size)
					continue;

				cur_code = next_code[code_size]++;

				for (l = code_size; l > 0; l--, cur_code >>= 1)
					rev_code = (rev_code << 1) | (cur_code & 1);

				if (code_size <= fast_lookup_bits)
				{
					uint32_t k = (code_size << 16) | sym_index;
					while (rev_code < huffman_fast_lookup_size)
					{
						if (m_lookup[rev_code] != 0)
						{
							// Supplied codesizes can't create a valid prefix code.
							return false;
						}

						m_lookup[rev_code] = k;
						rev_code += (1 << code_size);
					}
					continue;
				}

				int tree_cur;
				if (0 == (tree_cur = m_lookup[rev_code & (huffman_fast_lookup_size - 1)]))
				{
					const uint32_t idx = rev_code & (huffman_fast_lookup_size - 1);
					if (m_lookup[idx] != 0)
					{
						// Supplied codesizes can't create a valid prefix code.
						return false;
					}

					m_lookup[idx] = tree_next;
					tree_cur = tree_next;
					tree_next -= 2;
				}

				if (tree_cur >= 0)
				{
					// Supplied codesizes can't create a valid prefix code.
					return false;
				}

				rev_code >>= (fast_lookup_bits - 1);

				for (int j = code_size; j > ((int)fast_lookup_bits + 1); j--)
				{
					tree_cur -= ((rev_code >>= 1) & 1);

					const int idx = -tree_cur - 1;
					if (idx < 0)
						return false;
					else if (idx >= (int)m_tree.size())
						m_tree.resize(idx + 1);
										
					if (!m_tree[idx])
					{
						m_tree[idx] = (int16_t)tree_next;
						tree_cur = tree_next;
						tree_next -= 2;
					}
					else
					{
						tree_cur = m_tree[idx];
						if (tree_cur >= 0)
						{
							// Supplied codesizes can't create a valid prefix code.
							return false;
						}
					}
				}

				tree_cur -= ((rev_code >>= 1) & 1);

				const int idx = -tree_cur - 1;
				if (idx < 0)
					return false;
				else if (idx >= (int)m_tree.size())
					m_tree.resize(idx + 1);

				if (m_tree[idx] != 0)
				{
					// Supplied codesizes can't create a valid prefix code.
					return false;
				}

				m_tree[idx] = (int16_t)sym_index;
			}

			return true;
		}

		const basisu::uint8_vec &get_code_sizes() const { return m_code_sizes; }
		const basisu::int_vec &get_lookup() const { return m_lookup; }
		const basisu::int16_vec &get_tree() const { return m_tree; }

		bool is_valid() const { return m_code_sizes.size() > 0; }

	private:
		basisu::uint8_vec m_code_sizes;
		basisu::int_vec m_lookup;
		basisu::int16_vec m_tree;
	};

	class bitwise_decoder
	{
	public:
		bitwise_decoder() :
			m_buf_size(0),
			m_pBuf(nullptr),
			m_pBuf_start(nullptr),
			m_pBuf_end(nullptr),
			m_bit_buf(0),
			m_bit_buf_size(0)
		{
		}

		void clear()
		{
			m_buf_size = 0;
			m_pBuf = nullptr;
			m_pBuf_start = nullptr;
			m_pBuf_end = nullptr;
			m_bit_buf = 0;
			m_bit_buf_size = 0;
		}

		bool init(const uint8_t *pBuf, uint32_t buf_size)
		{
			if ((!pBuf) && (buf_size))
				return false;

			m_buf_size = buf_size;
			m_pBuf = pBuf;
			m_pBuf_start = pBuf;
			m_pBuf_end = pBuf + buf_size;
			m_bit_buf = 0;
			m_bit_buf_size = 0;
			return true;
		}

		void stop()
		{
		}
				
		inline uint32_t peek_bits(uint32_t num_bits)
		{
			if (!num_bits)
				return 0;

			assert(num_bits <= 25);

			while (m_bit_buf_size < num_bits)
			{
				uint32_t c = 0;
				if (m_pBuf < m_pBuf_end)
					c = *m_pBuf++;

				m_bit_buf |= (c << m_bit_buf_size);
				m_bit_buf_size += 8;
				assert(m_bit_buf_size <= 32);
			}

			return m_bit_buf & ((1 << num_bits) - 1);
		}

		void remove_bits(uint32_t num_bits)
		{
			assert(m_bit_buf_size >= num_bits);

			m_bit_buf >>= num_bits;
			m_bit_buf_size -= num_bits;
		}

		uint32_t get_bits(uint32_t num_bits)
		{
			if (num_bits > 25)
			{
				assert(num_bits <= 32);

				const uint32_t bits0 = peek_bits(25);
				m_bit_buf >>= 25;
				m_bit_buf_size -= 25;
				num_bits -= 25;

				const uint32_t bits = peek_bits(num_bits);
				m_bit_buf >>= num_bits;
				m_bit_buf_size -= num_bits;

				return bits0 | (bits << 25);
			}

			const uint32_t bits = peek_bits(num_bits);

			m_bit_buf >>= num_bits;
			m_bit_buf_size -= num_bits;

			return bits;
		}

		uint32_t decode_truncated_binary(uint32_t n)
		{
			assert(n >= 2);

			const uint32_t k = basisu::floor_log2i(n);
			const uint32_t u = (1 << (k + 1)) - n;

			uint32_t result = get_bits(k);

			if (result >= u)
				result = ((result << 1) | get_bits(1)) - u;

			return result;
		}

		uint32_t decode_rice(uint32_t m)
		{
			assert(m);

			uint32_t q = 0;
			for (;;)
			{
				uint32_t k = peek_bits(16);
				
				uint32_t l = 0;
				while (k & 1)
				{
					l++;
					k >>= 1;
				}
				
				q += l;

				remove_bits(l);

				if (l < 16)
					break;
			}

			return (q << m) + (get_bits(m + 1) >> 1);
		}

		inline uint32_t decode_vlc(uint32_t chunk_bits)
		{
			assert(chunk_bits);

			const uint32_t chunk_size = 1 << chunk_bits;
			const uint32_t chunk_mask = chunk_size - 1;
					
			uint32_t v = 0;
			uint32_t ofs = 0;

			for ( ; ; )
			{
				uint32_t s = get_bits(chunk_bits + 1);
				v |= ((s & chunk_mask) << ofs);
				ofs += chunk_bits;

				if ((s & chunk_size) == 0)
					break;
				
				if (ofs >= 32)
				{
					assert(0);
					break;
				}
			}

			return v;
		}

		inline uint32_t decode_huffman(const huffman_decoding_table &ct, int fast_lookup_bits = basisu::cHuffmanFastLookupBits)
		{
			assert(ct.m_code_sizes.size());

			const uint32_t huffman_fast_lookup_size = 1 << fast_lookup_bits;
						
			while (m_bit_buf_size < 16)
			{
				uint32_t c = 0;
				if (m_pBuf < m_pBuf_end)
					c = *m_pBuf++;

				m_bit_buf |= (c << m_bit_buf_size);
				m_bit_buf_size += 8;
				assert(m_bit_buf_size <= 32);
			}
						
			int code_len;

			int sym;
			if ((sym = ct.m_lookup[m_bit_buf & (huffman_fast_lookup_size - 1)]) >= 0)
			{
				code_len = sym >> 16;
				sym &= 0xFFFF;
			}
			else
			{
				code_len = fast_lookup_bits;
				do
				{
					sym = ct.m_tree[~sym + ((m_bit_buf >> code_len++) & 1)]; // ~sym = -sym - 1
				} while (sym < 0);
			}

			m_bit_buf >>= code_len;
			m_bit_buf_size -= code_len;

			return sym;
		}

		bool read_huffman_table(huffman_decoding_table &ct)
		{
			ct.clear();

			const uint32_t total_used_syms = get_bits(basisu::cHuffmanMaxSymsLog2);

			if (!total_used_syms)
				return true;
			if (total_used_syms > basisu::cHuffmanMaxSyms)
				return false;

			uint8_t code_length_code_sizes[basisu::cHuffmanTotalCodelengthCodes];
			basisu::clear_obj(code_length_code_sizes);

			const uint32_t num_codelength_codes = get_bits(5);
			if ((num_codelength_codes < 1) || (num_codelength_codes > basisu::cHuffmanTotalCodelengthCodes))
				return false;

			for (uint32_t i = 0; i < num_codelength_codes; i++)
				code_length_code_sizes[basisu::g_huffman_sorted_codelength_codes[i]] = static_cast<uint8_t>(get_bits(3));

			huffman_decoding_table code_length_table;
			if (!code_length_table.init(basisu::cHuffmanTotalCodelengthCodes, code_length_code_sizes))
				return false;

			if (!code_length_table.is_valid())
				return false;

			basisu::uint8_vec code_sizes(total_used_syms);

			uint32_t cur = 0;
			while (cur < total_used_syms)
			{
				int c = decode_huffman(code_length_table);

				if (c <= 16)
					code_sizes[cur++] = static_cast<uint8_t>(c);
				else if (c == basisu::cHuffmanSmallZeroRunCode)
					cur += get_bits(basisu::cHuffmanSmallZeroRunExtraBits) + basisu::cHuffmanSmallZeroRunSizeMin;
				else if (c == basisu::cHuffmanBigZeroRunCode)
					cur += get_bits(basisu::cHuffmanBigZeroRunExtraBits) + basisu::cHuffmanBigZeroRunSizeMin;
				else
				{
					if (!cur)
						return false;

					uint32_t l;
					if (c == basisu::cHuffmanSmallRepeatCode)
						l = get_bits(basisu::cHuffmanSmallRepeatExtraBits) + basisu::cHuffmanSmallRepeatSizeMin;
					else
						l = get_bits(basisu::cHuffmanBigRepeatExtraBits) + basisu::cHuffmanBigRepeatSizeMin;

					const uint8_t prev = code_sizes[cur - 1];
					if (prev == 0)
						return false;
					do
					{
						if (cur >= total_used_syms)
							return false;
						code_sizes[cur++] = prev;
					} while (--l > 0);
				}
			}

			if (cur != total_used_syms)
				return false;

			return ct.init(total_used_syms, &code_sizes[0]);
		}

		size_t get_bits_remaining() const
		{
			size_t total_bytes_remaining = m_pBuf_end - m_pBuf;
			return total_bytes_remaining * 8 + m_bit_buf_size;
		}

	private:
		uint32_t m_buf_size;
		const uint8_t *m_pBuf;
		const uint8_t *m_pBuf_start;
		const uint8_t *m_pBuf_end;

		uint32_t m_bit_buf;
		uint32_t m_bit_buf_size;
	};

	class simplified_bitwise_decoder
	{
	public:
		simplified_bitwise_decoder() :
			m_pBuf(nullptr),
			m_pBuf_end(nullptr),
			m_bit_buf(0)
		{
		}

		void clear()
		{
			m_pBuf = nullptr;
			m_pBuf_end = nullptr;
			m_bit_buf = 0;
		}

		bool init(const uint8_t* pBuf, size_t buf_size)
		{
			if ((!pBuf) && (buf_size))
				return false;

			m_pBuf = pBuf;
			m_pBuf_end = pBuf + buf_size;
			m_bit_buf = 1;
			return true;
		}

		bool init(const basisu::uint8_vec& buf)
		{
			return init(buf.data(), buf.size());
		}

		// num_bits must be 1, 2, 4 or 8 and codes cannot cross bytes
		inline uint32_t get_bits(uint32_t num_bits)
		{
			assert(m_pBuf);

			if (m_bit_buf <= 1)
				m_bit_buf = 256 | ((m_pBuf < m_pBuf_end) ? *m_pBuf++ : 0);

			const uint32_t mask = (1 << num_bits) - 1;
			const uint32_t res = m_bit_buf & mask;
			m_bit_buf >>= num_bits;
			assert(m_bit_buf >= 1);

			return res;
		}

		inline uint32_t get_bits1()
		{
			assert(m_pBuf);
			if (m_bit_buf <= 1)
				m_bit_buf = 256 | ((m_pBuf < m_pBuf_end) ? *m_pBuf++ : 0);
			const uint32_t res = m_bit_buf & 1;
			m_bit_buf >>= 1;
			assert(m_bit_buf >= 1);
			return res;
		}

		inline uint32_t get_bits2()
		{
			assert(m_pBuf);
			if (m_bit_buf <= 1)
				m_bit_buf = 256 | ((m_pBuf < m_pBuf_end) ? *m_pBuf++ : 0);
			const uint32_t res = m_bit_buf & 3;
			m_bit_buf >>= 2;
			assert(m_bit_buf >= 1);
			return res;
		}

		inline uint32_t get_bits4()
		{
			assert(m_pBuf);
			if (m_bit_buf <= 1)
				m_bit_buf = 256 | ((m_pBuf < m_pBuf_end) ? *m_pBuf++ : 0);
			const uint32_t res = m_bit_buf & 15;
			m_bit_buf >>= 4;
			assert(m_bit_buf >= 1);
			return res;
		}

		// No bitbuffer, can only ever retrieve bytes correctly.
		inline uint32_t get_bits8()
		{
			assert(m_pBuf);
			return (m_pBuf < m_pBuf_end) ? *m_pBuf++ : 0;
		}

		const uint8_t* m_pBuf;
		const uint8_t* m_pBuf_end;
		uint32_t m_bit_buf;
	};

	inline uint32_t basisd_rand(uint32_t seed)
	{
		if (!seed)
			seed++;
		uint32_t z = seed;
		BASISD_znew;
		return z;
	}

	// Returns random number in [0,limit). Max limit is 0xFFFF.
	inline uint32_t basisd_urand(uint32_t& seed, uint32_t limit)
	{
		seed = basisd_rand(seed);
		return (((seed ^ (seed >> 16)) & 0xFFFF) * limit) >> 16;
	}

	class approx_move_to_front
	{
	public:
		approx_move_to_front(uint32_t n)
		{
			init(n);
		}

		void init(uint32_t n)
		{
			m_values.resize(n);
			m_rover = n / 2;
		}

		const basisu::int_vec& get_values() const { return m_values; }
		basisu::int_vec& get_values() { return m_values; }

		uint32_t size() const { return (uint32_t)m_values.size(); }

		const int& operator[] (uint32_t index) const { return m_values[index]; }
		int operator[] (uint32_t index) { return m_values[index]; }

		void add(int new_value)
		{
			m_values[m_rover++] = new_value;
			if (m_rover == m_values.size())
				m_rover = (uint32_t)m_values.size() / 2;
		}

		void use(uint32_t index)
		{
			if (index)
			{
				//std::swap(m_values[index / 2], m_values[index]);
				int x = m_values[index / 2];
				int y = m_values[index];
				m_values[index / 2] = y;
				m_values[index] = x;
			}
		}

		// returns -1 if not found
		int find(int value) const
		{
			for (uint32_t i = 0; i < m_values.size(); i++)
				if (m_values[i] == value)
					return i;
			return -1;
		}

		void reset()
		{
			const uint32_t n = (uint32_t)m_values.size();

			m_values.clear();

			init(n);
		}

	private:
		basisu::int_vec m_values;
		uint32_t m_rover;
	};

	struct decoder_etc_block;
	
	inline uint8_t clamp255(int32_t i)
	{
		return (uint8_t)((i & 0xFFFFFF00U) ? (~(i >> 31)) : i);
	}

	enum eNoClamp
	{
		cNoClamp = 0
	};

	struct color32
	{
		union
		{
			struct
			{
				uint8_t r;
				uint8_t g;
				uint8_t b;
				uint8_t a;
			};

			uint8_t c[4];
			
			uint32_t m;
		};

		//color32() { }
		color32() = default;

		color32(uint32_t vr, uint32_t vg, uint32_t vb, uint32_t va) { set(vr, vg, vb, va); }
		color32(eNoClamp unused, uint32_t vr, uint32_t vg, uint32_t vb, uint32_t va) { (void)unused; set_noclamp_rgba(vr, vg, vb, va); }

		void set(uint32_t vr, uint32_t vg, uint32_t vb, uint32_t va) { c[0] = static_cast<uint8_t>(vr); c[1] = static_cast<uint8_t>(vg); c[2] = static_cast<uint8_t>(vb); c[3] = static_cast<uint8_t>(va); }

		void set_noclamp_rgb(uint32_t vr, uint32_t vg, uint32_t vb) { c[0] = static_cast<uint8_t>(vr); c[1] = static_cast<uint8_t>(vg); c[2] = static_cast<uint8_t>(vb); }
		void set_noclamp_rgba(uint32_t vr, uint32_t vg, uint32_t vb, uint32_t va) { set(vr, vg, vb, va); }

		void set_clamped(int vr, int vg, int vb, int va) { c[0] = clamp255(vr); c[1] = clamp255(vg);	c[2] = clamp255(vb); c[3] = clamp255(va); }

		uint8_t operator[] (uint32_t idx) const { assert(idx < 4); return c[idx]; }
		uint8_t &operator[] (uint32_t idx) { assert(idx < 4); return c[idx]; }

		bool operator== (const color32&rhs) const { return m == rhs.m; }

		static color32 comp_min(const color32& a, const color32& b) { return color32(cNoClamp, basisu::minimum(a[0], b[0]), basisu::minimum(a[1], b[1]), basisu::minimum(a[2], b[2]), basisu::minimum(a[3], b[3])); }
		static color32 comp_max(const color32& a, const color32& b) { return color32(cNoClamp, basisu::maximum(a[0], b[0]), basisu::maximum(a[1], b[1]), basisu::maximum(a[2], b[2]), basisu::maximum(a[3], b[3])); }
	};

	struct endpoint
	{
		color32 m_color5;
		uint8_t m_inten5;
		bool operator== (const endpoint& rhs) const
		{
			return (m_color5.r == rhs.m_color5.r) && (m_color5.g == rhs.m_color5.g) && (m_color5.b == rhs.m_color5.b) && (m_inten5 == rhs.m_inten5);
		}
		bool operator!= (const endpoint& rhs) const { return !(*this == rhs); }
	};

	// This duplicates key functionality in the encoder library's color_rgba class. Porting and retesting code that uses it to color32 is impractical.
	class color_rgba
	{
	public:
		union
		{
			uint8_t m_comps[4];

			struct
			{
				uint8_t r;
				uint8_t g;
				uint8_t b;
				uint8_t a;
			};
		};

		inline color_rgba()
		{
			static_assert(sizeof(*this) == 4, "sizeof(*this) != 4");
			static_assert(sizeof(*this) == sizeof(color32), "sizeof(*this) != sizeof(basist::color32)");
		}

		inline color_rgba(const color32& other) :
			r(other.r),
			g(other.g),
			b(other.b),
			a(other.a)
		{
		}

		color_rgba& operator= (const basist::color32& rhs)
		{
			r = rhs.r;
			g = rhs.g;
			b = rhs.b;
			a = rhs.a;
			return *this;
		}

		inline color_rgba(int y)
		{
			set(y);
		}

		inline color_rgba(int y, int na)
		{
			set(y, na);
		}

		inline color_rgba(int sr, int sg, int sb, int sa)
		{
			set(sr, sg, sb, sa);
		}

		inline color_rgba(eNoClamp, int sr, int sg, int sb, int sa)
		{
			set_noclamp_rgba((uint8_t)sr, (uint8_t)sg, (uint8_t)sb, (uint8_t)sa);
		}

		inline color_rgba& set_noclamp_y(int y)
		{
			m_comps[0] = (uint8_t)y;
			m_comps[1] = (uint8_t)y;
			m_comps[2] = (uint8_t)y;
			m_comps[3] = (uint8_t)255;
			return *this;
		}

		inline color_rgba& set_noclamp_rgba(int sr, int sg, int sb, int sa)
		{
			m_comps[0] = (uint8_t)sr;
			m_comps[1] = (uint8_t)sg;
			m_comps[2] = (uint8_t)sb;
			m_comps[3] = (uint8_t)sa;
			return *this;
		}

		inline color_rgba& set(int y)
		{
			m_comps[0] = static_cast<uint8_t>(basisu::clamp<int>(y, 0, 255));
			m_comps[1] = m_comps[0];
			m_comps[2] = m_comps[0];
			m_comps[3] = 255;
			return *this;
		}

		inline color_rgba& set(int y, int na)
		{
			m_comps[0] = static_cast<uint8_t>(basisu::clamp<int>(y, 0, 255));
			m_comps[1] = m_comps[0];
			m_comps[2] = m_comps[0];
			m_comps[3] = static_cast<uint8_t>(basisu::clamp<int>(na, 0, 255));
			return *this;
		}

		inline color_rgba& set(int sr, int sg, int sb, int sa)
		{
			m_comps[0] = static_cast<uint8_t>(basisu::clamp<int>(sr, 0, 255));
			m_comps[1] = static_cast<uint8_t>(basisu::clamp<int>(sg, 0, 255));
			m_comps[2] = static_cast<uint8_t>(basisu::clamp<int>(sb, 0, 255));
			m_comps[3] = static_cast<uint8_t>(basisu::clamp<int>(sa, 0, 255));
			return *this;
		}

		inline color_rgba& set_rgb(int sr, int sg, int sb)
		{
			m_comps[0] = static_cast<uint8_t>(basisu::clamp<int>(sr, 0, 255));
			m_comps[1] = static_cast<uint8_t>(basisu::clamp<int>(sg, 0, 255));
			m_comps[2] = static_cast<uint8_t>(basisu::clamp<int>(sb, 0, 255));
			return *this;
		}

		inline color_rgba& set_rgb(const color_rgba& other)
		{
			r = other.r;
			g = other.g;
			b = other.b;
			return *this;
		}

		inline const uint8_t& operator[] (uint32_t index) const { assert(index < 4); return m_comps[index]; }
		inline uint8_t& operator[] (uint32_t index) { assert(index < 4); return m_comps[index]; }

		inline void clear()
		{
			m_comps[0] = 0;
			m_comps[1] = 0;
			m_comps[2] = 0;
			m_comps[3] = 0;
		}

		inline bool operator== (const color_rgba& rhs) const
		{
			if (m_comps[0] != rhs.m_comps[0]) return false;
			if (m_comps[1] != rhs.m_comps[1]) return false;
			if (m_comps[2] != rhs.m_comps[2]) return false;
			if (m_comps[3] != rhs.m_comps[3]) return false;
			return true;
		}

		inline bool operator!= (const color_rgba& rhs) const
		{
			return !(*this == rhs);
		}

		inline bool operator<(const color_rgba& rhs) const
		{
			for (int i = 0; i < 4; i++)
			{
				if (m_comps[i] < rhs.m_comps[i])
					return true;
				else if (m_comps[i] != rhs.m_comps[i])
					return false;
			}
			return false;
		}

		inline color32 get_color32() const
		{
			return color32(r, g, b, a);
		}

		inline int get_709_luma() const { return (13938U * m_comps[0] + 46869U * m_comps[1] + 4729U * m_comps[2] + 32768U) >> 16U; }
	};

	struct selector
	{
		// Plain selectors (2-bits per value)
		uint8_t m_selectors[4];

		// ETC1 selectors
		uint8_t m_bytes[4];

		uint8_t m_lo_selector, m_hi_selector;
		uint8_t m_num_unique_selectors;
		bool operator== (const selector& rhs) const
		{
			return (m_selectors[0] == rhs.m_selectors[0]) &&
				(m_selectors[1] == rhs.m_selectors[1]) &&
				(m_selectors[2] == rhs.m_selectors[2]) &&
				(m_selectors[3] == rhs.m_selectors[3]);
		}
		bool operator!= (const selector& rhs) const
		{
			return !(*this == rhs);
		}

		void init_flags()
		{
			uint32_t hist[4] = { 0, 0, 0, 0 };
			for (uint32_t y = 0; y < 4; y++)
			{
				for (uint32_t x = 0; x < 4; x++)
				{
					uint32_t s = get_selector(x, y);
					hist[s]++;
				}
			}

			m_lo_selector = 3;
			m_hi_selector = 0;
			m_num_unique_selectors = 0;

			for (uint32_t i = 0; i < 4; i++)
			{
				if (hist[i])
				{
					m_num_unique_selectors++;
					if (i < m_lo_selector) m_lo_selector = static_cast<uint8_t>(i);
					if (i > m_hi_selector) m_hi_selector = static_cast<uint8_t>(i);
				}
			}
		}

		// Returned selector value ranges from 0-3 and is a direct index into g_etc1_inten_tables.
		inline uint32_t get_selector(uint32_t x, uint32_t y) const
		{
			assert((x < 4) && (y < 4));
			return (m_selectors[y] >> (x * 2)) & 3;
		}

		void set_selector(uint32_t x, uint32_t y, uint32_t val)
		{
			static const uint8_t s_selector_index_to_etc1[4] = { 3, 2, 0, 1 };

			assert((x | y | val) < 4);

			m_selectors[y] &= ~(3 << (x * 2));
			m_selectors[y] |= (val << (x * 2));

			const uint32_t etc1_bit_index = x * 4 + y;

			uint8_t *p = &m_bytes[3 - (etc1_bit_index >> 3)];

			const uint32_t byte_bit_ofs = etc1_bit_index & 7;
			const uint32_t mask = 1 << byte_bit_ofs;

			const uint32_t etc1_val = s_selector_index_to_etc1[val];

			const uint32_t lsb = etc1_val & 1;
			const uint32_t msb = etc1_val >> 1;

			p[0] &= ~mask;
			p[0] |= (lsb << byte_bit_ofs);

			p[-2] &= ~mask;
			p[-2] |= (msb << byte_bit_ofs);
		}
	};

	bool basis_block_format_is_uncompressed(block_format tex_type);

	//------------------------------------

	typedef uint16_t half_float;

	const double MIN_DENORM_HALF_FLOAT = 0.000000059604645; // smallest positive subnormal number
	const double MIN_HALF_FLOAT = 0.00006103515625; // smallest positive normal number
	const double MAX_HALF_FLOAT = 65504.0; // largest normal number
	const uint32_t MAX_HALF_FLOAT_AS_INT_BITS = 0x7BFF; // the half float rep for 65504.0

	inline uint32_t get_bits(uint32_t val, int low, int high)
	{
		const int num_bits = (high - low) + 1;
		assert((num_bits >= 1) && (num_bits <= 32));

		val >>= low;
		if (num_bits != 32)
			val &= ((1u << num_bits) - 1);

		return val;
	}

	inline bool is_half_inf_or_nan(half_float v)
	{
		return get_bits(v, 10, 14) == 31;
	}

	inline bool is_half_denorm(half_float v)
	{
		int e = (v >> 10) & 31;
		return !e;
	}

	inline int get_half_exp(half_float v)
	{
		int e = ((v >> 10) & 31);
		return e ? (e - 15) : -14;
	}

	inline int get_half_mantissa(half_float v)
	{
		if (is_half_denorm(v))
			return v & 0x3FF;
		return (v & 0x3FF) | 0x400;
	}

	inline float get_half_mantissaf(half_float v)
	{
		return ((float)get_half_mantissa(v)) / 1024.0f;
	}

	inline int get_half_sign(half_float v)
	{
		return v ? ((v & 0x8000) ? -1 : 1) : 0;
	}

	inline bool half_is_signed(half_float v)
	{
		return (v & 0x8000) != 0;
	}

#if 0
	int hexp = get_half_exp(Cf);
	float hman = get_half_mantissaf(Cf);
	int hsign = get_half_sign(Cf);
	float k = powf(2.0f, hexp) * hman * hsign;
	if (is_half_inf_or_nan(Cf))
		k = std::numeric_limits<float>::quiet_NaN();
#endif

	half_float float_to_half(float val);

	inline float half_to_float(half_float hval)
	{
		union { float f; uint32_t u; } x = { 0 };

		uint32_t s = ((uint32_t)hval >> 15) & 1;
		uint32_t e = ((uint32_t)hval >> 10) & 0x1F;
		uint32_t m = (uint32_t)hval & 0x3FF;

		if (!e)
		{
			if (!m)
			{
				// +- 0
				x.u = s << 31;
				return x.f;
			}
			else
			{
				// denormalized
				while (!(m & 0x00000400))
				{
					m <<= 1;
					--e;
				}

				++e;
				m &= ~0x00000400;
			}
		}
		else if (e == 31)
		{
			if (m == 0)
			{
				// +/- INF
				x.u = (s << 31) | 0x7f800000;
				return x.f;
			}
			else
			{
				// +/- NaN
				x.u = (s << 31) | 0x7f800000 | (m << 13);
				return x.f;
			}
		}

		e = e + (127 - 15);
		m = m << 13;

		assert(s <= 1);
		assert(m <= 0x7FFFFF);
		assert(e <= 255);

		x.u = m | (e << 23) | (s << 31);
		return x.f;
	}

	// Originally from bc6h_enc.h

	void bc6h_enc_init();

	const uint32_t MAX_BLOG16_VAL = 0xFFFF;

	// BC6H internals
	const uint32_t NUM_BC6H_MODES = 14;
	const uint32_t BC6H_LAST_MODE_INDEX = 13;
	const uint32_t BC6H_FIRST_1SUBSET_MODE_INDEX = 10; // in the MS docs, this is "mode 11" (where the first mode is 1), 60 bits for endpoints (10.10, 10.10, 10.10), 63 bits for weights
	const uint32_t TOTAL_BC6H_PARTITION_PATTERNS = 32;

	extern const uint8_t g_bc6h_mode_sig_bits[NUM_BC6H_MODES][4]; // base, r, g, b

	struct bc6h_bit_layout
	{
		int8_t m_comp; // R=0,G=1,B=2,D=3 (D=partition index)
		int8_t m_index; // 0-3, 0-1 Low/High subset 1, 2-3 Low/High subset 2, -1=partition index (d)
		int8_t m_last_bit;
		int8_t m_first_bit; // may be -1 if a single bit, may be >m_last_bit if reversed
	};

	const uint32_t MAX_BC6H_LAYOUT_INDEX = 25;
	extern const bc6h_bit_layout g_bc6h_bit_layouts[NUM_BC6H_MODES][MAX_BC6H_LAYOUT_INDEX];

	extern const uint8_t g_bc6h_2subset_patterns[TOTAL_BC6H_PARTITION_PATTERNS][4][4]; // [y][x]

	extern const uint8_t g_bc6h_weight3[8];
	extern const uint8_t g_bc6h_weight4[16];

	extern const int8_t g_bc6h_mode_lookup[32];
		
	// Converts b16 to half float
	inline half_float bc6h_blog16_to_half(uint32_t comp)
	{
		assert(comp <= 0xFFFF);

		// scale the magnitude by 31/64
		comp = (comp * 31u) >> 6u;
		return (half_float)comp;
	}

	const uint32_t MAX_BC6H_HALF_FLOAT_AS_UINT = 0x7BFF;

	// Inverts bc6h_blog16_to_half().
	// Returns the nearest blog16 given a half value. 
	inline uint32_t bc6h_half_to_blog16(half_float h)
	{
		assert(h <= MAX_BC6H_HALF_FLOAT_AS_UINT);
		return (h * 64 + 30) / 31;
	}

	// Suboptimal, but very close.
	inline uint32_t bc6h_half_to_blog(half_float h, uint32_t num_bits)
	{
		assert(h <= MAX_BC6H_HALF_FLOAT_AS_UINT);
		return (h * 64 + 30) / (31 * (1 << (16 - num_bits)));
	}

	struct bc6h_block
	{
		uint8_t m_bytes[16];
	};

	void bc6h_enc_block_mode10(bc6h_block* pPacked_block, const half_float pEndpoints[3][2], const uint8_t* pWeights);
	void bc6h_enc_block_1subset_4bit_weights(bc6h_block* pPacked_block, const half_float pEndpoints[3][2], const uint8_t* pWeights);
	void bc6h_enc_block_1subset_mode9_3bit_weights(bc6h_block* pPacked_block, const half_float pEndpoints[3][2], const uint8_t* pWeights);
	void bc6h_enc_block_1subset_3bit_weights(bc6h_block* pPacked_block, const half_float pEndpoints[3][2], const uint8_t* pWeights);
	void bc6h_enc_block_2subset_mode9_3bit_weights(bc6h_block* pPacked_block, uint32_t common_part_index, const half_float pEndpoints[2][3][2], const uint8_t* pWeights); // pEndpoints[subset][comp][lh_index]
	void bc6h_enc_block_2subset_3bit_weights(bc6h_block* pPacked_block, uint32_t common_part_index, const half_float pEndpoints[2][3][2], const uint8_t* pWeights); // pEndpoints[subset][comp][lh_index]
	bool bc6h_enc_block_solid_color(bc6h_block* pPacked_block, const half_float pColor[3]);

	struct bc6h_logical_block
	{
		uint32_t m_mode;
		uint32_t m_partition_pattern;	// must be 0 if 1 subset
		uint32_t m_endpoints[3][4];		// [comp][subset*2+lh_index] - must be already properly packed
		uint8_t m_weights[16];			// weights must be of the proper size, taking into account skipped MSB's which must be 0

		void clear()
		{
			basisu::clear_obj(*this);
		}
	};

	void pack_bc6h_block(bc6h_block& dst_blk, bc6h_logical_block& log_blk);
		
	namespace bc7_mode_5_encoder
	{
		void encode_bc7_mode_5_block(void* pDst_block, color32* pPixels, bool hq_mode);
	}

	namespace astc_6x6_hdr
	{
		extern uint8_t g_quantize_tables_preserve2[21 - 1][256]; // astc_helpers::TOTAL_ISE_RANGES=21
		extern uint8_t g_quantize_tables_preserve3[21 - 1][256];
	} // namespace astc_6x6_hdr

#if BASISD_SUPPORT_XUASTC
	namespace astc_ldr_t
	{
		const uint32_t ARITH_HEADER_MARKER = 0x01;
		const uint32_t ARITH_HEADER_MARKER_BITS = 5;

		const uint32_t FULL_ZSTD_HEADER_MARKER = 0x01;
		const uint32_t FULL_ZSTD_HEADER_MARKER_BITS = 5;

		const uint32_t FINAL_SYNC_MARKER = 0xAF;
		const uint32_t FINAL_SYNC_MARKER_BITS = 8;

		const uint32_t cMaxConfigReuseNeighbors = 3;

#pragma pack(push, 1)
		struct xuastc_ldr_arith_header
		{
			uint8_t m_flags;
			basisu::packed_uint<4> m_arith_bytes_len;
			basisu::packed_uint<4> m_mean0_bits_len;
			basisu::packed_uint<4> m_mean1_bytes_len;
			basisu::packed_uint<4> m_run_bytes_len;
			basisu::packed_uint<4> m_coeff_bytes_len;
			basisu::packed_uint<4> m_sign_bits_len;
			basisu::packed_uint<4> m_weight2_bits_len; // 2-bit weights (4 per byte), up to BISE_4_LEVELS
			basisu::packed_uint<4> m_weight3_bits_len; // 3-bit weights (2 per byte), up to BISE_8_LEVELS
			basisu::packed_uint<4> m_weight4_bits_len; // 4-bit weights (2 per byte), up to BISE_16_LEVELS
			basisu::packed_uint<4> m_weight8_bytes_len; // 8-bit weights (1 per byte), up to BISE_32_LEVELS
			basisu::packed_uint<4> m_unused;			// Future expansion
		};

		struct xuastc_ldr_full_zstd_header
		{
			uint8_t m_flags;

			// Control
			basisu::packed_uint<4> m_raw_bits_len; // uncompressed
			basisu::packed_uint<4> m_mode_bytes_len;
			basisu::packed_uint<4> m_solid_dpcm_bytes_len;
			
			// Endpoint DPCM
			basisu::packed_uint<4> m_endpoint_dpcm_reuse_indices_len;
			basisu::packed_uint<4> m_use_bc_bits_len;
			basisu::packed_uint<4> m_endpoint_dpcm_3bit_len;
			basisu::packed_uint<4> m_endpoint_dpcm_4bit_len;
			basisu::packed_uint<4> m_endpoint_dpcm_5bit_len;
			basisu::packed_uint<4> m_endpoint_dpcm_6bit_len;
			basisu::packed_uint<4> m_endpoint_dpcm_7bit_len;
			basisu::packed_uint<4> m_endpoint_dpcm_8bit_len;

			// Weight grid DCT
			basisu::packed_uint<4> m_mean0_bits_len;
			basisu::packed_uint<4> m_mean1_bytes_len;
			basisu::packed_uint<4> m_run_bytes_len;
			basisu::packed_uint<4> m_coeff_bytes_len;
			basisu::packed_uint<4> m_sign_bits_len;
			
			// Weight DPCM
			basisu::packed_uint<4> m_weight2_bits_len; // 2-bit weights (4 per byte), up to BISE_4_LEVELS
			basisu::packed_uint<4> m_weight3_bits_len; // 3-bit weights (4 per byte), up to BISE_8_LEVELS
			basisu::packed_uint<4> m_weight4_bits_len; // 4-bit weights (2 per byte), up to BISE_16_LEVELS
			basisu::packed_uint<4> m_weight8_bytes_len; // 8-bit weights (1 per byte), up to BISE_32_LEVELS

			basisu::packed_uint<4> m_unused;			// Future expansion
		};
#pragma pack(pop)

		const uint32_t DCT_RUN_LEN_EOB_SYM_INDEX = 64;
		const uint32_t DCT_MAX_ARITH_COEFF_MAG = 255;
		
		const uint32_t DCT_MEAN_LEVELS0 = 9, DCT_MEAN_LEVELS1 = 33;
		
		const uint32_t PART_HASH_BITS = 6u;
		const uint32_t PART_HASH_SIZE = 1u << PART_HASH_BITS;

		const uint32_t TM_HASH_BITS = 7u;
		const uint32_t TM_HASH_SIZE = 1u << TM_HASH_BITS;
				
		typedef basisu::vector<float> fvec;

		void init();

		color_rgba blue_contract_enc(color_rgba orig, bool& did_clamp, int encoded_b);
		color_rgba blue_contract_dec(int enc_r, int enc_g, int enc_b, int enc_a);
								
		struct astc_block_grid_config
		{
			uint16_t m_block_width, m_block_height;
			uint16_t m_grid_width, m_grid_height;

			astc_block_grid_config() {}

			astc_block_grid_config(uint32_t block_width, uint32_t block_height, uint32_t grid_width, uint32_t grid_height)
			{
				assert((block_width >= 4) && (block_width <= 12));
				assert((block_height >= 4) && (block_height <= 12));
				m_block_width = (uint16_t)block_width;
				m_block_height = (uint16_t)block_height;

				assert((grid_width >= 2) && (grid_width <= block_width));
				assert((grid_height >= 2) && (grid_height <= block_height));
				m_grid_width = (uint16_t)grid_width;
				m_grid_height = (uint16_t)grid_height;
			}

			bool operator==(const astc_block_grid_config& other) const
			{
				return (m_block_width == other.m_block_width) && (m_block_height == other.m_block_height) &&
					(m_grid_width == other.m_grid_width) && (m_grid_height == other.m_grid_height);
			}
		};

		struct astc_block_grid_data
		{
			float m_weight_gamma;

			// An unfortunate difference of containers, but in memory these matrices are both addressed as [r][c].
			basisu::vector2D<float> m_upsample_matrix;

			basisu::vector<float> m_downsample_matrix;

			astc_block_grid_data() {}
			astc_block_grid_data(float weight_gamma) : m_weight_gamma(weight_gamma) {}
		};

		typedef basisu::hash_map<astc_block_grid_config, astc_block_grid_data, bit_hasher<astc_block_grid_config> > astc_block_grid_data_hash_t;
						
		void decode_endpoints_ise20(uint32_t cem_index, const uint8_t* pEndpoint_vals, color32& l, color32& h);
		void decode_endpoints(uint32_t cem_index, const uint8_t* pEndpoint_vals, uint32_t endpoint_ise_index, color32& l, color32& h, float* pScale = nullptr);

		void decode_endpoints_ise20(uint32_t cem_index, const uint8_t* pEndpoint_vals, color_rgba& l, color_rgba& h);
		void decode_endpoints(uint32_t cem_index, const uint8_t* pEndpoint_vals, uint32_t endpoint_ise_index, color_rgba& l, color_rgba& h, float* pScale = nullptr);

		void compute_adjoint_downsample_matrix(basisu::vector<float>& downsample_matrix, uint32_t block_width, uint32_t block_height, uint32_t grid_width, uint32_t grid_height);
		void compute_upsample_matrix(basisu::vector2D<float>& upsample_matrix, uint32_t block_width, uint32_t block_height, uint32_t grid_width, uint32_t grid_height);

		class dct2f
		{
			enum { cMaxSize = 12 };

		public:
			dct2f() : m_rows(0u), m_cols(0u) {}

			// call with grid_height/grid_width (INVERTED)
			bool init(uint32_t rows, uint32_t cols);
			
			uint32_t rows() const { return m_rows; }
			uint32_t cols() const { return m_cols; }

			void forward(const float* pSrc, float* pDst, fvec& work) const;
			
			void inverse(const float* pSrc, float* pDst, fvec& work) const;
			
			// check variants use a less optimized implementation, used for sanity checking
			void inverse_check(const float* pSrc, float* pDst, fvec& work) const;
			
			void forward(const float* pSrc, uint32_t src_stride,
				float* pDst, uint32_t dst_stride, fvec& work) const;
			
			void inverse(const float* pSrc, uint32_t src_stride,
				float* pDst, uint32_t dst_stride, fvec& work) const;
			
			void inverse_check(const float* pSrc, uint32_t src_stride,
				float* pDst, uint32_t dst_stride, fvec& work) const;

		private:
			uint32_t m_rows, m_cols;
			fvec m_c_col;   // [u*m_rows + x]
			fvec m_c_row;   // [v*m_cols + y]
			fvec m_a_col;   // alpha(u)
			fvec m_a_row;   // alpha(v)
		};

		struct dct_syms
		{
			dct_syms()
			{
				clear();
			}

			void clear()
			{
				m_dc_sym = 0;
				m_num_dc_levels = 0;
				m_coeffs.resize(0);
				m_max_coeff_mag = 0;
				m_max_zigzag_index = 0;
			}

			uint32_t m_dc_sym;
			uint32_t m_num_dc_levels;

			struct coeff
			{
				uint16_t m_num_zeros;
				int16_t m_coeff; // or INT16_MAX if invalid

				coeff() {}
				coeff(uint16_t num_zeros, int16_t coeff) : m_num_zeros(num_zeros), m_coeff(coeff) {}
			};

			basisu::static_vector<coeff, 65> m_coeffs;

			uint32_t m_max_coeff_mag;
			uint32_t m_max_zigzag_index;
		};
				
		struct grid_dim_key
		{
			int m_grid_width;
			int m_grid_height;

			grid_dim_key() {}

			grid_dim_key(int w, int h) : m_grid_width(w), m_grid_height(h) {}

			bool operator== (const grid_dim_key& rhs) const
			{
				return (m_grid_width == rhs.m_grid_width) && (m_grid_height == rhs.m_grid_height);
			}
		};

		struct grid_dim_value
		{
			basisu::int_vec m_zigzag;
			dct2f m_dct;
		};

		typedef basisu::hash_map<grid_dim_key, grid_dim_value, bit_hasher<grid_dim_key> > grid_dim_hash_map;

		void init_astc_block_grid_data_hash();

		const astc_block_grid_data* find_astc_block_grid_data(uint32_t block_width, uint32_t block_height, uint32_t grid_width, uint32_t grid_height);

		const float DEADZONE_ALPHA = .5f;
		const float SCALED_WEIGHT_BASE_CODING_SCALE = .5f; // typically ~5 bits [0,32], or 3 [0,8]

		struct sample_quant_table_state
		{
			float m_q, m_sx, m_sy, m_level_scale;

			void init(float q,
				uint32_t block_width, uint32_t block_height,
				float level_scale)
			{
				m_q = q;
				m_level_scale = level_scale;

				const int Bx = block_width, By = block_height;

				m_sx = (float)8.0f / (float)Bx;
				m_sy = (float)8.0f / (float)By;
			}
		};

		class grid_weight_dct
		{
		public:
			grid_weight_dct() { }

			void init(uint32_t block_width, uint32_t block_height);
			
			static uint32_t get_num_weight_dc_levels(uint32_t weight_ise_range)
			{
				float scaled_weight_coding_scale = SCALED_WEIGHT_BASE_CODING_SCALE;
				if (weight_ise_range <= astc_helpers::BISE_8_LEVELS)
					scaled_weight_coding_scale = 1.0f / 8.0f;

				return (uint32_t)(64.0f * scaled_weight_coding_scale) + 1;
			}

			struct block_stats
			{
				float m_mean_weight;
				uint32_t m_total_coded_acs;
				uint32_t m_max_ac_coeff;
			};

			bool decode_block_weights(
				float q, uint32_t plane_index, // plane of weights to decode and IDCT from stream
				astc_helpers::log_astc_block& log_blk, // must be initialized except for the plane weights which are decoded
				basist::bitwise_decoder* pDec,
				const astc_block_grid_data* pGrid_data, // grid data for this grid size
				block_stats* pS,
				fvec& dct_work, // thread local
				const dct_syms* pSyms = nullptr) const;
						
			enum { m_zero_run = 3, m_coeff = 2 };

			uint32_t m_block_width, m_block_height;

			grid_dim_hash_map m_grid_dim_key_vals;
						
			// Adaptively compensate for weight level quantization noise being fed into the DCT. 
			// The more coursely the weight levels are quantized, the more noise injected, and the more noise will be spread between multiple AC coefficients.
			// This will cause some previously 0 coefficients to increase in mag, but they're likely noise. So carefully nudge the quant step size to compensate.
			static float scale_quant_steps(int Q_astc, float gamma = 0.1f /*.13f*/, float clamp_max = 2.0f)
			{
				assert(Q_astc >= 2);
				float factor = 63.0f / (Q_astc - 1);
				// TODO: Approximate powf()
				float scaled = powf(factor, gamma);
				scaled = basisu::clamp<float>(scaled, 1.0f, clamp_max);
				return scaled;
			}

			float compute_level_scale(float q, float span_len, float weight_gamma, uint32_t grid_width, uint32_t grid_height, uint32_t weight_ise_range) const;

			int sample_quant_table(sample_quant_table_state& state, uint32_t x, uint32_t y) const;

			void compute_quant_table(float q,
				uint32_t grid_width, uint32_t grid_height,
				float level_scale, int* dct_quant_tab) const;
			
			float get_max_span_len(const astc_helpers::log_astc_block& log_blk, uint32_t plane_index) const;
			
			inline int quantize_deadzone(float d, int L, float alpha, uint32_t x, uint32_t y) const
			{
				assert((x < m_block_width) && (y < m_block_height));

				if (((x == 1) && (y == 0)) ||
					((x == 0) && (y == 1)))
				{
					return (int)std::round(d / (float)L);
				}

				// L = quant step, alpha in [0,1.2] (typical 0.7–0.85)
				if (L <= 0) 
					return 0;

				float s = fabsf(d);
				float tau = alpha * float(L);                 // half-width of the zero band

				if (s <= tau) 
					return 0;                       // inside dead-zone towards zero

				// Quantize the residual outside the dead-zone with mid-tread rounding
				float qf = (s - tau) / float(L);
				int   q = (int)floorf(qf + 0.5f);            // ties-nearest
				return (d < 0.0f) ? -q : q;
			}

			inline float dequant_deadzone(int q, int L, float alpha, uint32_t x, uint32_t y) const
			{
				assert((x < m_block_width) && (y < m_block_height));

				if (((x == 1) && (y == 0)) ||
					((x == 0) && (y == 1)))
				{
					return (float)q * (float)L;
				}

				if (q == 0 || L <= 0) 
					return 0.0f;

				float tau = alpha * float(L);
				float mag = tau + float(abs(q)) * float(L);   // center of the (nonzero) bin
				return (q < 0) ? -mag : mag;
			}
		};

		struct trial_mode
		{
			uint32_t m_grid_width;
			uint32_t m_grid_height;
			uint32_t m_cem;
			int m_ccs_index;
			uint32_t m_endpoint_ise_range;
			uint32_t m_weight_ise_range;
			uint32_t m_num_parts;

			bool operator==(const trial_mode& other) const
			{
#define BU_COMP(a) if (a != other.a) return false;
				BU_COMP(m_grid_width);
				BU_COMP(m_grid_height);
				BU_COMP(m_cem);
				BU_COMP(m_ccs_index);
				BU_COMP(m_endpoint_ise_range);
				BU_COMP(m_weight_ise_range);
				BU_COMP(m_num_parts);
#undef BU_COMP
				return true;
			}

			bool operator<(const trial_mode& rhs) const
			{
#define BU_COMP(a) if (a < rhs.a) return true; else if (a > rhs.a) return false;
				BU_COMP(m_grid_width);
				BU_COMP(m_grid_height);
				BU_COMP(m_cem);
				BU_COMP(m_ccs_index);
				BU_COMP(m_endpoint_ise_range);
				BU_COMP(m_weight_ise_range);
				BU_COMP(m_num_parts);
#undef BU_COMP
				return false;
			}

			operator size_t() const
			{
				size_t h = 0xABC1F419;
#define BU_FIELD(a) do { h ^= hash_hsieh(reinterpret_cast<const uint8_t *>(&a), sizeof(a)); } while(0)
				BU_FIELD(m_grid_width);
				BU_FIELD(m_grid_height);
				BU_FIELD(m_cem);
				BU_FIELD(m_ccs_index);
				BU_FIELD(m_endpoint_ise_range);
				BU_FIELD(m_weight_ise_range);
				BU_FIELD(m_num_parts);
#undef BU_FIELD
				return h;
			}
		};

		// Organize trial modes for faster initial mode triaging.
		const uint32_t OTM_NUM_CEMS = 14; // 0-13 (13=highest valid LDR CEM)
		const uint32_t OTM_NUM_SUBSETS = 3; // 1-3
		const uint32_t OTM_NUM_CCS = 5; // -1 to 3
		const uint32_t OTM_NUM_GRID_SIZES = 2; // 0=small or 1=large (grid_w>=block_w-1 and grid_h>=block_h-1)
		const uint32_t OTM_NUM_GRID_ANISOS = 3; // 0=W=H, 1=W>H, 2=W<H

		inline uint32_t calc_grid_aniso_val(uint32_t gw, uint32_t gh, uint32_t bw, uint32_t bh)
		{
			assert((gw > 0) && (gh > 0));
			assert((bw > 0) && (bh > 0));
			assert((gw <= 12) && (gh <= 12) && (bw <= 12) && (bh <= 12));
			assert((gw <= bw) && (gh <= bh));
						
#if 0
			// Prev. code:
			uint32_t grid_aniso = 0;
			if (tm.m_grid_width != tm.m_grid_height) // not optimal for non-square block sizes
			{
				const float grid_x_fract = (float)tm.m_grid_width / (float)block_width;
				const float grid_y_fract = (float)tm.m_grid_height / (float)block_height;
				if (grid_x_fract >= grid_y_fract)
					grid_aniso = 1;
				else if (grid_x_fract < grid_y_fract)
					grid_aniso = 2;
			}
#endif
			// Compare gw/bw vs. gh/bh using integer math:
			// gw*bh >= gh*bw  -> X-dominant (1), else Y-dominant (2)
			const uint32_t lhs = gw * bh;
			const uint32_t rhs = gh * bw;

			// Equal (isotropic), X=Y
			if (lhs == rhs)
				return 0;

			// Anisotropic - 1=X, 2=Y
			return (lhs >= rhs) ? 1 : 2;
		}

		struct grouped_trial_modes
		{
			basisu::uint_vec m_tm_groups[OTM_NUM_CEMS][OTM_NUM_SUBSETS][OTM_NUM_CCS][OTM_NUM_GRID_SIZES][OTM_NUM_GRID_ANISOS]; // indices of encoder trial modes in each bucket

			void clear()
			{
				for (uint32_t cem_iter = 0; cem_iter < OTM_NUM_CEMS; cem_iter++)
					for (uint32_t subsets_iter = 0; subsets_iter < OTM_NUM_SUBSETS; subsets_iter++)
						for (uint32_t ccs_iter = 0; ccs_iter < OTM_NUM_CCS; ccs_iter++)
							for (uint32_t grid_sizes_iter = 0; grid_sizes_iter < OTM_NUM_GRID_SIZES; grid_sizes_iter++)
								for (uint32_t grid_anisos_iter = 0; grid_anisos_iter < OTM_NUM_GRID_ANISOS; grid_anisos_iter++)
									m_tm_groups[cem_iter][subsets_iter][ccs_iter][grid_sizes_iter][grid_anisos_iter].clear();
			}

			void add(uint32_t block_width, uint32_t block_height,
				const trial_mode& tm, uint32_t tm_index)
			{
				const uint32_t cem_index = tm.m_cem;
				assert(cem_index < OTM_NUM_CEMS);

				const uint32_t subset_index = tm.m_num_parts - 1;
				assert(subset_index < OTM_NUM_SUBSETS);

				const uint32_t ccs_index = tm.m_ccs_index + 1;
				assert(ccs_index < OTM_NUM_CCS);

				const uint32_t grid_size = (tm.m_grid_width >= (block_width - 1)) && (tm.m_grid_height >= (block_height - 1));
				const uint32_t grid_aniso = calc_grid_aniso_val(tm.m_grid_width, tm.m_grid_height, block_width, block_height);

				basisu::uint_vec& v = m_tm_groups[cem_index][subset_index][ccs_index][grid_size][grid_aniso];
				if (!v.capacity())
					v.reserve(64);

				v.push_back(tm_index);
			}

			uint32_t count_used_groups() const
			{
				uint32_t n = 0;

				for (uint32_t cem_iter = 0; cem_iter < OTM_NUM_CEMS; cem_iter++)
					for (uint32_t subsets_iter = 0; subsets_iter < OTM_NUM_SUBSETS; subsets_iter++)
						for (uint32_t ccs_iter = 0; ccs_iter < OTM_NUM_CCS; ccs_iter++)
							for (uint32_t grid_sizes_iter = 0; grid_sizes_iter < OTM_NUM_GRID_SIZES; grid_sizes_iter++)
								for (uint32_t grid_anisos_iter = 0; grid_anisos_iter < OTM_NUM_GRID_ANISOS; grid_anisos_iter++)
								{
									if (m_tm_groups[cem_iter][subsets_iter][ccs_iter][grid_sizes_iter][grid_anisos_iter].size())
										n++;
								}
				return n;
			}
		};

		extern grouped_trial_modes g_grouped_encoder_trial_modes[astc_helpers::cTOTAL_BLOCK_SIZES];

		inline const basisu::uint_vec& get_tm_candidates(const grouped_trial_modes& grouped_enc_trial_modes,
			uint32_t cem_index, uint32_t subset_index, uint32_t ccs_index, uint32_t grid_size, uint32_t grid_aniso)
		{
			assert(cem_index < OTM_NUM_CEMS);
			assert(subset_index < OTM_NUM_SUBSETS);
			assert(ccs_index < OTM_NUM_CCS);
			assert(grid_size < OTM_NUM_GRID_SIZES);
			assert(grid_aniso < OTM_NUM_GRID_ANISOS);

			const basisu::uint_vec& modes = grouped_enc_trial_modes.m_tm_groups[cem_index][subset_index][ccs_index][grid_size][grid_aniso];
			return modes;
		}

		const uint32_t CFG_PACK_GRID_BITS = 7;
		const uint32_t CFG_PACK_CEM_BITS = 3;
		const uint32_t CFG_PACK_CCS_BITS = 3;
		const uint32_t CFG_PACK_SUBSETS_BITS = 2;
		const uint32_t CFG_PACK_WISE_BITS = 4;
		const uint32_t CFG_PACK_EISE_BITS = 5;

		extern const int s_unique_ldr_index_to_astc_cem[6];

		enum class xuastc_mode
		{
			cMODE_SOLID = 0,
			cMODE_RAW = 1,

			// Full cfg, partition ID, and all endpoint value reuse.
			cMODE_REUSE_CFG_ENDPOINTS_LEFT = 2,
			cMODE_REUSE_CFG_ENDPOINTS_UP = 3,
			cMODE_REUSE_CFG_ENDPOINTS_DIAG = 4,

			cMODE_RUN = 5,

			cMODE_TOTAL,
		};

		enum class xuastc_zstd_mode
		{
			// len=1 bits
			cMODE_RAW = 0b0,

			// len=2 bits
			cMODE_RUN = 0b01,

			// len=4 bits
			cMODE_SOLID = 0b0011,
			cMODE_REUSE_CFG_ENDPOINTS_LEFT = 0b0111, 
			cMODE_REUSE_CFG_ENDPOINTS_UP = 0b1011,   
			cMODE_REUSE_CFG_ENDPOINTS_DIAG = 0b1111  
		};

		const uint32_t XUASTC_LDR_MODE_BYTE_IS_BASE_OFS_FLAG = 1 << 3;
		const uint32_t XUASTC_LDR_MODE_BYTE_PART_HASH_HIT = 1 << 4;
		const uint32_t XUASTC_LDR_MODE_BYTE_DPCM_ENDPOINTS_FLAG = 1 << 5;
		const uint32_t XUASTC_LDR_MODE_BYTE_TM_HASH_HIT_FLAG = 1 << 6;
		const uint32_t XUASTC_LDR_MODE_BYTE_USE_DCT = 1 << 7;

		enum class xuastc_ldr_syntax
		{
			cFullArith = 0,
			cHybridArithZStd = 1,
			cFullZStd = 2,

			cTotal
		};

		void create_encoder_trial_modes_table(uint32_t block_width, uint32_t block_height,
			basisu::vector<trial_mode>& encoder_trial_modes, grouped_trial_modes& grouped_encoder_trial_modes,
			bool print_debug_info, bool print_modes);

		extern basisu::vector<trial_mode> g_encoder_trial_modes[astc_helpers::cTOTAL_BLOCK_SIZES];
				
		inline uint32_t part_hash_index(uint32_t x)
		{
			// fib hash
			return (x * 2654435769u) & (PART_HASH_SIZE - 1);
		}

		// Full ZStd syntax only
		inline uint32_t tm_hash_index(uint32_t x)
		{
			// fib hash
			return (x * 2654435769u) & (TM_HASH_SIZE - 1);
		}

		// TODO: Some fields are unused during transcoding.
		struct prev_block_state
		{
			bool m_was_solid_color;
			bool m_used_weight_dct;
			bool m_first_endpoint_uses_bc;
			bool m_reused_full_cfg;
			bool m_used_part_hash;

			int m_tm_index; // -1 if invalid (solid color block)
			uint32_t m_base_cem_index; // doesn't include base+ofs
			uint32_t m_subset_index, m_ccs_index, m_grid_size, m_grid_aniso;

			prev_block_state()
			{
				clear();
			}

			void clear()
			{
				basisu::clear_obj(*this);
			}
		};

		struct prev_block_state_full_zstd
		{
			int m_tm_index; // -1 if invalid (solid color block)
			
			bool was_solid_color() const { return m_tm_index < 0; }

			prev_block_state_full_zstd()
			{
				clear();
			}

			void clear()
			{
				basisu::clear_obj(*this);
			}
		};

		inline uint32_t cem_to_ldrcem_index(uint32_t cem)
		{
			switch (cem)
			{
			case astc_helpers::CEM_LDR_LUM_DIRECT: return 0;
			case astc_helpers::CEM_LDR_LUM_ALPHA_DIRECT: return 1;
			case astc_helpers::CEM_LDR_RGB_BASE_SCALE: return 2;
			case astc_helpers::CEM_LDR_RGB_DIRECT: return 3;
			case astc_helpers::CEM_LDR_RGB_BASE_PLUS_OFFSET: return 4;
			case astc_helpers::CEM_LDR_RGB_BASE_SCALE_PLUS_TWO_A: return 5;
			case astc_helpers::CEM_LDR_RGBA_DIRECT: return 6;
			case astc_helpers::CEM_LDR_RGBA_BASE_PLUS_OFFSET: return 7;
			default:
				assert(0);
				break;
			}

			return 0;
		}
				
		bool pack_base_offset(
			uint32_t cem_index, uint32_t dst_ise_endpoint_range, uint8_t* pPacked_endpoints,
			const color_rgba& l, const color_rgba& h,
			bool use_blue_contraction, bool auto_disable_blue_contraction_if_clamped,
			bool& blue_contraction_clamped_flag, bool& base_ofs_clamped_flag, bool& endpoints_swapped);

		bool convert_endpoints_across_cems(
			uint32_t prev_cem, uint32_t prev_endpoint_ise_range, const uint8_t* pPrev_endpoints,
			uint32_t dst_cem, uint32_t dst_endpoint_ise_range, uint8_t* pDst_endpoints,
			bool always_repack,
			bool use_blue_contraction, bool auto_disable_blue_contraction_if_clamped,
			bool& blue_contraction_clamped_flag, bool& base_ofs_clamped_flag);

		uint32_t get_total_unique_patterns(uint32_t astc_block_size_index, uint32_t num_parts);
		//uint16_t unique_pat_index_to_part_seed(uint32_t astc_block_size_index, uint32_t num_parts, uint32_t unique_pat_index);

		typedef bool (*xuastc_decomp_image_init_callback_ptr)(uint32_t num_blocks_x, uint32_t num_blocks_y, uint32_t block_width, uint32_t block_height, bool srgb_decode_profile, float dct_q, bool has_alpha, void* pData);
		typedef bool (*xuastc_decomp_image_block_callback_ptr)(uint32_t bx, uint32_t by, const astc_helpers::log_astc_block& log_blk, void* pData);
				
		bool xuastc_ldr_decompress_image(
			const uint8_t* pComp_data, size_t comp_data_size,
			uint32_t& astc_block_width, uint32_t& astc_block_height,
			uint32_t& actual_width, uint32_t& actual_height, bool& has_alpha, bool& uses_srgb_astc_decode_mode,
			bool debug_output,
			xuastc_decomp_image_init_callback_ptr pInit_callback, void *pInit_callback_data,
			xuastc_decomp_image_block_callback_ptr pBlock_callback, void *pBlock_callback_data);
		
	} // namespace astc_ldr_t

	namespace arith_fastbits_f32
	{
		enum { TABLE_BITS = 8 };               // 256..1024 entries typical (8..10)
		enum { TABLE_SIZE = 1 << TABLE_BITS };
		enum { MANT_BITS = 23 };
		enum { FRAC_BITS = MANT_BITS - TABLE_BITS };
		enum { FRAC_MASK = (1u << FRAC_BITS) - 1u };

		extern bool g_initialized;
		extern float g_lut_edge[TABLE_SIZE + 1]; // samples at m = 1 + i/TABLE_SIZE (for linear)

		inline void init()
		{
			if (g_initialized)
				return;

			const float inv_ln2 = 1.4426950408889634f; // 1/ln(2)

			for (int i = 0; i <= TABLE_SIZE; ++i)
			{
				float m = 1.0f + float(i) / float(TABLE_SIZE);      // m in [1,2]
				g_lut_edge[i] = logf(m) * inv_ln2;             // log2(m)
			}

			g_initialized = true;
		}

		inline void unpack(float p, int& e_unbiased, uint32_t& mant)
		{
			// kill any denorms
			if (p < FLT_MIN)
				p = 0;

			union { float f; uint32_t u; } x;
			x.f = p;
			e_unbiased = int((x.u >> 23) & 0xFF) - 127;
			mant = (x.u & 0x7FFFFFu); // 23-bit mantissa
		}

		// Returns estimated bits given probability p, approximates -log2f(p).
		inline float bits_from_prob_linear(float p)
		{
			assert((p > 0.0f) && (p <= 1.0f));
			if (!g_initialized)
				init();

			int e; uint32_t mant;
			unpack(p, e, mant);

			uint32_t idx = mant >> FRAC_BITS;            // 0..TABLE_SIZE-1
			uint32_t frac = mant & FRAC_MASK;             // low FRAC_BITS
			const float inv_scale = 1.0f / float(1u << FRAC_BITS);
			float t = float(frac) * inv_scale;                 // [0,1)

			float y0 = g_lut_edge[idx];
			float y1 = g_lut_edge[idx + 1];
			float log2m = y0 + t * (y1 - y0);

			return -(float(e) + log2m);
		}

	} // namespace arith_fastbits_f32

	namespace arith
	{
		// A simple range coder
		const uint32_t ArithMaxSyms = 2048;
		const uint32_t DMLenShift = 15u;
		const uint32_t DMMaxCount = 1u << DMLenShift;
		const uint32_t BMLenShift = 13u;
		const uint32_t BMMaxCount = 1u << BMLenShift;
		const uint32_t ArithMinLen = 1u << 24u;
		const uint32_t ArithMaxLen = UINT32_MAX;
		const uint32_t ArithMinExpectedDataBufSize = 5;

		class arith_bit_model
		{
		public:
			arith_bit_model()
			{
				reset();
			}

			void init()
			{
				reset();
			}

			void reset()
			{
				m_bit0_count = 1;
				m_bit_count = 2;
				m_bit0_prob = 1U << (BMLenShift - 1);
				m_update_interval = 4;
				m_bits_until_update = 4;
			}

			float get_price(bool bit) const
			{
				const float prob_0 = (float)m_bit0_prob / (float)BMMaxCount;
				const float prob = bit ? (1.0f - prob_0) : prob_0;
				const float bits = arith_fastbits_f32::bits_from_prob_linear(prob);
				assert(fabs(bits - (-log2f(prob))) < .00125f); // basic sanity check
				return bits;
			}

			void update()
			{
				assert(m_bit_count >= 2);
				assert(m_bit0_count < m_bit_count);

				if (m_bit_count >= BMMaxCount)
				{
					assert(m_bit_count && m_bit0_count);

					m_bit_count = (m_bit_count + 1) >> 1;
					m_bit0_count = (m_bit0_count + 1) >> 1;

					if (m_bit0_count == m_bit_count)
						++m_bit_count;

					assert(m_bit0_count < m_bit_count);
				}

				const uint32_t scale = 0x80000000U / m_bit_count;
				m_bit0_prob = (m_bit0_count * scale) >> (31 - BMLenShift);

				m_update_interval = basisu::clamp<uint32_t>((5 * m_update_interval) >> 2, 4u, 128);

				m_bits_until_update = m_update_interval;
			}

			void print_prices(const char* pDesc)
			{
				if (pDesc)
					printf("arith_data_model bit prices for model %s:\n", pDesc);
				for (uint32_t i = 0; i < 2; i++)
					printf("%u: %3.3f bits\n", i, get_price(i));
				printf("\n");
			}

		private:
			friend class arith_enc;
			friend class arith_dec;

			uint32_t m_bit0_prob;	// snapshot made at last update

			uint32_t m_bit0_count;	// live
			uint32_t m_bit_count;	// live

			int m_bits_until_update;
			uint32_t m_update_interval;
		};

		enum { cARITH_GAMMA_MAX_TAIL_CTX = 4, cARITH_GAMMA_MAX_PREFIX_CTX = 3 };
		struct arith_gamma_contexts
		{
			arith_bit_model m_ctx_prefix[cARITH_GAMMA_MAX_PREFIX_CTX]; // for unary continue prefix
			arith_bit_model m_ctx_tail[cARITH_GAMMA_MAX_TAIL_CTX];     // for binary suffix bits
		};

		class arith_data_model
		{
		public:
			arith_data_model() :
				m_num_data_syms(0),
				m_total_sym_freq(0),
				m_update_interval(0),
				m_num_syms_until_next_update(0)
			{
			}

			arith_data_model(uint32_t num_syms, bool faster_update = false) :
				m_num_data_syms(0),
				m_total_sym_freq(0),
				m_update_interval(0),
				m_num_syms_until_next_update(0)
			{
				init(num_syms, faster_update);
			}

			void clear()
			{
				m_cum_sym_freqs.clear();
				m_sym_freqs.clear();

				m_num_data_syms = 0;
				m_total_sym_freq = 0;
				m_update_interval = 0;
				m_num_syms_until_next_update = 0;
			}

			void init(uint32_t num_syms, bool faster_update = false)
			{
				assert((num_syms >= 2) && (num_syms <= ArithMaxSyms));

				m_num_data_syms = num_syms;

				m_sym_freqs.resize(num_syms);
				m_cum_sym_freqs.resize(num_syms + 1);

				reset(faster_update);
			}

			void reset(bool faster_update = false)
			{
				if (!m_num_data_syms)
					return;

				m_sym_freqs.set_all(1);
				m_total_sym_freq = m_num_data_syms;

				m_update_interval = m_num_data_syms;
				m_num_syms_until_next_update = 0;

				update(false);

				if (faster_update)
				{
					m_update_interval = basisu::clamp<uint32_t>((m_num_data_syms + 7) / 8, 4u, (m_num_data_syms + 6) << 3);
					m_num_syms_until_next_update = m_update_interval;
				}
			}

			void update(bool enc_flag)
			{
				assert(m_num_data_syms);
				BASISU_NOTE_UNUSED(enc_flag);

				if (!m_num_data_syms)
					return;

				while (m_total_sym_freq >= DMMaxCount)
				{
					m_total_sym_freq = 0;

					for (uint32_t n = 0; n < m_num_data_syms; n++)
					{
						m_sym_freqs[n] = (m_sym_freqs[n] + 1u) >> 1u;
						m_total_sym_freq += m_sym_freqs[n];
					}
				}

				const uint32_t scale = 0x80000000U / m_total_sym_freq;

				uint32_t sum = 0;
				for (uint32_t i = 0; i < m_num_data_syms; ++i)
				{
					assert(((uint64_t)scale * sum) <= UINT32_MAX);
					m_cum_sym_freqs[i] = (scale * sum) >> (31 - DMLenShift);
					sum += m_sym_freqs[i];
				}
				assert(sum == m_total_sym_freq);

				m_cum_sym_freqs[m_num_data_syms] = DMMaxCount;

				m_update_interval = basisu::clamp<uint32_t>((5 * m_update_interval) >> 2, 4u, (m_num_data_syms + 6) << 3);

				m_num_syms_until_next_update = m_update_interval;
			}

			float get_price(uint32_t sym_index) const
			{
				assert(sym_index < m_num_data_syms);

				if (sym_index >= m_num_data_syms)
					return 0.0f;

				const float prob = (float)(m_cum_sym_freqs[sym_index + 1] - m_cum_sym_freqs[sym_index]) / (float)DMMaxCount;

				const float bits = arith_fastbits_f32::bits_from_prob_linear(prob);
				assert(fabs(bits - (-log2f(prob))) < .00125f); // basic sanity check
				return bits;
			}

			void print_prices(const char* pDesc)
			{
				if (pDesc)
					printf("arith_data_model bit prices for model %s:\n", pDesc);
				for (uint32_t i = 0; i < m_num_data_syms; i++)
					printf("%u: %3.3f bits\n", i, get_price(i));
				printf("\n");
			}

			uint32_t get_num_data_syms() const { return m_num_data_syms; }

		private:
			friend class arith_enc;
			friend class arith_dec;

			uint32_t m_num_data_syms;

			basisu::uint_vec m_sym_freqs;		// live histogram
			uint32_t m_total_sym_freq; // always live vs. m_sym_freqs

			basisu::uint_vec m_cum_sym_freqs; // has 1 extra entry, snapshot from last update

			uint32_t m_update_interval;
			int m_num_syms_until_next_update;

			uint32_t get_last_sym_index() const { return m_num_data_syms - 1; }
		};

		class arith_enc
		{
		public:
			arith_enc()
			{
				clear();
			}

			void clear()
			{
				m_data_buf.clear();

				m_base = 0;
				m_length = ArithMaxLen;
			}

			void init(size_t reserve_size)
			{
				m_data_buf.reserve(reserve_size);
				m_data_buf.resize(0);

				m_base = 0;
				m_length = ArithMaxLen;

				// Place 8-bit marker at beginning. 
				// This virtually always guarantees no backwards carries can be lost at the very beginning of the stream. (Should be impossible with this design.)
				// It always pushes out 1 0 byte at the very beginning to absorb future carries.
				// Caller does this now, we send a tiny header anyway
				//put_bits(0x1, 8);
				//assert(m_data_buf[0] != 0xFF);
			}

			void put_bit(uint32_t bit)
			{
				m_length >>= 1;

				if (bit)
				{
					const uint32_t orig_base = m_base;

					m_base += m_length;

					if (orig_base > m_base)
						prop_carry();
				}

				if (m_length < ArithMinLen)
					renorm();
			}

			enum { cMaxPutBitsLen = 20 };
			void put_bits(uint32_t val, uint32_t num_bits)
			{
				assert(num_bits && (num_bits <= cMaxPutBitsLen));
				assert(val < (1u << num_bits));

				m_length >>= num_bits;

				const uint32_t orig_base = m_base;

				m_base += val * m_length;

				if (orig_base > m_base)
					prop_carry();

				if (m_length < ArithMinLen)
					renorm();
			}

			// returns # of bits actually written
			inline uint32_t put_truncated_binary(uint32_t v, uint32_t n)
			{
				assert((n >= 2) && (v < n));

				uint32_t k = basisu::floor_log2i(n);
				uint32_t u = (1 << (k + 1)) - n;

				if (v < u)
				{
					put_bits(v, k);
					return k;
				}

				uint32_t x = v + u;
				assert((x >> 1) >= u);

				put_bits(x >> 1, k);
				put_bits(x & 1, 1);
				return k + 1;
			}

			static inline uint32_t get_truncated_binary_bits(uint32_t v, uint32_t n)
			{
				assert((n >= 2) && (v < n));

				uint32_t k = basisu::floor_log2i(n);
				uint32_t u = (1 << (k + 1)) - n;

				if (v < u)
					return k;

#ifdef _DEBUG
				uint32_t x = v + u;
				assert((x >> 1) >= u);
#endif

				return k + 1;
			}

			inline uint32_t put_rice(uint32_t v, uint32_t m)
			{
				assert(m);

				uint32_t q = v >> m, r = v & ((1 << m) - 1);

				// rice coding sanity check
				assert(q <= 64);

				uint32_t total_bits = q;

				// TODO: put_bits the pattern inverted in bit order
				while (q)
				{
					put_bit(1);
					q--;
				}

				put_bit(0);

				put_bits(r, m);

				total_bits += (m + 1);

				return total_bits;
			}

			static inline uint32_t get_rice_price(uint32_t v, uint32_t m)
			{
				assert(m);

				uint32_t q = v >> m;

				// rice coding sanity check
				assert(q <= 64);

				uint32_t total_bits = q + 1 + m;

				return total_bits;
			}

			inline void put_gamma(uint32_t n, arith_gamma_contexts& ctxs)
			{
				assert(n);
				if (!n)
					return;

				const int k = basisu::floor_log2i(n);
				if (k > 16)
				{
					assert(0);
					return;
				}

				// prefix: k times '1' then a '0'
				for (int i = 0; i < k; ++i)
					encode(1, ctxs.m_ctx_prefix[basisu::minimum<int>(i, cARITH_GAMMA_MAX_PREFIX_CTX - 1)]);

				encode(0, ctxs.m_ctx_prefix[basisu::minimum(k, cARITH_GAMMA_MAX_PREFIX_CTX - 1)]);

				// suffix: the k low bits of n
				for (int i = k - 1; i >= 0; --i)
				{
					uint32_t bit = (n >> i) & 1u;
					encode(bit, ctxs.m_ctx_tail[basisu::minimum<int>(i, cARITH_GAMMA_MAX_TAIL_CTX - 1)]);
				}
			}

			inline float put_gamma_and_return_price(uint32_t n, arith_gamma_contexts& ctxs)
			{
				assert(n);
				if (!n)
					return 0.0f;

				const int k = basisu::floor_log2i(n);
				if (k > 16)
				{
					assert(0);
					return 0.0f;
				}

				float total_price = 0.0f;

				// prefix: k times '1' then a '0'
				for (int i = 0; i < k; ++i)
				{
					total_price += ctxs.m_ctx_prefix[basisu::minimum<int>(i, cARITH_GAMMA_MAX_PREFIX_CTX - 1)].get_price(1);
					encode(1, ctxs.m_ctx_prefix[basisu::minimum<int>(i, cARITH_GAMMA_MAX_PREFIX_CTX - 1)]);
				}

				total_price += ctxs.m_ctx_prefix[basisu::minimum(k, cARITH_GAMMA_MAX_PREFIX_CTX - 1)].get_price(0);
				encode(0, ctxs.m_ctx_prefix[basisu::minimum(k, cARITH_GAMMA_MAX_PREFIX_CTX - 1)]);

				// suffix: the k low bits of n
				for (int i = k - 1; i >= 0; --i)
				{
					uint32_t bit = (n >> i) & 1u;
					total_price += ctxs.m_ctx_tail[basisu::minimum<int>(i, cARITH_GAMMA_MAX_TAIL_CTX - 1)].get_price(bit);
					encode(bit, ctxs.m_ctx_tail[basisu::minimum<int>(i, cARITH_GAMMA_MAX_TAIL_CTX - 1)]);
				}

				return total_price;
			}

			// prediced price, won't be accurate if a binary arith model decides to update in between
			inline float get_gamma_price(uint32_t n, const arith_gamma_contexts& ctxs)
			{
				assert(n);
				if (!n)
					return 0.0f;

				const int k = basisu::floor_log2i(n);
				if (k > 16)
				{
					assert(0);
					return 0.0f;
				}

				float total_price = 0.0f;

				// prefix: k times '1' then a '0'
				for (int i = 0; i < k; ++i)
					total_price += ctxs.m_ctx_prefix[basisu::minimum<int>(i, cARITH_GAMMA_MAX_PREFIX_CTX - 1)].get_price(1);

				total_price += ctxs.m_ctx_prefix[basisu::minimum(k, cARITH_GAMMA_MAX_PREFIX_CTX - 1)].get_price(0);

				// suffix: the k low bits of n
				for (int i = k - 1; i >= 0; --i)
				{
					uint32_t bit = (n >> i) & 1u;
					total_price += ctxs.m_ctx_tail[basisu::minimum<int>(i, cARITH_GAMMA_MAX_TAIL_CTX - 1)].get_price(bit);
				}

				return total_price;
			}

			void encode(uint32_t bit, arith_bit_model& dm)
			{
				uint32_t x = dm.m_bit0_prob * (m_length >> BMLenShift);

				if (!bit)
				{
					m_length = x;
					++dm.m_bit0_count;
				}
				else
				{
					const uint32_t orig_base = m_base;
					m_base += x;
					m_length -= x;

					if (orig_base > m_base)
						prop_carry();
				}
				++dm.m_bit_count;

				if (m_length < ArithMinLen)
					renorm();

				if (--dm.m_bits_until_update <= 0)
					dm.update();
			}

			float encode_and_return_price(uint32_t bit, arith_bit_model& dm)
			{
				const float price = dm.get_price(bit);
				encode(bit, dm);
				return price;
			}

			void encode(uint32_t sym, arith_data_model& dm)
			{
				assert(sym < dm.m_num_data_syms);

				const uint32_t orig_base = m_base;

				if (sym == dm.get_last_sym_index())
				{
					uint32_t x = dm.m_cum_sym_freqs[sym] * (m_length >> DMLenShift);
					m_base += x;
					m_length -= x;
				}
				else
				{
					m_length >>= DMLenShift;
					uint32_t x = dm.m_cum_sym_freqs[sym] * m_length;
					m_base += x;
					m_length = dm.m_cum_sym_freqs[sym + 1] * m_length - x;
				}

				if (orig_base > m_base)
					prop_carry();

				if (m_length < ArithMinLen)
					renorm();

				++dm.m_sym_freqs[sym];
				++dm.m_total_sym_freq;

				if (--dm.m_num_syms_until_next_update <= 0)
					dm.update(true);
			}

			float encode_and_return_price(uint32_t sym, arith_data_model& dm)
			{
				const float price = dm.get_price(sym);
				encode(sym, dm);
				return price;
			}

			void flush()
			{
				const uint32_t orig_base = m_base;

				if (m_length <= (2 * ArithMinLen))
				{
					m_base += ArithMinLen >> 1;
					m_length = ArithMinLen >> 9;
				}
				else
				{
					m_base += ArithMinLen;
					m_length = ArithMinLen >> 1;
				}

				if (orig_base > m_base)
					prop_carry();

				renorm();

				// Pad output to min 5 bytes - quite conservative; we're typically compressing large streams so the overhead shouldn't matter.
				if (m_data_buf.size() < ArithMinExpectedDataBufSize)
					m_data_buf.resize(ArithMinExpectedDataBufSize);
			}

			basisu::uint8_vec& get_data_buf() { return m_data_buf; }
			const basisu::uint8_vec& get_data_buf() const { return m_data_buf; }

		private:
			basisu::uint8_vec m_data_buf;
			uint32_t m_base, m_length;

			inline void prop_carry()
			{
				int64_t ofs = m_data_buf.size() - 1;

				for (; (ofs >= 0) && (m_data_buf[(size_t)ofs] == 0xFF); --ofs)
					m_data_buf[(size_t)ofs] = 0;

				if (ofs >= 0)
					++m_data_buf[(size_t)ofs];
			}

			inline void renorm()
			{
				assert(m_length < ArithMinLen);
				do
				{
					m_data_buf.push_back((uint8_t)(m_base >> 24u));
					m_base <<= 8u;
					m_length <<= 8u;
				} while (m_length < ArithMinLen);
			}
		};

		class arith_dec
		{
		public:
			arith_dec()
			{
				clear();
			}

			void clear()
			{
				m_pData_buf = nullptr;
				m_pData_buf_last_byte = nullptr;
				m_pData_buf_cur = nullptr;
				m_data_buf_size = 0;

				m_value = 0;
				m_length = 0;
			}

			bool init(const uint8_t* pBuf, size_t buf_size)
			{
				if (buf_size < ArithMinExpectedDataBufSize)
				{
					assert(0);
					return false;
				}

				m_pData_buf = pBuf;
				m_pData_buf_last_byte = pBuf + buf_size - 1;
				m_pData_buf_cur = m_pData_buf + 4;
				m_data_buf_size = buf_size;

				m_value = ((uint32_t)(pBuf[0]) << 24u) | ((uint32_t)(pBuf[1]) << 16u) | ((uint32_t)(pBuf[2]) << 8u) | (uint32_t)(pBuf[3]);
				m_length = ArithMaxLen;

				// Check for the 8-bit marker we always place at the beginning of the stream.
				//uint32_t marker = get_bits(8);
				//if (marker != 0x1)
				//	return false;

				return true;
			}

			uint32_t get_bit()
			{
				assert(m_data_buf_size);

				m_length >>= 1;

				uint32_t bit = (m_value >= m_length);

				if (bit)
					m_value -= m_length;

				if (m_length < ArithMinLen)
					renorm();

				return bit;
			}

			enum { cMaxGetBitsLen = 20 };

			uint32_t get_bits(uint32_t num_bits)
			{
				assert(m_data_buf_size);

				if ((num_bits < 1) || (num_bits > cMaxGetBitsLen))
				{
					assert(0);
					return 0;
				}

				m_length >>= num_bits;
				assert(m_length);

				const uint32_t v = m_value / m_length;

				m_value -= m_length * v;

				if (m_length < ArithMinLen)
					renorm();

				return v;
			}

			uint32_t decode_truncated_binary(uint32_t n)
			{
				assert(n >= 2);

				const uint32_t k = basisu::floor_log2i(n);
				const uint32_t u = (1 << (k + 1)) - n;

				uint32_t result = get_bits(k);

				if (result >= u)
					result = ((result << 1) | get_bits(1)) - u;

				return result;
			}

			uint32_t decode_rice(uint32_t m)
			{
				assert(m);

				uint32_t q = 0;
				for (;;)
				{
					uint32_t k = get_bit();
					if (!k)
						break;

					q++;
					if (q > 64)
					{
						assert(0);
						return 0;
					}
				}

				return (q << m) + get_bits(m);
			}

			uint32_t decode_bit(arith_bit_model& dm)
			{
				assert(m_data_buf_size);

				uint32_t x = dm.m_bit0_prob * (m_length >> BMLenShift);
				uint32_t bit = (m_value >= x);

				if (bit == 0)
				{
					m_length = x;
					++dm.m_bit0_count;
				}
				else
				{
					m_value -= x;
					m_length -= x;
				}
				++dm.m_bit_count;

				if (m_length < ArithMinLen)
					renorm();

				if (--dm.m_bits_until_update <= 0)
					dm.update();

				return bit;
			}

			inline uint32_t decode_gamma(arith_gamma_contexts& ctxs)
			{
				int k = 0;
				while (decode_bit(ctxs.m_ctx_prefix[basisu::minimum<int>(k, cARITH_GAMMA_MAX_PREFIX_CTX - 1)]))
				{
					++k;

					if (k > 16)
					{
						// something is very wrong
						assert(0);
						return 0;
					}
				}

				int n = 1 << k;
				for (int i = k - 1; i >= 0; --i)
				{
					uint32_t bit = decode_bit(ctxs.m_ctx_tail[basisu::minimum<int>(i, cARITH_GAMMA_MAX_TAIL_CTX - 1)]);
					n |= (bit << i);
				}

				return n;
			}

			uint32_t decode_sym(arith_data_model& dm)
			{
				assert(m_data_buf_size);
				assert(dm.m_num_data_syms);

				uint32_t x = 0, y = m_length;

				m_length >>= DMLenShift;

				uint32_t low_idx = 0, hi_idx = dm.m_num_data_syms;
				uint32_t mid_idx = hi_idx >> 1;

				do
				{
					uint32_t z = m_length * dm.m_cum_sym_freqs[mid_idx];

					if (z > m_value)
					{
						hi_idx = mid_idx;
						y = z;
					}
					else
					{
						low_idx = mid_idx;
						x = z;
					}
					mid_idx = (low_idx + hi_idx) >> 1;

				} while (mid_idx != low_idx);

				m_value -= x;
				m_length = y - x;

				if (m_length < ArithMinLen)
					renorm();

				++dm.m_sym_freqs[low_idx];
				++dm.m_total_sym_freq;

				if (--dm.m_num_syms_until_next_update <= 0)
					dm.update(false);

				return low_idx;
			}

		private:
			const uint8_t* m_pData_buf;
			const uint8_t* m_pData_buf_last_byte;
			const uint8_t* m_pData_buf_cur;
			size_t m_data_buf_size;

			uint32_t m_value, m_length;

			inline void renorm()
			{
				do
				{
					const uint32_t next_byte = (m_pData_buf_cur > m_pData_buf_last_byte) ? 0 : *m_pData_buf_cur++;

					m_value = (m_value << 8u) | next_byte;

				} while ((m_length <<= 8u) < ArithMinLen);
			}
		};

	} // namespace arith
#endif // BASISD_SUPPORT_XUASTC

#if BASISD_SUPPORT_XUASTC
	namespace bc7u
	{
		int determine_bc7_mode(const void* pBlock);
		int determine_bc7_mode_4_index_mode(const void* pBlock);
		int determine_bc7_mode_4_or_5_rotation(const void* pBlock);
		bool unpack_bc7_mode6(const void* pBlock_bits, color_rgba* pPixels);
		bool unpack_bc7(const void* pBlock, color_rgba* pPixels);
	} // namespace bc7u

	namespace bc7f 
	{
		enum
		{
			// Low-level BC7 encoder configuration flags.
			cPackBC7FlagUse2SubsetsRGB = 1, // use mode 1/3 for RGB blocks
			cPackBC7FlagUse2SubsetsRGBA = 2, // use mode 7 for RGBA blocks

			cPackBC7FlagUse3SubsetsRGB = 4, // also use mode 0/2, cPackBC7FlagUse2SubsetsRGB MUST be enabled too

			cPackBC7FlagUseDualPlaneRGB = 8, // enable mode 4/5 usage for RGB blocks
			cPackBC7FlagUseDualPlaneRGBA = 16, // enable mode 4/5 usage for RGBA blocks

			cPackBC7FlagPBitOpt = 32, // enable to disable usage of fixed p-bits on some modes; slower
			cPackBC7FlagPBitOptMode6 = 64, // enable to disable usage of fixed p-bits on mode 6, alpha on fully opaque blocks may be 254 however; slower

			cPackBC7FlagUseTrivialMode6 = 128, // enable trivial fast mode 6 encoder on blocks with very low variances (highly recommended)

			cPackBC7FlagPartiallyAnalyticalRGB = 256, // partially analytical mode for RGB blocks, slower but higher quality, computes actual SSE's on complex blocks to resolve which mode to use vs. predictions
			cPackBC7FlagPartiallyAnalyticalRGBA = 512, // partially analytical mode for RGBA blocks, slower but higher quality, computes actual SSE's on complex blocks to resolve which mode to use vs. predictions

			// Non-analytical is really still partially analytical on the mode pairs (0 vs. 2, 1 vs 3, 4 vs. 5).
			cPackBC7FlagNonAnalyticalRGB = 1024, // very slow/brute force, totally abuses the encoder, MUST use with cPackBC7FlagPartiallyAnalyticalRGB flag
			cPackBC7FlagNonAnalyticalRGBA = 2048, // very slow/brute force, totally abuses the encoder, MUST use with cPackBC7FlagPartiallyAnalyticalRGBA flag

			// Default to use first:

			// Decent analytical BC7 defaults
			cPackBC7FlagDefaultFastest = cPackBC7FlagUseTrivialMode6, // very weak particularly on alpha, mode 6 only for RGB/RGBA, 

			// Mode 6 with pbits for RGB, Modes 4,5,6 for alpha.
			cPackBC7FlagDefaultFaster = cPackBC7FlagPBitOpt | cPackBC7FlagUseDualPlaneRGBA | cPackBC7FlagUseTrivialMode6,

			cPackBC7FlagDefaultFast = cPackBC7FlagUse2SubsetsRGB | cPackBC7FlagUse2SubsetsRGBA | cPackBC7FlagUseDualPlaneRGBA |
				cPackBC7FlagPBitOpt | cPackBC7FlagUseTrivialMode6,

			cPackBC7FlagDefault = (cPackBC7FlagUse2SubsetsRGB | cPackBC7FlagUse2SubsetsRGBA | cPackBC7FlagUse3SubsetsRGB) |
				(cPackBC7FlagUseDualPlaneRGB | cPackBC7FlagUseDualPlaneRGBA) |
				(cPackBC7FlagPBitOpt | cPackBC7FlagPBitOptMode6) |
				cPackBC7FlagUseTrivialMode6,

			// Default partially analytical BC7 defaults (slower)
			cPackBC7FlagDefaultPartiallyAnalytical = cPackBC7FlagDefault | (cPackBC7FlagPartiallyAnalyticalRGB | cPackBC7FlagPartiallyAnalyticalRGBA),

			// Default non-analytical BC7 defaults (very slow). In reality the encoder is still analytical on the mode pairs, but at the highest level is non-analytical.
			cPackBC7FlagDefaultNonAnalytical = (cPackBC7FlagDefaultPartiallyAnalytical | (cPackBC7FlagNonAnalyticalRGB | cPackBC7FlagNonAnalyticalRGBA)) & ~cPackBC7FlagUseTrivialMode6
		};

		void init();
		
		void fast_pack_bc7_rgb_analytical(uint8_t* pBlock, const color_rgba* pPixels, uint32_t flags);
		uint32_t fast_pack_bc7_rgb_partial_analytical(uint8_t* pBlock, const color_rgba* pPixels, uint32_t flags);

		void fast_pack_bc7_rgba_analytical(uint8_t* pBlock, const color_rgba* pPixels, uint32_t flags);
		uint32_t fast_pack_bc7_rgba_partial_analytical(uint8_t* pBlock, const color_rgba* pPixels, uint32_t flags);
		
		uint32_t fast_pack_bc7_auto_rgba(uint8_t* pBlock, const color_rgba* pPixels, uint32_t flags);

		void print_perf_stats();

#if 0
		// Very basic BC7 mode 6 only to ASTC.
		void fast_pack_astc(void* pBlock, const color_rgba* pPixels);
#endif
		
		uint32_t calc_sse(const uint8_t* pBlock, const color_rgba* pPixels);

	} // namespace bc7f

	namespace etc1f
	{
		struct pack_etc1_state
		{
			uint64_t m_prev_solid_block;
			//decoder_etc_block m_prev_solid_block;

			int m_prev_solid_r8;
			int m_prev_solid_g8;
			int m_prev_solid_b8;

			pack_etc1_state()
			{
				clear();
			}

			void clear()
			{
				m_prev_solid_r8 = -1;
				m_prev_solid_g8 = -1;
				m_prev_solid_b8 = -1;
			}
		};

		void init();

		void pack_etc1_solid(uint8_t* pBlock, const color_rgba& color, pack_etc1_state& state, bool init_flag = false);

		void pack_etc1(uint8_t* pBlock, const color_rgba* pPixels, pack_etc1_state& state);
		
		void pack_etc1_grayscale(uint8_t* pBlock, const uint8_t* pPixels, pack_etc1_state& state);

	} // namespace etc1f
#endif // BASISD_SUPPORT_XUASTC

	// Private/internal XUASTC LDR transcoding helpers

	// XUASTC LDR formats only
	enum class transcoder_texture_format;
	block_format xuastc_get_block_format(transcoder_texture_format tex_fmt);

#if BASISD_SUPPORT_XUASTC
	// Low-quality, but fast, PVRTC1 RGB/RGBA encoder. Power of 2 texture dimensions required.
	// Note: Not yet part of our public API: this API may change! 
	void encode_pvrtc1(
		block_format fmt, void* pDst_blocks,
		const basisu::vector2D<color32>& temp_image,
		uint32_t dst_num_blocks_x, uint32_t dst_num_blocks_y, bool from_alpha);
		
	void transcode_4x4_block(
		block_format fmt, // desired output block format
		uint32_t block_x, uint32_t block_y, // 4x4 block being processed
		void* pDst_blocks, // base pointer to output buffer/bitmap
		uint8_t* pDst_block_u8, // pointer to output block/or first pixel to write
		const color32* block_pixels, // pointer to 4x4 (16) 32bpp RGBA pixels
		uint32_t output_block_or_pixel_stride_in_bytes, uint32_t output_row_pitch_in_blocks_or_pixels, uint32_t output_rows_in_pixels, // output buffer dimensions
		int channel0, int channel1, // channels to process, used by some block formats
		bool high_quality, bool from_alpha,	// Flags specific to certain block formats
		uint32_t bc7f_flags, // Real-time bc7f BC7 encoder flags, see bc7f::cPackBC7FlagDefault etc.
		etc1f::pack_etc1_state& etc1_pack_state,  // etc1f thread local state
		int has_alpha = -1); // has_alpha = -1 unknown, 0=definitely no (a all 255's), 1=potentially yes
#endif // BASISD_SUPPORT_XUASTC

	struct bc7_mode_5
	{
		union
		{
			struct
			{
				uint64_t m_mode : 6;
				uint64_t m_rot : 2;

				uint64_t m_r0 : 7;
				uint64_t m_r1 : 7;
				uint64_t m_g0 : 7;
				uint64_t m_g1 : 7;
				uint64_t m_b0 : 7;
				uint64_t m_b1 : 7;
				uint64_t m_a0 : 8;
				uint64_t m_a1_0 : 6;

			} m_lo;

			uint64_t m_lo_bits;
		};

		union
		{
			struct
			{
				uint64_t m_a1_1 : 2;

				// bit 2
				uint64_t m_c00 : 1;
				uint64_t m_c10 : 2;
				uint64_t m_c20 : 2;
				uint64_t m_c30 : 2;

				uint64_t m_c01 : 2;
				uint64_t m_c11 : 2;
				uint64_t m_c21 : 2;
				uint64_t m_c31 : 2;

				uint64_t m_c02 : 2;
				uint64_t m_c12 : 2;
				uint64_t m_c22 : 2;
				uint64_t m_c32 : 2;

				uint64_t m_c03 : 2;
				uint64_t m_c13 : 2;
				uint64_t m_c23 : 2;
				uint64_t m_c33 : 2;

				// bit 33
				uint64_t m_a00 : 1;
				uint64_t m_a10 : 2;
				uint64_t m_a20 : 2;
				uint64_t m_a30 : 2;

				uint64_t m_a01 : 2;
				uint64_t m_a11 : 2;
				uint64_t m_a21 : 2;
				uint64_t m_a31 : 2;

				uint64_t m_a02 : 2;
				uint64_t m_a12 : 2;
				uint64_t m_a22 : 2;
				uint64_t m_a32 : 2;

				uint64_t m_a03 : 2;
				uint64_t m_a13 : 2;
				uint64_t m_a23 : 2;
				uint64_t m_a33 : 2;

			} m_hi;

			uint64_t m_hi_bits;
		};
	};

} // namespace basist



