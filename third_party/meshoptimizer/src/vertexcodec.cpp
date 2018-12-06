// This file is part of meshoptimizer library; see meshoptimizer.h for version/license details
#include "meshoptimizer.h"

#include <assert.h>
#include <string.h>

#if defined(__ARM_NEON__) || defined(__ARM_NEON)
#define SIMD_NEON
#endif

#if defined(__AVX__) || defined(__SSSE3__)
#define SIMD_SSE
#endif

#if !defined(SIMD_SSE) && defined(_MSC_VER) && (defined(_M_IX86) || defined(_M_X64))
#define SIMD_SSE
#define SIMD_FALLBACK
#include <intrin.h>
#endif

#ifdef SIMD_SSE
#include <tmmintrin.h>
#endif

#ifdef SIMD_NEON
#include <arm_neon.h>
#endif

namespace meshopt
{

const unsigned char kVertexHeader = 0xa0;

const size_t kVertexBlockSizeBytes = 8192;
const size_t kVertexBlockMaxSize = 256;
const size_t kByteGroupSize = 16;
const size_t kTailMaxSize = 32;

static size_t getVertexBlockSize(size_t vertex_size)
{
	// make sure the entire block fits into the scratch buffer
	size_t result = kVertexBlockSizeBytes / vertex_size;

	// align to byte group size; we encode each byte as a byte group
	// if vertex block is misaligned, it results in wasted bytes, so just truncate the block size
	result &= ~(kByteGroupSize - 1);

	return (result < kVertexBlockMaxSize) ? result : kVertexBlockMaxSize;
}

inline unsigned char zigzag8(unsigned char v)
{
	return ((signed char)(v) >> 7) ^ (v << 1);
}

inline unsigned char unzigzag8(unsigned char v)
{
	return -(v & 1) ^ (v >> 1);
}

static bool encodeBytesGroupZero(const unsigned char* buffer)
{
	for (size_t i = 0; i < kByteGroupSize; ++i)
		if (buffer[i])
			return false;

	return true;
}

static size_t encodeBytesGroupMeasure(const unsigned char* buffer, int bits)
{
	assert(bits >= 1 && bits <= 8);

	if (bits == 1)
		return encodeBytesGroupZero(buffer) ? 0 : size_t(-1);

	if (bits == 8)
		return kByteGroupSize;

	size_t result = kByteGroupSize * bits / 8;

	unsigned char sentinel = (1 << bits) - 1;

	for (size_t i = 0; i < kByteGroupSize; ++i)
		result += buffer[i] >= sentinel;

	return result;
}

static unsigned char* encodeBytesGroup(unsigned char* data, const unsigned char* buffer, int bits)
{
	assert(bits >= 1 && bits <= 8);

	if (bits == 1)
		return data;

	if (bits == 8)
	{
		memcpy(data, buffer, kByteGroupSize);
		return data + kByteGroupSize;
	}

	size_t byte_size = 8 / bits;
	assert(kByteGroupSize % byte_size == 0);

	// fixed portion: bits bits for each value
	// variable portion: full byte for each out-of-range value (using 1...1 as sentinel)
	unsigned char sentinel = (1 << bits) - 1;

	for (size_t i = 0; i < kByteGroupSize; i += byte_size)
	{
		unsigned char byte = 0;

		for (size_t k = 0; k < byte_size; ++k)
		{
			unsigned char enc = (buffer[i + k] >= sentinel) ? sentinel : buffer[i + k];

			byte <<= bits;
			byte |= enc;
		}

		*data++ = byte;
	}

	for (size_t i = 0; i < kByteGroupSize; ++i)
	{
		if (buffer[i] >= sentinel)
		{
			*data++ = buffer[i];
		}
	}

	return data;
}

static unsigned char* encodeBytes(unsigned char* data, unsigned char* data_end, const unsigned char* buffer, size_t buffer_size)
{
	assert(buffer_size % kByteGroupSize == 0);

	unsigned char* header = data;

	// round number of groups to 4 to get number of header bytes
	size_t header_size = (buffer_size / kByteGroupSize + 3) / 4;

	if (size_t(data_end - data) < header_size)
		return 0;

	data += header_size;

	memset(header, 0, header_size);

	for (size_t i = 0; i < buffer_size; i += kByteGroupSize)
	{
		if (size_t(data_end - data) < kTailMaxSize)
			return 0;

		int best_bits = 8;
		size_t best_size = encodeBytesGroupMeasure(buffer + i, 8);

		for (int bits = 1; bits < 8; bits *= 2)
		{
			size_t size = encodeBytesGroupMeasure(buffer + i, bits);

			if (size < best_size)
			{
				best_bits = bits;
				best_size = size;
			}
		}

		int bitslog2 = (best_bits == 1) ? 0 : (best_bits == 2) ? 1 : (best_bits == 4) ? 2 : 3;
		assert((1 << bitslog2) == best_bits);

		size_t header_offset = i / kByteGroupSize;

		header[header_offset / 4] |= bitslog2 << ((header_offset % 4) * 2);

		unsigned char* next = encodeBytesGroup(data, buffer + i, best_bits);

		assert(data + best_size == next);
		data = next;
	}

	return data;
}

static unsigned char* encodeVertexBlock(unsigned char* data, unsigned char* data_end, const unsigned char* vertex_data, size_t vertex_count, size_t vertex_size, unsigned char last_vertex[256])
{
	assert(vertex_count > 0 && vertex_count <= kVertexBlockMaxSize);

	unsigned char buffer[kVertexBlockMaxSize];
	assert(sizeof(buffer) % kByteGroupSize == 0);

	// we sometimes encode elements we didn't fill when rounding to kByteGroupSize
	memset(buffer, 0, sizeof(buffer));

	for (size_t k = 0; k < vertex_size; ++k)
	{
		size_t vertex_offset = k;

		unsigned char p = last_vertex[k];

		for (size_t i = 0; i < vertex_count; ++i)
		{
			buffer[i] = zigzag8(vertex_data[vertex_offset] - p);

			p = vertex_data[vertex_offset];

			vertex_offset += vertex_size;
		}

		data = encodeBytes(data, data_end, buffer, (vertex_count + kByteGroupSize - 1) & ~(kByteGroupSize - 1));
		if (!data)
			return 0;
	}

	memcpy(last_vertex, &vertex_data[vertex_size * (vertex_count - 1)], vertex_size);

	return data;
}

#if defined(SIMD_FALLBACK) || (!defined(SIMD_SSE) && !defined(SIMD_NEON))
static const unsigned char* decodeBytesGroup(const unsigned char* data, unsigned char* buffer, int bitslog2)
{
#define READ() byte = *data++
#define NEXT(bits) enc = byte >> (8 - bits), byte <<= bits, encv = *data_var, *buffer++ = (enc == (1 << bits) - 1) ? encv : enc, data_var += (enc == (1 << bits) - 1)

	unsigned char byte, enc, encv;
	const unsigned char* data_var;

	switch (bitslog2)
	{
	case 0:
		memset(buffer, 0, kByteGroupSize);
		return data;
	case 1:
		data_var = data + 4;

		// 4 groups with 4 2-bit values in each byte
		READ(), NEXT(2), NEXT(2), NEXT(2), NEXT(2);
		READ(), NEXT(2), NEXT(2), NEXT(2), NEXT(2);
		READ(), NEXT(2), NEXT(2), NEXT(2), NEXT(2);
		READ(), NEXT(2), NEXT(2), NEXT(2), NEXT(2);

		return data_var;
	case 2:
		data_var = data + 8;

		// 8 groups with 2 4-bit values in each byte
		READ(), NEXT(4), NEXT(4);
		READ(), NEXT(4), NEXT(4);
		READ(), NEXT(4), NEXT(4);
		READ(), NEXT(4), NEXT(4);
		READ(), NEXT(4), NEXT(4);
		READ(), NEXT(4), NEXT(4);
		READ(), NEXT(4), NEXT(4);
		READ(), NEXT(4), NEXT(4);

		return data_var;
	case 3:
		memcpy(buffer, data, kByteGroupSize);
		return data + kByteGroupSize;
	default:
		assert(!"Unexpected bit length"); // This can never happen since bitslog2 is a 2-bit value
		return data;
	}

#undef READ
#undef NEXT
}

static const unsigned char* decodeBytes(const unsigned char* data, const unsigned char* data_end, unsigned char* buffer, size_t buffer_size)
{
	assert(buffer_size % kByteGroupSize == 0);

	const unsigned char* header = data;

	// round number of groups to 4 to get number of header bytes
	size_t header_size = (buffer_size / kByteGroupSize + 3) / 4;

	if (size_t(data_end - data) < header_size)
		return 0;

	data += header_size;

	for (size_t i = 0; i < buffer_size; i += kByteGroupSize)
	{
		if (size_t(data_end - data) < kTailMaxSize)
			return 0;

		size_t header_offset = i / kByteGroupSize;

		int bitslog2 = (header[header_offset / 4] >> ((header_offset % 4) * 2)) & 3;

		data = decodeBytesGroup(data, buffer + i, bitslog2);
	}

	return data;
}

static const unsigned char* decodeVertexBlock(const unsigned char* data, const unsigned char* data_end, unsigned char* vertex_data, size_t vertex_count, size_t vertex_size, unsigned char last_vertex[256])
{
	assert(vertex_count > 0 && vertex_count <= kVertexBlockMaxSize);

	unsigned char buffer[kVertexBlockMaxSize];
	unsigned char transposed[kVertexBlockSizeBytes];

	size_t vertex_count_aligned = (vertex_count + kByteGroupSize - 1) & ~(kByteGroupSize - 1);

	for (size_t k = 0; k < vertex_size; ++k)
	{
		data = decodeBytes(data, data_end, buffer, vertex_count_aligned);
		if (!data)
			return 0;

		size_t vertex_offset = k;

		unsigned char p = last_vertex[k];

		for (size_t i = 0; i < vertex_count; ++i)
		{
			unsigned char v = unzigzag8(buffer[i]) + p;

			transposed[vertex_offset] = v;
			p = v;

			vertex_offset += vertex_size;
		}
	}

	memcpy(vertex_data, transposed, vertex_count * vertex_size);

	memcpy(last_vertex, &transposed[vertex_size * (vertex_count - 1)], vertex_size);

	return data;
}
#endif

#if defined(SIMD_SSE) || defined(SIMD_NEON)
static unsigned char kDecodeBytesGroupShuffle[256][8];
static unsigned char kDecodeBytesGroupCount[256];

static bool decodeBytesGroupBuildTables()
{
	for (int mask = 0; mask < 256; ++mask)
	{
		unsigned char shuffle[8];
		unsigned char count = 0;

		for (int i = 0; i < 8; ++i)
		{
			int maski = (mask >> i) & 1;
			shuffle[i] = maski ? count : 0x80;
			count += (unsigned char)(maski);
		}

		memcpy(kDecodeBytesGroupShuffle[mask], shuffle, 8);
		kDecodeBytesGroupCount[mask] = count;
	}

	return true;
}

static bool gDecodeBytesGroupInitialized = decodeBytesGroupBuildTables();
#endif

#ifdef SIMD_SSE
static __m128i decodeShuffleMask(unsigned char mask0, unsigned char mask1)
{
	__m128i sm0 = _mm_loadl_epi64(reinterpret_cast<const __m128i*>(&kDecodeBytesGroupShuffle[mask0]));
	__m128i sm1 = _mm_loadl_epi64(reinterpret_cast<const __m128i*>(&kDecodeBytesGroupShuffle[mask1]));
	__m128i sm1off = _mm_set1_epi8(kDecodeBytesGroupCount[mask0]);

	__m128i sm1r = _mm_add_epi8(sm1, sm1off);

	return _mm_unpacklo_epi64(sm0, sm1r);
}

static void transpose8(__m128i& x0, __m128i& x1, __m128i& x2, __m128i& x3)
{
	__m128i t0 = _mm_unpacklo_epi8(x0, x1);
	__m128i t1 = _mm_unpackhi_epi8(x0, x1);
	__m128i t2 = _mm_unpacklo_epi8(x2, x3);
	__m128i t3 = _mm_unpackhi_epi8(x2, x3);

	x0 = _mm_unpacklo_epi16(t0, t2);
	x1 = _mm_unpackhi_epi16(t0, t2);
	x2 = _mm_unpacklo_epi16(t1, t3);
	x3 = _mm_unpackhi_epi16(t1, t3);
}

static __m128i unzigzag8(__m128i v)
{
	__m128i xl = _mm_sub_epi8(_mm_setzero_si128(), _mm_and_si128(v, _mm_set1_epi8(1)));
	__m128i xr = _mm_and_si128(_mm_srli_epi16(v, 1), _mm_set1_epi8(127));

	return _mm_xor_si128(xl, xr);
}

static const unsigned char* decodeBytesGroupSimd(const unsigned char* data, unsigned char* buffer, int bitslog2)
{
	switch (bitslog2)
	{
	case 0:
	{
		__m128i result = _mm_setzero_si128();

		_mm_storeu_si128(reinterpret_cast<__m128i*>(buffer), result);

		return data;
	}

	case 1:
	{
#ifdef __GNUC__
		typedef int __attribute__((aligned(1))) unaligned_int;
#else
		typedef int unaligned_int;
#endif

		__m128i sel2 = _mm_cvtsi32_si128(*reinterpret_cast<const unaligned_int*>(data));
		__m128i rest = _mm_loadu_si128(reinterpret_cast<const __m128i*>(data + 4));

		__m128i sel22 = _mm_unpacklo_epi8(_mm_srli_epi16(sel2, 4), sel2);
		__m128i sel2222 = _mm_unpacklo_epi8(_mm_srli_epi16(sel22, 2), sel22);
		__m128i sel = _mm_and_si128(sel2222, _mm_set1_epi8(3));

		__m128i mask = _mm_cmpeq_epi8(sel, _mm_set1_epi8(3));
		int mask16 = _mm_movemask_epi8(mask);
		unsigned char mask0 = (unsigned char)(mask16 & 255);
		unsigned char mask1 = (unsigned char)(mask16 >> 8);

		__m128i shuf = decodeShuffleMask(mask0, mask1);

		__m128i result = _mm_or_si128(_mm_shuffle_epi8(rest, shuf), _mm_andnot_si128(mask, sel));

		_mm_storeu_si128(reinterpret_cast<__m128i*>(buffer), result);

		return data + 4 + kDecodeBytesGroupCount[mask0] + kDecodeBytesGroupCount[mask1];
	}

	case 2:
	{
		__m128i sel4 = _mm_loadl_epi64(reinterpret_cast<const __m128i*>(data));
		__m128i rest = _mm_loadu_si128(reinterpret_cast<const __m128i*>(data + 8));

		__m128i sel44 = _mm_unpacklo_epi8(_mm_srli_epi16(sel4, 4), sel4);
		__m128i sel = _mm_and_si128(sel44, _mm_set1_epi8(15));

		__m128i mask = _mm_cmpeq_epi8(sel, _mm_set1_epi8(15));
		int mask16 = _mm_movemask_epi8(mask);
		unsigned char mask0 = (unsigned char)(mask16 & 255);
		unsigned char mask1 = (unsigned char)(mask16 >> 8);

		__m128i shuf = decodeShuffleMask(mask0, mask1);

		__m128i result = _mm_or_si128(_mm_shuffle_epi8(rest, shuf), _mm_andnot_si128(mask, sel));

		_mm_storeu_si128(reinterpret_cast<__m128i*>(buffer), result);

		return data + 8 + kDecodeBytesGroupCount[mask0] + kDecodeBytesGroupCount[mask1];
	}

	case 3:
	{
		__m128i rest = _mm_loadu_si128(reinterpret_cast<const __m128i*>(data));

		__m128i result = rest;

		_mm_storeu_si128(reinterpret_cast<__m128i*>(buffer), result);

		return data + 16;
	}

	default:
		assert(!"Unexpected bit length"); // This can never happen since bitslog2 is a 2-bit value
		return data;
	}
}
#endif

#ifdef SIMD_NEON
static uint8x16_t shuffleBytes(unsigned char mask0, unsigned char mask1, uint8x8_t rest0, uint8x8_t rest1)
{
	uint8x8_t sm0 = vld1_u8(kDecodeBytesGroupShuffle[mask0]);
	uint8x8_t sm1 = vld1_u8(kDecodeBytesGroupShuffle[mask1]);

	uint8x8_t r0 = vtbl1_u8(rest0, sm0);
	uint8x8_t r1 = vtbl1_u8(rest1, sm1);

	return vcombine_u8(r0, r1);
}

static int neonMoveMask(uint8x16_t mask)
{
	static const uint8x16_t byte_mask = {1, 2, 4, 8, 16, 32, 64, 128, 1, 2, 4, 8, 16, 32, 64, 128};

	uint8x16_t masked = vandq_u8(mask, byte_mask);

	// we need horizontal sums of each half of masked
	// the endianness of the platform determines the order of high/low in sum1 below
	// we assume little endian here
	uint8x8_t sum1 = vpadd_u8(vget_low_u8(masked), vget_high_u8(masked));
	uint8x8_t sum2 = vpadd_u8(sum1, sum1);
	uint8x8_t sum3 = vpadd_u8(sum2, sum2);

	// note: here we treat 2b sum as a 16bit mask; this depends on endianness, see above
	return vget_lane_u16(vreinterpret_u16_u8(sum3), 0);
}

static void transpose8(uint8x16_t& x0, uint8x16_t& x1, uint8x16_t& x2, uint8x16_t& x3)
{
	uint8x16x2_t t01 = vzipq_u8(x0, x1);
	uint8x16x2_t t23 = vzipq_u8(x2, x3);

	uint16x8x2_t x01 = vzipq_u16(vreinterpretq_u16_u8(t01.val[0]), vreinterpretq_u16_u8(t23.val[0]));
	uint16x8x2_t x23 = vzipq_u16(vreinterpretq_u16_u8(t01.val[1]), vreinterpretq_u16_u8(t23.val[1]));

	x0 = vreinterpretq_u8_u16(x01.val[0]);
	x1 = vreinterpretq_u8_u16(x01.val[1]);
	x2 = vreinterpretq_u8_u16(x23.val[0]);
	x3 = vreinterpretq_u8_u16(x23.val[1]);
}

static uint8x16_t unzigzag8(uint8x16_t v)
{
	uint8x16_t xl = vreinterpretq_u8_s8(vnegq_s8(vreinterpretq_s8_u8(vandq_u8(v, vdupq_n_u8(1)))));
	uint8x16_t xr = vshrq_n_u8(v, 1);

	return veorq_u8(xl, xr);
}

static const unsigned char* decodeBytesGroupSimd(const unsigned char* data, unsigned char* buffer, int bitslog2)
{
	switch (bitslog2)
	{
	case 0:
	{
		uint8x16_t result = vdupq_n_u8(0);

		vst1q_u8(buffer, result);

		return data;
	}

	case 1:
	{
		uint8x8_t sel2 = vld1_u8(data);
		uint8x8_t sel22 = vzip_u8(vshr_n_u8(sel2, 4), sel2).val[0];
		uint8x8x2_t sel2222 = vzip_u8(vshr_n_u8(sel22, 2), sel22);
		uint8x16_t sel = vandq_u8(vcombine_u8(sel2222.val[0], sel2222.val[1]), vdupq_n_u8(3));

		uint8x16_t mask = vceqq_u8(sel, vdupq_n_u8(3));
		int mask16 = neonMoveMask(mask);
		unsigned char mask0 = (unsigned char)(mask16 & 255);
		unsigned char mask1 = (unsigned char)(mask16 >> 8);

		uint8x8_t rest0 = vld1_u8(data + 4);
		uint8x8_t rest1 = vld1_u8(data + 4 + kDecodeBytesGroupCount[mask0]);

		uint8x16_t result = vbslq_u8(mask, shuffleBytes(mask0, mask1, rest0, rest1), sel);

		vst1q_u8(buffer, result);

		return data + 4 + kDecodeBytesGroupCount[mask0] + kDecodeBytesGroupCount[mask1];
	}

	case 2:
	{
		uint8x8_t sel4 = vld1_u8(data);
		uint8x8x2_t sel44 = vzip_u8(vshr_n_u8(sel4, 4), vand_u8(sel4, vdup_n_u8(15)));
		uint8x16_t sel = vcombine_u8(sel44.val[0], sel44.val[1]);

		uint8x16_t mask = vceqq_u8(sel, vdupq_n_u8(15));
		int mask16 = neonMoveMask(mask);
		unsigned char mask0 = (unsigned char)(mask16 & 255);
		unsigned char mask1 = (unsigned char)(mask16 >> 8);

		uint8x8_t rest0 = vld1_u8(data + 8);
		uint8x8_t rest1 = vld1_u8(data + 8 + kDecodeBytesGroupCount[mask0]);

		uint8x16_t result = vbslq_u8(mask, shuffleBytes(mask0, mask1, rest0, rest1), sel);

		vst1q_u8(buffer, result);

		return data + 8 + kDecodeBytesGroupCount[mask0] + kDecodeBytesGroupCount[mask1];
	}

	case 3:
	{
		uint8x16_t rest = vld1q_u8(data);

		uint8x16_t result = rest;

		vst1q_u8(buffer, result);

		return data + 16;
	}

	default:
		assert(!"Unexpected bit length"); // This can never happen since bitslog2 is a 2-bit value
		return data;
	}
}
#endif

#if defined(SIMD_SSE) || defined(SIMD_NEON)
static const unsigned char* decodeBytesSimd(const unsigned char* data, const unsigned char* data_end, unsigned char* buffer, size_t buffer_size)
{
	assert(buffer_size % kByteGroupSize == 0);
	assert(kByteGroupSize == 16);

	const unsigned char* header = data;

	// round number of groups to 4 to get number of header bytes
	size_t header_size = (buffer_size / kByteGroupSize + 3) / 4;

	if (size_t(data_end - data) < header_size)
		return 0;

	data += header_size;

	size_t i = 0;

	// fast-path: process 4 groups at a time, do a shared bounds check - each group reads <=32b
	for (; i + kByteGroupSize * 4 <= buffer_size && size_t(data_end - data) >= kTailMaxSize * 4; i += kByteGroupSize * 4)
	{
		size_t header_offset = i / kByteGroupSize;
		unsigned char header_byte = header[header_offset / 4];

		data = decodeBytesGroupSimd(data, buffer + i + kByteGroupSize * 0, (header_byte >> 0) & 3);
		data = decodeBytesGroupSimd(data, buffer + i + kByteGroupSize * 1, (header_byte >> 2) & 3);
		data = decodeBytesGroupSimd(data, buffer + i + kByteGroupSize * 2, (header_byte >> 4) & 3);
		data = decodeBytesGroupSimd(data, buffer + i + kByteGroupSize * 3, (header_byte >> 6) & 3);
	}

	// slow-path: process remaining groups
	for (; i < buffer_size; i += kByteGroupSize)
	{
		if (size_t(data_end - data) < kTailMaxSize)
			return 0;

		size_t header_offset = i / kByteGroupSize;

		int bitslog2 = (header[header_offset / 4] >> ((header_offset % 4) * 2)) & 3;

		data = decodeBytesGroupSimd(data, buffer + i, bitslog2);
	}

	return data;
}

static const unsigned char* decodeVertexBlockSimd(const unsigned char* data, const unsigned char* data_end, unsigned char* vertex_data, size_t vertex_count, size_t vertex_size, unsigned char last_vertex[256])
{
	assert(vertex_count > 0 && vertex_count <= kVertexBlockMaxSize);

	unsigned char buffer[kVertexBlockMaxSize * 4];
	unsigned char transposed[kVertexBlockSizeBytes];

	size_t vertex_count_aligned = (vertex_count + kByteGroupSize - 1) & ~(kByteGroupSize - 1);

	for (size_t k = 0; k < vertex_size; k += 4)
	{
		for (size_t j = 0; j < 4; ++j)
		{
			data = decodeBytesSimd(data, data_end, buffer + j * vertex_count_aligned, vertex_count_aligned);
			if (!data)
				return 0;
		}

#ifdef SIMD_SSE
#define TEMP __m128i
#define LOAD(i) __m128i r##i = _mm_loadu_si128(reinterpret_cast<const __m128i*>(buffer + j + i * vertex_count_aligned))
#define GRP4(i) t0 = _mm_shuffle_epi32(r##i, 0), t1 = _mm_shuffle_epi32(r##i, 1), t2 = _mm_shuffle_epi32(r##i, 2), t3 = _mm_shuffle_epi32(r##i, 3)
#define FIXD(i) t##i = pi = _mm_add_epi8(pi, t##i)
#define SAVE(i) *reinterpret_cast<int*>(savep) = _mm_cvtsi128_si32(t##i), savep += vertex_size
#endif

#ifdef SIMD_NEON
#define TEMP uint8x8_t
#define LOAD(i) uint8x16_t r##i = vld1q_u8(buffer + j + i * vertex_count_aligned)
#define GRP4(i) t0 = vget_low_u8(r##i), t1 = vreinterpret_u8_u32(vdup_lane_u32(vreinterpret_u32_u8(t0), 1)), t2 = vget_high_u8(r##i), t3 = vreinterpret_u8_u32(vdup_lane_u32(vreinterpret_u32_u8(t2), 1))
#define FIXD(i) t##i = pi = vadd_u8(pi, t##i)
#define SAVE(i) vst1_lane_u32(reinterpret_cast<uint32_t*>(savep), vreinterpret_u32_u8(t##i), 0), savep += vertex_size
#endif

#ifdef SIMD_SSE
		__m128i pi = _mm_cvtsi32_si128(*reinterpret_cast<const int*>(last_vertex + k));
#endif
#ifdef SIMD_NEON
		uint8x8_t pi = vreinterpret_u8_u32(vld1_lane_u32(reinterpret_cast<uint32_t*>(last_vertex + k), vdup_n_u32(0), 0));
#endif

		unsigned char* savep = transposed + k;

		for (size_t j = 0; j < vertex_count_aligned; j += 16)
		{
			LOAD(0);
			LOAD(1);
			LOAD(2);
			LOAD(3);

			r0 = unzigzag8(r0);
			r1 = unzigzag8(r1);
			r2 = unzigzag8(r2);
			r3 = unzigzag8(r3);

			transpose8(r0, r1, r2, r3);

			TEMP t0, t1, t2, t3;

			GRP4(0);
			FIXD(0), FIXD(1), FIXD(2), FIXD(3);
			SAVE(0), SAVE(1), SAVE(2), SAVE(3);

			GRP4(1);
			FIXD(0), FIXD(1), FIXD(2), FIXD(3);
			SAVE(0), SAVE(1), SAVE(2), SAVE(3);

			GRP4(2);
			FIXD(0), FIXD(1), FIXD(2), FIXD(3);
			SAVE(0), SAVE(1), SAVE(2), SAVE(3);

			GRP4(3);
			FIXD(0), FIXD(1), FIXD(2), FIXD(3);
			SAVE(0), SAVE(1), SAVE(2), SAVE(3);

#undef TEMP
#undef LOAD
#undef GRP4
#undef FIXD
#undef SAVE
		}
	}

	memcpy(vertex_data, transposed, vertex_count * vertex_size);

	memcpy(last_vertex, &transposed[vertex_size * (vertex_count - 1)], vertex_size);

	return data;
}
#endif

} // namespace meshopt

size_t meshopt_encodeVertexBuffer(unsigned char* buffer, size_t buffer_size, const void* vertices, size_t vertex_count, size_t vertex_size)
{
	using namespace meshopt;

	assert(vertex_size > 0 && vertex_size <= 256);
	assert(vertex_size % 4 == 0);

	const unsigned char* vertex_data = static_cast<const unsigned char*>(vertices);

	unsigned char* data = buffer;
	unsigned char* data_end = buffer + buffer_size;

	if (size_t(data_end - data) < 1 + vertex_size)
		return 0;

	*data++ = kVertexHeader;

	unsigned char last_vertex[256] = {};
	if (vertex_count > 0)
		memcpy(last_vertex, vertex_data, vertex_size);

	size_t vertex_block_size = getVertexBlockSize(vertex_size);

	size_t vertex_offset = 0;

	while (vertex_offset < vertex_count)
	{
		size_t block_size = (vertex_offset + vertex_block_size < vertex_count) ? vertex_block_size : vertex_count - vertex_offset;

		data = encodeVertexBlock(data, data_end, vertex_data + vertex_offset * vertex_size, block_size, vertex_size, last_vertex);
		if (!data)
			return 0;

		vertex_offset += block_size;
	}

	size_t tail_size = vertex_size < kTailMaxSize ? kTailMaxSize : vertex_size;

	if (size_t(data_end - data) < tail_size)
		return 0;

	// write first vertex to the end of the stream and pad it to 32 bytes; this is important to simplify bounds checks in decoder
	if (vertex_size < kTailMaxSize)
	{
		memset(data, 0, kTailMaxSize - vertex_size);
		data += kTailMaxSize - vertex_size;
	}

	memcpy(data, vertex_data, vertex_size);
	data += vertex_size;

	assert(data >= buffer + tail_size);
	assert(data <= buffer + buffer_size);

	return data - buffer;
}

size_t meshopt_encodeVertexBufferBound(size_t vertex_count, size_t vertex_size)
{
	using namespace meshopt;

	assert(vertex_size > 0 && vertex_size <= 256);
	assert(vertex_size % 4 == 0);

	size_t vertex_block_size = getVertexBlockSize(vertex_size);
	size_t vertex_block_count = (vertex_count + vertex_block_size - 1) / vertex_block_size;

	size_t vertex_block_header_size = (vertex_block_size / kByteGroupSize + 3) / 4;
	size_t vertex_block_data_size = vertex_block_size;

	size_t tail_size = vertex_size < kTailMaxSize ? kTailMaxSize : vertex_size;

	return 1 + vertex_block_count * vertex_size * (vertex_block_header_size + vertex_block_data_size) + tail_size;
}

int meshopt_decodeVertexBuffer(void* destination, size_t vertex_count, size_t vertex_size, const unsigned char* buffer, size_t buffer_size)
{
	using namespace meshopt;

	assert(vertex_size > 0 && vertex_size <= 256);
	assert(vertex_size % 4 == 0);

	const unsigned char* (*decode)(const unsigned char*, const unsigned char*, unsigned char*, size_t, size_t, unsigned char[256]) = 0;

#if defined(SIMD_SSE) && defined(SIMD_FALLBACK)
	int cpuinfo[4] = {};
	__cpuid(cpuinfo, 1);
	decode = (cpuinfo[2] & (1 << 9)) ? decodeVertexBlockSimd : decodeVertexBlock;
#elif defined(SIMD_SSE) || defined(SIMD_NEON)
	decode = decodeVertexBlockSimd;
#else
	decode = decodeVertexBlock;
#endif

#if defined(SIMD_SSE) || defined(SIMD_NEON)
	assert(gDecodeBytesGroupInitialized);
#endif

	unsigned char* vertex_data = static_cast<unsigned char*>(destination);

	const unsigned char* data = buffer;
	const unsigned char* data_end = buffer + buffer_size;

	if (size_t(data_end - data) < 1 + vertex_size)
		return -2;

	if (*data++ != kVertexHeader)
		return -1;

	unsigned char last_vertex[256];
	memcpy(last_vertex, data_end - vertex_size, vertex_size);

	size_t vertex_block_size = getVertexBlockSize(vertex_size);

	size_t vertex_offset = 0;

	while (vertex_offset < vertex_count)
	{
		size_t block_size = (vertex_offset + vertex_block_size < vertex_count) ? vertex_block_size : vertex_count - vertex_offset;

		data = decode(data, data_end, vertex_data + vertex_offset * vertex_size, block_size, vertex_size, last_vertex);
		if (!data)
			return -2;

		vertex_offset += block_size;
	}

	size_t tail_size = vertex_size < kTailMaxSize ? kTailMaxSize : vertex_size;

	if (size_t(data_end - data) != tail_size)
		return -3;

	return 0;
}
