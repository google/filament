/*
 * TinyEXR texcomp - ASTC encoder
 *
 * Copyright (c) 2014-2026 Syoyo Fujita and TinyEXR authors
 * SPDX-License-Identifier: Apache-2.0
 */

#include "texcomp.h"
#include "texcomp_internal.h"

#include <limits.h>
#include <stdlib.h>
#include <string.h>

unsigned int tc_astc_ise_sequence_bitcount(unsigned int value_count,
                                           unsigned int quant_level) {
    static const uint8_t scale[21] = {1, 8, 2, 7, 13, 3, 10, 18, 4, 13, 23,
                                      5, 16, 28, 6, 19, 33, 7, 22, 38, 8};
    static const uint8_t div_code[21] = {0, 2, 0, 1, 2, 0, 1, 2, 0, 1, 2,
                                         0, 1, 2, 0, 1, 2, 0, 1, 2, 0};
    unsigned int divisor;
    if (quant_level >= 21u) return 1024u;
    divisor = (unsigned int)div_code[quant_level] * 2u + 1u;
    return ((unsigned int)scale[quant_level] * value_count + divisor - 1u) /
           divisor;
}

static const uint8_t tc_astc_integer_of_quints[5][5][5] = {
    {{0, 1, 2, 3, 4},
     {8, 9, 10, 11, 12},
     {16, 17, 18, 19, 20},
     {24, 25, 26, 27, 28},
     {5, 13, 21, 29, 6}},
    {{32, 33, 34, 35, 36},
     {40, 41, 42, 43, 44},
     {48, 49, 50, 51, 52},
     {56, 57, 58, 59, 60},
     {37, 45, 53, 61, 14}},
    {{64, 65, 66, 67, 68},
     {72, 73, 74, 75, 76},
     {80, 81, 82, 83, 84},
     {88, 89, 90, 91, 92},
     {69, 77, 85, 93, 22}},
    {{96, 97, 98, 99, 100},
     {104, 105, 106, 107, 108},
     {112, 113, 114, 115, 116},
     {120, 121, 122, 123, 124},
     {101, 109, 117, 125, 30}},
    {{102, 103, 70, 71, 38},
     {110, 111, 78, 79, 46},
     {118, 119, 86, 87, 54},
     {126, 127, 94, 95, 62},
     {39, 47, 55, 63, 31}}};

static const uint8_t tc_astc_integer_of_trits[3][3][3][3][3] = {
    {{{{0, 1, 2}, {4, 5, 6}, {8, 9, 10}},
      {{16, 17, 18}, {20, 21, 22}, {24, 25, 26}},
      {{3, 7, 15}, {19, 23, 27}, {12, 13, 14}}},
     {{{32, 33, 34}, {36, 37, 38}, {40, 41, 42}},
      {{48, 49, 50}, {52, 53, 54}, {56, 57, 58}},
      {{35, 39, 47}, {51, 55, 59}, {44, 45, 46}}},
     {{{64, 65, 66}, {68, 69, 70}, {72, 73, 74}},
      {{80, 81, 82}, {84, 85, 86}, {88, 89, 90}},
      {{67, 71, 79}, {83, 87, 91}, {76, 77, 78}}}},
    {{{{128, 129, 130}, {132, 133, 134}, {136, 137, 138}},
      {{144, 145, 146}, {148, 149, 150}, {152, 153, 154}},
      {{131, 135, 143}, {147, 151, 155}, {140, 141, 142}}},
     {{{160, 161, 162}, {164, 165, 166}, {168, 169, 170}},
      {{176, 177, 178}, {180, 181, 182}, {184, 185, 186}},
      {{163, 167, 175}, {179, 183, 187}, {172, 173, 174}}},
     {{{192, 193, 194}, {196, 197, 198}, {200, 201, 202}},
      {{208, 209, 210}, {212, 213, 214}, {216, 217, 218}},
      {{195, 199, 207}, {211, 215, 219}, {204, 205, 206}}}},
    {{{{96, 97, 98}, {100, 101, 102}, {104, 105, 106}},
      {{112, 113, 114}, {116, 117, 118}, {120, 121, 122}},
      {{99, 103, 111}, {115, 119, 123}, {108, 109, 110}}},
     {{{224, 225, 226}, {228, 229, 230}, {232, 233, 234}},
      {{240, 241, 242}, {244, 245, 246}, {248, 249, 250}},
      {{227, 231, 239}, {243, 247, 251}, {236, 237, 238}}},
     {{{28, 29, 30}, {60, 61, 62}, {92, 93, 94}},
      {{156, 157, 158}, {188, 189, 190}, {220, 221, 222}},
      {{31, 63, 127}, {159, 191, 255}, {252, 253, 254}}}}};

tc_result tc_astc_ise_encode_bits(unsigned int quant_level, unsigned int value_count,
                                  const uint8_t *values, uint8_t *out,
                                  size_t out_size, unsigned int bit_offset) {
    static const uint8_t bits[21] = {1, 0, 2, 0, 1, 3, 1, 2, 4, 2, 3,
                                     5, 3, 4, 6, 4, 5, 7, 5, 6, 8};
    static const uint8_t has_trit[21] = {
        0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0};
    static const uint8_t has_quint[21] = {
        0, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0};
    unsigned int i, nbits, total_bits;
    if (!values || !out || !value_count || quant_level >= 21u)
        return TC_ERROR_INVALID_ARGUMENT;
    nbits = bits[quant_level];
    total_bits = tc_astc_ise_sequence_bitcount(value_count, quant_level);
    if (((size_t)bit_offset + total_bits + 7u) / 8u > out_size)
        return TC_ERROR_INVALID_ARGUMENT;
    if (has_trit[quant_level]) {
        unsigned int mask = (1u << nbits) - 1u;
        i = 0;
        while (i + 4u < value_count) {
            uint8_t t = tc_astc_integer_of_trits[values[i + 4u] >> nbits]
                                                 [values[i + 3u] >> nbits]
                                                 [values[i + 2u] >> nbits]
                                                 [values[i + 1u] >> nbits]
                                                 [values[i] >> nbits];
            uint32_t pack = (values[i++] & mask) | (((uint32_t)t & 0x3u) << nbits);
            tc_set_bits(out, &bit_offset, pack, nbits + 2u);
            pack = (values[i++] & mask) | ((((uint32_t)t >> 2) & 0x3u) << nbits);
            tc_set_bits(out, &bit_offset, pack, nbits + 2u);
            pack = (values[i++] & mask) | ((((uint32_t)t >> 4) & 0x1u) << nbits);
            tc_set_bits(out, &bit_offset, pack, nbits + 1u);
            pack = (values[i++] & mask) | ((((uint32_t)t >> 5) & 0x3u) << nbits);
            tc_set_bits(out, &bit_offset, pack, nbits + 2u);
            pack = (values[i++] & mask) | ((((uint32_t)t >> 7) & 0x1u) << nbits);
            tc_set_bits(out, &bit_offset, pack, nbits + 1u);
        }
        if (i != value_count) {
            unsigned int i4 = 0;
            unsigned int i3 = i + 3u >= value_count ? 0 : values[i + 3u] >> nbits;
            unsigned int i2 = i + 2u >= value_count ? 0 : values[i + 2u] >> nbits;
            unsigned int i1 = i + 1u >= value_count ? 0 : values[i + 1u] >> nbits;
            unsigned int i0 = values[i] >> nbits;
            uint8_t t = tc_astc_integer_of_trits[i4][i3][i2][i1][i0];
            unsigned int j = 0;
            static const uint8_t tbits[4] = {2, 2, 1, 2};
            static const uint8_t tshift[4] = {0, 2, 4, 5};
            while (i < value_count) {
                uint32_t pack = (values[i++] & mask) |
                                ((((uint32_t)t >> tshift[j]) &
                                  ((1u << tbits[j]) - 1u))
                                 << nbits);
                tc_set_bits(out, &bit_offset, pack, nbits + tbits[j]);
                ++j;
            }
        }
        return TC_SUCCESS;
    }
    if (has_quint[quant_level]) {
        unsigned int mask = (1u << nbits) - 1u;
        i = 0;
        while (i + 2u < value_count) {
            uint8_t q = tc_astc_integer_of_quints[values[i + 2u] >> nbits]
                                                  [values[i + 1u] >> nbits]
                                                  [values[i] >> nbits];
            uint32_t pack = (values[i++] & mask) | (((uint32_t)q & 0x7u) << nbits);
            tc_set_bits(out, &bit_offset, pack, nbits + 3u);
            pack = (values[i++] & mask) | ((((uint32_t)q >> 3) & 0x3u) << nbits);
            tc_set_bits(out, &bit_offset, pack, nbits + 2u);
            pack = (values[i++] & mask) | ((((uint32_t)q >> 5) & 0x3u) << nbits);
            tc_set_bits(out, &bit_offset, pack, nbits + 2u);
        }
        if (i != value_count) {
            unsigned int i2 = 0;
            unsigned int i1 = i + 1u >= value_count ? 0 : values[i + 1u] >> nbits;
            unsigned int i0 = values[i] >> nbits;
            uint8_t q = tc_astc_integer_of_quints[i2][i1][i0];
            unsigned int j = 0;
            static const uint8_t tbits[2] = {3, 2};
            static const uint8_t tshift[2] = {0, 3};
            while (i < value_count) {
                uint32_t pack = (values[i++] & mask) |
                                ((((uint32_t)q >> tshift[j]) &
                                  ((1u << tbits[j]) - 1u))
                                 << nbits);
                tc_set_bits(out, &bit_offset, pack, nbits + tbits[j]);
                ++j;
            }
        }
        return TC_SUCCESS;
    }
    for (i = 0; i < value_count; ++i) {
        tc_set_bits(out, &bit_offset, values[i], nbits);
    }
    return TC_SUCCESS;
}

static void tc_astc_write_const_from_sum(const uint32_t sum[4], uint32_t count,
                                         uint8_t out[16]) {
    uint32_t c;
    static const uint8_t prefix[8] = {0xfc, 0xfd, 0xff, 0xff,
                                      0xff, 0xff, 0xff, 0xff};
    memcpy(out, prefix, 8u);
    for (c = 0; c < 4u; ++c) {
        uint32_t avg = (sum[c] + count / 2u) / count;
        uint32_t v16 = avg * 257u;
        out[8u + c * 2u] = (uint8_t)v16;
        out[9u + c * 2u] = (uint8_t)(v16 >> 8);
    }
}

static uint8_t tc_bitrev8(uint8_t v) {
    v = (uint8_t)(((v & 0x0fu) << 4) | ((v >> 4) & 0x0fu));
    v = (uint8_t)(((v & 0x33u) << 2) | ((v >> 2) & 0x33u));
    v = (uint8_t)(((v & 0x55u) << 1) | ((v >> 1) & 0x55u));
    return v;
}

static int tc_astc_block_is_solid(const uint8_t block[144][4], uint32_t count) {
    uint32_t i, c;
    for (i = 1; i < count; ++i) {
        for (c = 0; c < 4u; ++c) {
            if (block[i][c] != block[0][c]) return 0;
        }
    }
    return 1;
}

#if defined(TC_X86)
TC_TARGET("sse2")
static int tc_astc_block_is_opaque_sse2(const uint8_t block[144][4],
                                        uint32_t count) {
    const __m128i vff = _mm_set1_epi8((char)0xff);
    uint32_t i = 0;
    for (; i + 4u <= count; i += 4u) {
        __m128i px = _mm_loadu_si128((const __m128i *)block[i]);
        uint32_t mask = (uint32_t)_mm_movemask_epi8(_mm_cmpeq_epi8(px, vff));
        if ((mask & 0x8888u) != 0x8888u) return 0;
    }
    for (; i < count; ++i) {
        if (block[i][3] != 255u) return 0;
    }
    return 1;
}
#endif

static int tc_astc_block_is_opaque(const uint8_t block[144][4], uint32_t count) {
    uint32_t i;
#if defined(TC_X86)
    if (tc_cpu_caps() & (TC_CPU_SSE2 | TC_CPU_SSE41 | TC_CPU_AVX2))
        return tc_astc_block_is_opaque_sse2(block, count);
#endif
    for (i = 0; i < count; ++i) {
        if (block[i][3] != 255u) return 0;
    }
    return 1;
}

static int tc_astc_block_is_luminance(const uint8_t block[144][4],
                                      uint32_t count) {
    uint32_t i;
    for (i = 0; i < count; ++i) {
        if (block[i][0] != block[i][1] || block[i][0] != block[i][2]) return 0;
    }
    return 1;
}

static int tc_astc_block_is_rgb_scale(const uint8_t block[144][4],
                                      uint32_t count) {
    uint32_t i, c, anchor = 0, major = 0;
    uint32_t best_sum = 0, major_v;
    for (i = 0; i < count; ++i) {
        uint32_t sum = (uint32_t)block[i][0] + block[i][1] + block[i][2];
        if (sum > best_sum) {
            best_sum = sum;
            anchor = i;
        }
    }
    if (!best_sum) return 0;
    for (c = 1; c < 3u; ++c) {
        if (block[anchor][c] > block[anchor][major]) major = c;
    }
    major_v = block[anchor][major];
    if (!major_v) return 0;
    for (i = 0; i < count; ++i) {
        for (c = 0; c < 3u; ++c) {
            if (c == major) continue;
            int32_t a = (int32_t)block[i][c] * (int32_t)major_v;
            int32_t b = (int32_t)block[i][major] * (int32_t)block[anchor][c];
            int32_t d = a > b ? a - b : b - a;
            if (d > (int32_t)(major_v * 6u)) return 0;
        }
    }
    return 1;
}

static uint32_t tc_astc_axis_value(const uint8_t p[4], uint32_t axis) {
    if (axis < 4u) return p[axis];
    return tc_luma_u8(p);
}

static uint32_t tc_astc_weight_unquant(uint32_t q, uint32_t quant_level) {
    static const uint8_t table[12][32] = {
        {0, 64},
        {0, 32, 64},
        {0, 21, 43, 64},
        {0, 16, 32, 48, 64},
        {0, 12, 25, 39, 52, 64},
        {0, 9, 18, 27, 37, 46, 55, 64},
        {0, 7, 14, 21, 28, 36, 43, 50, 57, 64},
        {0, 5, 11, 17, 23, 28, 36, 41, 47, 53, 59, 64},
        {0, 4, 8, 12, 17, 21, 25, 29, 35, 39, 43, 47, 52, 56, 60, 64},
        {0, 3, 6, 9, 13, 16, 19, 23, 26, 29, 35, 38, 41, 45, 48, 51,
         55, 58, 61, 64},
        {0, 2, 5, 8, 11, 13, 16, 19, 22, 24, 27, 30, 34, 37, 40, 42,
         45, 48, 51, 53, 56, 59, 62, 64},
        {0, 2, 4, 6, 8, 10, 12, 14, 16, 18, 20, 22, 24, 26, 28, 30,
         34, 36, 38, 40, 42, 44, 46, 48, 50, 52, 54, 56, 58, 60, 62, 64}};
    return table[quant_level][q];
}

static uint8_t tc_astc_weight_scramble(uint32_t q, uint32_t quant_level) {
    static const uint8_t table[12][32] = {
        {0, 1},
        {0, 1, 2},
        {0, 1, 2, 3},
        {0, 1, 2, 3, 4},
        {0, 2, 4, 5, 3, 1},
        {0, 1, 2, 3, 4, 5, 6, 7},
        {0, 2, 4, 6, 8, 9, 7, 5, 3, 1},
        {0, 4, 8, 2, 6, 10, 11, 7, 3, 9, 5, 1},
        {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15},
        {0, 4, 8, 12, 16, 2, 6, 10, 14, 18, 19, 15, 11, 7, 3, 17,
         13, 9, 5, 1},
        {0, 8, 16, 2, 10, 18, 4, 12, 20, 6, 14, 22, 23, 15, 7, 21,
         13, 5, 19, 11, 3, 17, 9, 1},
        {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15,
         16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31}};
    return table[quant_level][q];
}

static uint32_t tc_astc_quant_levels(uint32_t quant_method) {
    static const uint16_t levels[21] = {2,  3,  4,  5,  6,  8,   10,
                                        12, 16, 20, 24, 32, 40,  48,
                                        64, 80, 96, 128, 160, 192, 256};
    return levels[quant_method];
}

static const uint8_t tc_astc_color_pquant_to_uquant_q6[6] = {
    0u, 255u, 51u, 204u, 102u, 153u};
static const uint8_t tc_astc_color_pquant_to_uquant_q8[8] = {
    0u, 36u, 73u, 109u, 146u, 182u, 219u, 255u};
static const uint8_t tc_astc_color_pquant_to_uquant_q10[10] = {
    0u, 255u, 28u, 227u, 56u, 199u, 84u, 171u, 113u, 142u};
static const uint8_t tc_astc_color_pquant_to_uquant_q12[12] = {
    0u, 255u, 69u, 186u, 23u, 232u, 92u, 163u, 46u, 209u, 116u, 139u};
static const uint8_t tc_astc_color_pquant_to_uquant_q16[16] = {
    0u,   17u,  34u,  51u,  68u,  85u,  102u, 119u,
    136u, 153u, 170u, 187u, 204u, 221u, 238u, 255u};
static const uint8_t tc_astc_color_pquant_to_uquant_q20[20] = {
    0u,  255u, 67u, 188u, 13u, 242u, 80u, 175u, 27u, 228u,
    94u, 161u, 40u, 215u, 107u, 148u, 54u, 201u, 121u, 134u};
static const uint8_t tc_astc_color_pquant_to_uquant_q24[24] = {
    0u,  255u, 33u, 222u, 66u, 189u, 99u, 156u, 11u, 244u, 44u, 211u,
    77u, 178u, 110u, 145u, 22u, 233u, 55u, 200u, 88u, 167u, 121u, 134u};
static const uint8_t tc_astc_color_pquant_to_uquant_q32[32] = {
    0u,   8u,   16u,  24u,  33u,  41u,  49u,  57u,
    66u,  74u,  82u,  90u,  99u,  107u, 115u, 123u,
    132u, 140u, 148u, 156u, 165u, 173u, 181u, 189u,
    198u, 206u, 214u, 222u, 231u, 239u, 247u, 255u};
static const uint8_t tc_astc_color_pquant_to_uquant_q40[40] = {
    0u,   255u, 32u, 223u, 65u, 190u, 97u, 158u, 6u,   249u,
    39u,  216u, 71u, 184u, 104u, 151u, 13u, 242u, 45u, 210u,
    78u,  177u, 110u, 145u, 19u, 236u, 52u, 203u, 84u, 171u,
    117u, 138u, 26u, 229u, 58u, 197u, 91u, 164u, 123u, 132u};
static const uint8_t tc_astc_color_pquant_to_uquant_q48[48] = {
    0u,   255u, 16u, 239u, 32u, 223u, 48u, 207u, 65u, 190u, 81u, 174u,
    97u,  158u, 113u, 142u, 5u,  250u, 21u, 234u, 38u, 217u, 54u, 201u,
    70u,  185u, 86u, 169u, 103u, 152u, 119u, 136u, 11u, 244u, 27u, 228u,
    43u,  212u, 59u, 196u, 76u, 179u, 92u, 163u, 108u, 147u, 124u, 131u};
static const uint8_t tc_astc_color_pquant_to_uquant_q64[64] = {
    0u,   4u,   8u,   12u,  16u,  20u,  24u,  28u,
    32u,  36u,  40u,  44u,  48u,  52u,  56u,  60u,
    65u,  69u,  73u,  77u,  81u,  85u,  89u,  93u,
    97u,  101u, 105u, 109u, 113u, 117u, 121u, 125u,
    130u, 134u, 138u, 142u, 146u, 150u, 154u, 158u,
    162u, 166u, 170u, 174u, 178u, 182u, 186u, 190u,
    195u, 199u, 203u, 207u, 211u, 215u, 219u, 223u,
    227u, 231u, 235u, 239u, 243u, 247u, 251u, 255u};
static const uint8_t tc_astc_color_pquant_to_uquant_q80[80] = {
    0u,   255u, 16u, 239u, 32u, 223u, 48u, 207u, 64u, 191u, 80u, 175u,
    96u,  159u, 112u, 143u, 3u,  252u, 19u, 236u, 35u, 220u, 51u, 204u,
    67u,  188u, 83u, 172u, 100u, 155u, 116u, 139u, 6u,  249u, 22u, 233u,
    38u,  217u, 54u, 201u, 71u, 184u, 87u, 168u, 103u, 152u, 119u, 136u,
    9u,   246u, 25u, 230u, 42u, 213u, 58u, 197u, 74u, 181u, 90u, 165u,
    106u, 149u, 122u, 133u, 13u, 242u, 29u, 226u, 45u, 210u, 61u, 194u,
    77u,  178u, 93u, 162u, 109u, 146u, 125u, 130u};
static const uint8_t tc_astc_color_pquant_to_uquant_q96[96] = {
    0u,   255u, 8u,   247u, 16u, 239u, 24u, 231u, 32u, 223u, 40u, 215u,
    48u,  207u, 56u,  199u, 64u, 191u, 72u, 183u, 80u, 175u, 88u, 167u,
    96u,  159u, 104u, 151u, 112u, 143u, 120u, 135u, 2u,  253u, 10u, 245u,
    18u,  237u, 26u,  229u, 35u, 220u, 43u, 212u, 51u, 204u, 59u, 196u,
    67u,  188u, 75u,  180u, 83u, 172u, 91u, 164u, 99u, 156u, 107u, 148u,
    115u, 140u, 123u, 132u, 5u,  250u, 13u, 242u, 21u, 234u, 29u, 226u,
    37u,  218u, 45u,  210u, 53u, 202u, 61u, 194u, 70u, 185u, 78u, 177u,
    86u,  169u, 94u,  161u, 102u, 153u, 110u, 145u, 118u, 137u, 126u, 129u};
static const uint8_t tc_astc_color_pquant_to_uquant_q128[128] = {
    0u,   2u,   4u,   6u,   8u,   10u,  12u,  14u,  16u,  18u,  20u,
    22u,  24u,  26u,  28u,  30u,  32u,  34u,  36u,  38u,  40u,  42u,
    44u,  46u,  48u,  50u,  52u,  54u,  56u,  58u,  60u,  62u,  64u,
    66u,  68u,  70u,  72u,  74u,  76u,  78u,  80u,  82u,  84u,  86u,
    88u,  90u,  92u,  94u,  96u,  98u,  100u, 102u, 104u, 106u, 108u,
    110u, 112u, 114u, 116u, 118u, 120u, 122u, 124u, 126u, 129u, 131u,
    133u, 135u, 137u, 139u, 141u, 143u, 145u, 147u, 149u, 151u, 153u,
    155u, 157u, 159u, 161u, 163u, 165u, 167u, 169u, 171u, 173u, 175u,
    177u, 179u, 181u, 183u, 185u, 187u, 189u, 191u, 193u, 195u, 197u,
    199u, 201u, 203u, 205u, 207u, 209u, 211u, 213u, 215u, 217u, 219u,
    221u, 223u, 225u, 227u, 229u, 231u, 233u, 235u, 237u, 239u, 241u,
    243u, 245u, 247u, 249u, 251u, 253u, 255u};
static const uint8_t tc_astc_color_pquant_to_uquant_q160[160] = {
    0u,   255u, 8u,   247u, 16u, 239u, 24u, 231u, 32u, 223u, 40u, 215u,
    48u,  207u, 56u,  199u, 64u, 191u, 72u, 183u, 80u, 175u, 88u, 167u,
    96u,  159u, 104u, 151u, 112u, 143u, 120u, 135u, 1u,  254u, 9u,  246u,
    17u,  238u, 25u,  230u, 33u, 222u, 41u, 214u, 49u, 206u, 57u, 198u,
    65u,  190u, 73u,  182u, 81u, 174u, 89u, 166u, 97u, 158u, 105u, 150u,
    113u, 142u, 121u, 134u, 3u,  252u, 11u, 244u, 19u, 236u, 27u, 228u,
    35u,  220u, 43u,  212u, 51u, 204u, 59u, 196u, 67u, 188u, 75u, 180u,
    83u,  172u, 91u,  164u, 99u, 156u, 107u, 148u, 115u, 140u, 123u, 132u,
    4u,   251u, 12u,  243u, 20u, 235u, 28u, 227u, 36u, 219u, 44u, 211u,
    52u,  203u, 60u,  195u, 68u, 187u, 76u, 179u, 84u, 171u, 92u, 163u,
    100u, 155u, 108u, 147u, 116u, 139u, 124u, 131u, 6u,  249u, 14u, 241u,
    22u,  233u, 30u,  225u, 38u, 217u, 46u, 209u, 54u, 201u, 62u, 193u,
    70u,  185u, 78u,  177u, 86u, 169u, 94u, 161u, 102u, 153u, 110u, 145u,
    118u, 137u, 126u, 129u};
static const uint8_t tc_astc_color_pquant_to_uquant_q192[192] = {
    0u,   255u, 4u,   251u, 8u,   247u, 12u,  243u, 16u,  239u, 20u,  235u,
    24u,  231u, 28u,  227u, 32u,  223u, 36u,  219u, 40u,  215u, 44u,  211u,
    48u,  207u, 52u,  203u, 56u,  199u, 60u,  195u, 64u,  191u, 68u,  187u,
    72u,  183u, 76u,  179u, 80u,  175u, 84u,  171u, 88u,  167u, 92u,  163u,
    96u,  159u, 100u, 155u, 104u, 151u, 108u, 147u, 112u, 143u, 116u, 139u,
    120u, 135u, 124u, 131u, 1u,   254u, 5u,   250u, 9u,   246u, 13u,  242u,
    17u,  238u, 21u,  234u, 25u,  230u, 29u,  226u, 33u,  222u, 37u,  218u,
    41u,  214u, 45u,  210u, 49u,  206u, 53u,  202u, 57u,  198u, 61u,  194u,
    65u,  190u, 69u,  186u, 73u,  182u, 77u,  178u, 81u,  174u, 85u,  170u,
    89u,  166u, 93u,  162u, 97u,  158u, 101u, 154u, 105u, 150u, 109u, 146u,
    113u, 142u, 117u, 138u, 121u, 134u, 125u, 130u, 2u,   253u, 6u,   249u,
    10u,  245u, 14u,  241u, 18u,  237u, 22u,  233u, 26u,  229u, 30u,  225u,
    34u,  221u, 38u,  217u, 42u,  213u, 46u,  209u, 50u,  205u, 54u,  201u,
    58u,  197u, 62u,  193u, 66u,  189u, 70u,  185u, 74u,  181u, 78u,  177u,
    82u,  173u, 86u,  169u, 90u,  165u, 94u,  161u, 98u,  157u, 102u, 153u,
    106u, 149u, 110u, 145u, 114u, 141u, 118u, 137u, 122u, 133u, 126u, 129u};

static const uint8_t *const tc_astc_color_pquant_to_uquant[17] = {
    tc_astc_color_pquant_to_uquant_q6,   tc_astc_color_pquant_to_uquant_q8,
    tc_astc_color_pquant_to_uquant_q10,  tc_astc_color_pquant_to_uquant_q12,
    tc_astc_color_pquant_to_uquant_q16,  tc_astc_color_pquant_to_uquant_q20,
    tc_astc_color_pquant_to_uquant_q24,  tc_astc_color_pquant_to_uquant_q32,
    tc_astc_color_pquant_to_uquant_q40,  tc_astc_color_pquant_to_uquant_q48,
    tc_astc_color_pquant_to_uquant_q64,  tc_astc_color_pquant_to_uquant_q80,
    tc_astc_color_pquant_to_uquant_q96,  tc_astc_color_pquant_to_uquant_q128,
    tc_astc_color_pquant_to_uquant_q160, tc_astc_color_pquant_to_uquant_q192,
    NULL};

typedef struct tc_astc_block_mode_info {
    uint16_t block_mode;
    uint8_t weight_x;
    uint8_t weight_y;
    uint8_t quant_method;
    uint8_t weight_bits;
    uint8_t dual_plane;
} tc_astc_block_mode_info;

typedef struct tc_astc_decim_cache_entry {
    uint8_t block_x;
    uint8_t block_y;
    uint8_t weight_x;
    uint8_t weight_y;
    uint8_t direct; /* weight grid == block footprint: one weight per texel */
    uint8_t tw_idx[144][4];
    uint8_t tw_contrib[144][4];
    uint8_t tw_count[144];
    uint16_t weight_contrib[64];
} tc_astc_decim_cache_entry;

typedef struct tc_astc_candidate_cache_entry {
    uint8_t valid;
    uint8_t endpoint_end_bit;
    uint32_t count;
    tc_astc_block_mode_info candidates[2048];
} tc_astc_candidate_cache_entry;

#ifndef TC_ASTC_MEDIUM_SCAN_CAP
#define TC_ASTC_MEDIUM_SCAN_CAP 20u
#endif
#ifndef TC_ASTC_MEDIUM_OPAQUE_SCAN_CAP
#define TC_ASTC_MEDIUM_OPAQUE_SCAN_CAP 10u
#endif
#ifndef TC_ASTC_MEDIUM_SELECTED_LIMIT
#define TC_ASTC_MEDIUM_SELECTED_LIMIT 6u
#endif
#ifndef TC_ASTC_MEDIUM_OPAQUE_SELECTED_LIMIT
#define TC_ASTC_MEDIUM_OPAQUE_SELECTED_LIMIT 4u
#endif
#ifndef TC_ASTC_MEDIUM_FULL_AXIS_LIMIT
#define TC_ASTC_MEDIUM_FULL_AXIS_LIMIT 3u
#endif
#ifndef TC_ASTC_MEDIUM_OPAQUE_FULL_AXIS_LIMIT
#define TC_ASTC_MEDIUM_OPAQUE_FULL_AXIS_LIMIT 2u
#endif
#ifndef TC_ASTC_MEDIUM_PART_SCAN_CAP
#define TC_ASTC_MEDIUM_PART_SCAN_CAP 16u
#endif
#ifndef TC_ASTC_MEDIUM_REFINE_ROUNDS
#define TC_ASTC_MEDIUM_REFINE_ROUNDS 2u
#endif
#ifndef TC_ASTC_MEDIUM_PARTITION_ERR_SCALE
#define TC_ASTC_MEDIUM_PARTITION_ERR_SCALE 512u
#endif
#ifndef TC_ASTC_MEDIUM_OPAQUE_PARTITION_ERR_SCALE
#define TC_ASTC_MEDIUM_OPAQUE_PARTITION_ERR_SCALE 3072u
#endif
#ifndef TC_ASTC_PART2_SHORTLIST_BASE
#define TC_ASTC_PART2_SHORTLIST_BASE 32u
#endif
#ifndef TC_ASTC_PART2_SHORTLIST_SHIFT
#define TC_ASTC_PART2_SHORTLIST_SHIFT 1u
#endif

typedef struct tc_astc_partition_info {
    uint16_t partition_index;
    uint8_t partition_of_texel[144];
    uint8_t partition_texel_count[4];
    /* Two-partition assignment as a bitmap (bit i = texel i's partition),
     * used by the estimation prefilter in the partition search. */
    uint64_t bits[3];
} tc_astc_partition_info;

typedef struct tc_astc_encode_context {
    uint32_t block_x;
    uint32_t block_y;
    uint32_t texel_count;
    int quality;
    tc_astc_candidate_cache_entry candidate_cache[4];
    tc_astc_decim_cache_entry decim_cache[169];
    /* (weight_y-2)*11 + (weight_x-2) -> decim_cache slot + 1, 0 = unbuilt. */
    uint8_t decim_index[121];
    tc_astc_partition_info part2_cache[1024];
    tc_astc_partition_info part3_cache[1024];
    tc_astc_partition_info part4_cache[1024];
    uint8_t color_pquant_lut[21][256];
    /* [value_count][bit_budget] -> color quant method, 0xff = none fits. */
    uint8_t color_quant_lut[19][112];
    /* Ideal weight [0,64] -> quantized index / unquantized value, per weight
     * quant method. Collapses the quantize+unquant chain in the fit loops. */
    uint8_t wq_q[12][65];
    uint8_t wq_wt[12][65];
    uint16_t wq_err[12][65];
    tc_astc_block_mode_info dual_candidates[2048];
    uint32_t dual_count;
    uint32_t decim_cache_count;
    uint32_t part2_count;
    uint32_t part3_count;
    uint32_t part4_count;
} tc_astc_encode_context;

static void tc_astc_build_color_pquant_lut(tc_astc_encode_context *ctx);
static uint32_t tc_astc_get_candidates(
    tc_astc_encode_context *ctx, uint32_t endpoint_end_bit,
    const tc_astc_block_mode_info **out_candidates);
static const tc_astc_decim_cache_entry *tc_astc_get_decim_cache(
    tc_astc_encode_context *ctx, uint32_t weight_x, uint32_t weight_y);
static uint32_t tc_astc_build_dual_block_mode_candidates(
    uint32_t block_x, uint32_t block_y, uint32_t endpoint_end_bit, int quality,
    tc_astc_block_mode_info out[2048]);
static int tc_astc_choose_color_quant(uint32_t value_count,
                                      uint32_t bit_budget,
                                      uint32_t *quant_method);
static uint32_t tc_astc_color_symbol_uquant(uint32_t quant_method,
                                            uint32_t sym);
static uint32_t tc_astc_color_roundtrip(const tc_astc_encode_context *ctx,
                                        uint32_t quant_method, uint32_t v);
static void tc_astc_infill_weights(const uint8_t *weights,
                                   const tc_astc_decim_cache_entry *decim,
                                   uint32_t count, uint32_t quant_method,
                                   uint8_t wt[144]);
static void tc_astc_infill_weights_grid(const uint8_t grid[64],
                                        const tc_astc_decim_cache_entry *decim,
                                        uint32_t count, uint8_t wt[144]);

static int tc_astc_decode_block_mode_2d(uint32_t block_mode,
                                        tc_astc_block_mode_info *info) {
    uint32_t base_quant_mode = (block_mode >> 4) & 1u;
    uint32_t h = (block_mode >> 9) & 1u;
    uint32_t d = (block_mode >> 10) & 1u;
    uint32_t a = (block_mode >> 5) & 3u;
    uint32_t weights_x = 0, weights_y = 0, weight_count, real_weight_count;
    uint32_t quant_method, weight_bits;

    if ((block_mode & 3u) != 0u) {
        uint32_t b;
        base_quant_mode |= (block_mode & 3u) << 1;
        b = (block_mode >> 7) & 3u;
        switch ((block_mode >> 2) & 3u) {
            case 0:
                weights_x = b + 4u;
                weights_y = a + 2u;
                break;
            case 1:
                weights_x = b + 8u;
                weights_y = a + 2u;
                break;
            case 2:
                weights_x = a + 2u;
                weights_y = b + 8u;
                break;
            default:
                b &= 1u;
                if (block_mode & 0x100u) {
                    weights_x = b + 2u;
                    weights_y = a + 2u;
                } else {
                    weights_x = a + 2u;
                    weights_y = b + 6u;
                }
                break;
        }
    } else {
        uint32_t b;
        base_quant_mode |= ((block_mode >> 2) & 3u) << 1;
        if (((block_mode >> 2) & 3u) == 0u) return 0;
        b = (block_mode >> 9) & 3u;
        switch ((block_mode >> 7) & 3u) {
            case 0:
                weights_x = 12u;
                weights_y = a + 2u;
                break;
            case 1:
                weights_x = a + 2u;
                weights_y = 12u;
                break;
            case 2:
                weights_x = a + 6u;
                weights_y = b + 6u;
                d = 0u;
                h = 0u;
                break;
            default:
                if (a == 0u) {
                    weights_x = 6u;
                    weights_y = 10u;
                } else if (a == 1u) {
                    weights_x = 10u;
                    weights_y = 6u;
                } else {
                    return 0;
                }
                break;
        }
    }

    if (base_quant_mode < 2u) return 0;
    quant_method = (base_quant_mode - 2u) + 6u * h;
    if (quant_method >= 12u) return 0;
    weight_count = weights_x * weights_y;
    real_weight_count = weight_count * (d + 1u);
    weight_bits = tc_astc_ise_sequence_bitcount(real_weight_count, quant_method);
    if (real_weight_count > 64u || weight_bits < 24u || weight_bits > 96u)
        return 0;
    info->block_mode = (uint16_t)block_mode;
    info->weight_x = (uint8_t)weights_x;
    info->weight_y = (uint8_t)weights_y;
    info->quant_method = (uint8_t)quant_method;
    info->weight_bits = (uint8_t)weight_bits;
    info->dual_plane = (uint8_t)d;
    return 1;
}

#include "texcomp_astc_percentiles.inc"

static int tc_astc_footprint_index(uint32_t block_x, uint32_t block_y) {
    static const uint8_t dims[14][2] = {{4, 4},  {5, 4},  {5, 5},  {6, 5},
                                        {6, 6},  {8, 5},  {8, 6},  {8, 8},
                                        {10, 5}, {10, 6}, {10, 8}, {10, 10},
                                        {12, 10}, {12, 12}};
    int i;
    for (i = 0; i < 14; ++i) {
        if (dims[i][0] == block_x && dims[i][1] == block_y) return i;
    }
    return -1;
}

/* Ranks candidates by real-world usage: primary key is the astcenc
 * block-mode usage percentile for this footprint (lower = used more by
 * actual best encodings), tie-broken by budget utilization x grid
 * resolution. The reduced-effort quality levels only scan a prefix of
 * this ordering. */
static uint64_t tc_astc_candidate_rank(const tc_astc_block_mode_info *info,
                                       int fp, uint32_t dual) {
    uint32_t perc = 65535u;
    uint32_t merit = (uint32_t)info->weight_bits +
                     (uint32_t)info->weight_x * info->weight_y;
    if (fp >= 0) {
        perc = tc_astc_mode_percentile[fp][dual][info->weight_y - 2u]
                                      [info->weight_x - 2u][info->quant_method];
    }
    /* Lower percentile first; higher merit first within a tie. */
    return ((uint64_t)perc << 16) | (0xffffu - merit);
}

static void tc_astc_rank_block_mode_candidates(tc_astc_block_mode_info *out,
                                               uint32_t count,
                                               uint32_t block_x,
                                               uint32_t block_y,
                                               uint32_t dual) {
    int fp = tc_astc_footprint_index(block_x, block_y);
    uint32_t i, j;
    for (i = 1; i < count; ++i) {
        tc_astc_block_mode_info key = out[i];
        uint64_t key_rank = tc_astc_candidate_rank(&key, fp, dual);
        j = i;
        while (j > 0u &&
               tc_astc_candidate_rank(&out[j - 1u], fp, dual) > key_rank) {
            out[j] = out[j - 1u];
            --j;
        }
        out[j] = key;
    }
}

static uint32_t tc_astc_build_block_mode_candidates(uint32_t block_x,
                                                   uint32_t block_y,
                                                   uint32_t endpoint_end_bit,
                                                   int quality,
                                                   tc_astc_block_mode_info out[2048]) {
    uint32_t mode, count = 0;
    uint32_t max_quant = quality > 1 ? 11u : (quality > 0 ? 5u : 2u);
    uint8_t seen[13][13][12];
    memset(seen, 0, sizeof(seen));
    for (mode = 0; mode < 2048u; ++mode) {
        tc_astc_block_mode_info info;
        if (!tc_astc_decode_block_mode_2d(mode, &info)) continue;
        if (info.dual_plane) continue;
        if (info.weight_x > block_x || info.weight_y > block_y) continue;
        if (info.quant_method > max_quant) continue;
        if (endpoint_end_bit > 128u - info.weight_bits) continue;
        if (seen[info.weight_y][info.weight_x][info.quant_method]) continue;
        seen[info.weight_y][info.weight_x][info.quant_method] = 1u;
        out[count++] = info;
    }
    tc_astc_rank_block_mode_candidates(out, count, block_x, block_y, 0u);
    return count;
}

static uint32_t tc_astc_build_dual_block_mode_candidates(
    uint32_t block_x, uint32_t block_y, uint32_t endpoint_end_bit, int quality,
    tc_astc_block_mode_info out[2048]) {
    uint32_t mode, count = 0;
    uint32_t max_quant = quality > 1 ? 11u : 5u;
    uint8_t seen[13][13][12];
    memset(seen, 0, sizeof(seen));
    for (mode = 0; mode < 2048u; ++mode) {
        tc_astc_block_mode_info info;
        if (!tc_astc_decode_block_mode_2d(mode, &info)) continue;
        if (!info.dual_plane) continue;
        if (info.weight_x > block_x || info.weight_y > block_y) continue;
        if (info.quant_method > max_quant) continue;
        if (endpoint_end_bit + 2u > 128u - info.weight_bits) continue;
        if (seen[info.weight_y][info.weight_x][info.quant_method]) continue;
        seen[info.weight_y][info.weight_x][info.quant_method] = 1u;
        out[count++] = info;
    }
    tc_astc_rank_block_mode_candidates(out, count, block_x, block_y, 1u);
    return count;
}

static uint32_t tc_astc_hash52(uint32_t inp) {
    inp ^= inp >> 15;
    inp *= 0xeede0891u;
    inp ^= inp >> 5;
    inp += inp << 16;
    inp ^= inp >> 7;
    inp ^= inp >> 3;
    inp ^= inp << 6;
    inp ^= inp >> 17;
    return inp;
}

static uint8_t tc_astc_select_partition(uint32_t seed, uint32_t x, uint32_t y,
                                        uint32_t partition_count,
                                        int small_block) {
    uint32_t rnum;
    uint32_t seed1, seed2, seed3, seed4, seed5, seed6, seed7, seed8;
    uint32_t seed9, seed10, seed11, seed12;
    uint32_t sh1, sh2, sh3, a, b, c, d;
    if (small_block) {
        x <<= 1;
        y <<= 1;
    }
    seed += (partition_count - 1u) * 1024u;
    rnum = tc_astc_hash52(seed);
    seed1 = rnum & 15u;
    seed2 = (rnum >> 4) & 15u;
    seed3 = (rnum >> 8) & 15u;
    seed4 = (rnum >> 12) & 15u;
    seed5 = (rnum >> 16) & 15u;
    seed6 = (rnum >> 20) & 15u;
    seed7 = (rnum >> 24) & 15u;
    seed8 = (rnum >> 28) & 15u;
    seed9 = (rnum >> 18) & 15u;
    seed10 = (rnum >> 22) & 15u;
    seed11 = (rnum >> 26) & 15u;
    seed12 = ((rnum >> 30) | (rnum << 2)) & 15u;
    seed1 *= seed1;
    seed2 *= seed2;
    seed3 *= seed3;
    seed4 *= seed4;
    seed5 *= seed5;
    seed6 *= seed6;
    seed7 *= seed7;
    seed8 *= seed8;
    seed9 *= seed9;
    seed10 *= seed10;
    seed11 *= seed11;
    seed12 *= seed12;
    if (seed & 1u) {
        sh1 = (seed & 2u) ? 4u : 5u;
        sh2 = partition_count == 3u ? 6u : 5u;
    } else {
        sh1 = partition_count == 3u ? 6u : 5u;
        sh2 = (seed & 2u) ? 4u : 5u;
    }
    sh3 = (seed & 0x10u) ? sh1 : sh2;
    seed1 >>= sh1;
    seed2 >>= sh2;
    seed3 >>= sh1;
    seed4 >>= sh2;
    seed5 >>= sh1;
    seed6 >>= sh2;
    seed7 >>= sh1;
    seed8 >>= sh2;
    seed9 >>= sh3;
    seed10 >>= sh3;
    seed11 >>= sh3;
    seed12 >>= sh3;
    a = seed1 * x + seed2 * y + (rnum >> 14);
    b = seed3 * x + seed4 * y + (rnum >> 10);
    c = seed5 * x + seed6 * y + (rnum >> 6);
    d = seed7 * x + seed8 * y + (rnum >> 2);
    a &= 63u;
    b &= 63u;
    c &= partition_count > 2u ? 63u : 0u;
    d &= partition_count > 3u ? 63u : 0u;
    if (a >= b && a >= c && a >= d) return 0;
    if (b >= c && b >= d) return 1;
    if (c >= d) return 2;
    return 3;
}

/* First-appearance relabeling of a partition map; two seeds with the same
 * canonical form assign the same texel sets to (differently numbered)
 * partitions and are interchangeable for encoding. */
static uint32_t tc_astc_canonical_partition_map(const uint8_t *map,
                                                uint32_t texel_count,
                                                uint8_t canon[144]) {
    uint8_t remap[4] = {0xff, 0xff, 0xff, 0xff};
    uint32_t next = 0, i, h = 2166136261u;
    for (i = 0; i < texel_count; ++i) {
        uint8_t p = map[i];
        if (remap[p] == 0xffu) remap[p] = (uint8_t)next++;
        canon[i] = remap[p];
        h = (h ^ canon[i]) * 16777619u;
    }
    return h;
}

static void tc_astc_build_partition_cache(tc_astc_encode_context *ctx,
                                          uint32_t partition_count,
                                          tc_astc_partition_info cache[1024],
                                          uint32_t *out_count) {
    uint32_t seed, texel_count = ctx->texel_count;
    int small_block = texel_count < 32u;
    uint32_t hashes[1024];
    *out_count = 0;
    for (seed = 0; seed < 1024u; ++seed) {
        tc_astc_partition_info *pi = cache + *out_count;
        uint8_t canon[144];
        uint32_t i, p, hash, counts[4] = {0, 0, 0, 0};
        int duplicate = 0;
        for (i = 0; i < texel_count; ++i) {
            uint32_t x = i % ctx->block_x;
            uint32_t y = i / ctx->block_x;
            uint8_t part =
                tc_astc_select_partition(seed, x, y, partition_count, small_block);
            if (part >= partition_count) part = (uint8_t)(partition_count - 1u);
            pi->partition_of_texel[i] = part;
            ++counts[part];
        }
        for (p = 0; p < partition_count; ++p) {
            if (!counts[p]) break;
        }
        if (p != partition_count) continue;
        /* Drop seeds whose canonical texel assignment already exists; the
         * kept (lowest) seed encodes the identical partitioning. */
        hash = tc_astc_canonical_partition_map(pi->partition_of_texel,
                                               texel_count, canon);
        for (i = 0; i < *out_count; ++i) {
            uint8_t other[144];
            if (hashes[i] != hash) continue;
            (void)tc_astc_canonical_partition_map(
                cache[i].partition_of_texel, texel_count, other);
            if (memcmp(canon, other, texel_count) == 0) {
                duplicate = 1;
                break;
            }
        }
        if (duplicate) continue;
        hashes[*out_count] = hash;
        pi->partition_index = (uint16_t)seed;
        for (p = 0; p < partition_count; ++p)
            pi->partition_texel_count[p] = (uint8_t)counts[p];
        pi->bits[0] = pi->bits[1] = pi->bits[2] = 0;
        if (partition_count == 2u) {
            for (i = 0; i < texel_count; ++i) {
                if (pi->partition_of_texel[i])
                    pi->bits[i >> 6] |= 1ull << (i & 63u);
            }
        }
        ++*out_count;
    }
}

static uint32_t tc_popcount64(uint64_t v) {
#if defined(__GNUC__) || defined(__clang__)
    return (uint32_t)__builtin_popcountll(v);
#else
    uint32_t n = 0;
    while (v) {
        v &= v - 1u;
        ++n;
    }
    return n;
#endif
}

#if defined(TC_X86)
/* Hardware-popcount body for the prefilter mismatch metric; dispatched when
 * SSE4.x is available (POPCNT shipped alongside it). Without the target
 * attribute the baseline build calls the libgcc software popcount. */
TC_TARGET("popcnt")
static uint32_t tc_astc_bitmap_mismatch_popcnt(const uint64_t bbits[3],
                                               const uint64_t seed_bits[3],
                                               const uint64_t mask[3],
                                               uint32_t words) {
    uint32_t direct = 0, inverse = 0, w;
    for (w = 0; w < words; ++w) {
        direct += (uint32_t)__builtin_popcountll((bbits[w] ^ seed_bits[w]) &
                                                 mask[w]);
        inverse += (uint32_t)__builtin_popcountll((bbits[w] ^ ~seed_bits[w]) &
                                                  mask[w]);
    }
    return direct < inverse ? direct : inverse;
}
#endif

#if defined(__ARM_NEON) || defined(__ARM_NEON__)
static uint32_t tc_neon_popcount_u64x2(uint64x2_t v) {
    uint8x16_t c8 = vcntq_u8(vreinterpretq_u8_u64(v));
    uint16x8_t c16 = vpaddlq_u8(c8);
    uint32x4_t c32 = vpaddlq_u16(c16);
    uint64x2_t c64 = vpaddlq_u32(c32);
    return (uint32_t)(vgetq_lane_u64(c64, 0) + vgetq_lane_u64(c64, 1));
}

static uint32_t tc_astc_bitmap_mismatch_neon(const uint64_t bbits[3],
                                             const uint64_t seed_bits[3],
                                             const uint64_t mask[3],
                                             uint32_t words) {
    uint32_t direct = 0, inverse = 0, w = 0;
    const uint64x2_t all_ones = vdupq_n_u64(UINT64_MAX);
    for (; w + 2u <= words; w += 2u) {
        uint64x2_t b = vld1q_u64(bbits + w);
        uint64x2_t s = vld1q_u64(seed_bits + w);
        uint64x2_t m = vld1q_u64(mask + w);
        uint64x2_t sd = vandq_u64(veorq_u64(b, s), m);
        uint64x2_t si = vandq_u64(veorq_u64(b, veorq_u64(s, all_ones)), m);
        direct += tc_neon_popcount_u64x2(sd);
        inverse += tc_neon_popcount_u64x2(si);
    }
    for (; w < words; ++w) {
        direct += tc_popcount64((bbits[w] ^ seed_bits[w]) & mask[w]);
        inverse += tc_popcount64((bbits[w] ^ ~seed_bits[w]) & mask[w]);
    }
    return direct < inverse ? direct : inverse;
}
#endif

static uint32_t tc_astc_bitmap_mismatch(const uint64_t bbits[3],
                                        const uint64_t seed_bits[3],
                                        const uint64_t mask[3],
                                        uint32_t words) {
    uint32_t direct = 0, inverse = 0, w;
#if defined(TC_X86)
    if (tc_cpu_caps() & (TC_CPU_SSE41 | TC_CPU_AVX2))
        return tc_astc_bitmap_mismatch_popcnt(bbits, seed_bits, mask, words);
#endif
#if defined(__ARM_NEON) || defined(__ARM_NEON__)
    if (tc_cpu_caps() & TC_CPU_NEON)
        return tc_astc_bitmap_mismatch_neon(bbits, seed_bits, mask, words);
#endif
    for (w = 0; w < words; ++w) {
        direct += tc_popcount64((bbits[w] ^ seed_bits[w]) & mask[w]);
        inverse += tc_popcount64((bbits[w] ^ ~seed_bits[w]) & mask[w]);
    }
    return direct < inverse ? direct : inverse;
}

static void tc_astc_encode_context_init(tc_astc_encode_context *ctx,
                                        uint32_t block_x, uint32_t block_y,
                                        int quality) {
    memset(ctx, 0, sizeof(*ctx));
    ctx->block_x = block_x;
    ctx->block_y = block_y;
    ctx->texel_count = block_x * block_y;
    ctx->quality = quality;
    tc_astc_build_color_pquant_lut(ctx);
    {
        uint32_t vc, bits, qm, ideal;
        for (vc = 1; vc < 19u; ++vc) {
            for (bits = 0; bits < 112u; ++bits) {
                uint32_t m;
                if (!tc_astc_choose_color_quant(vc, bits, &m)) m = 0xffu;
                ctx->color_quant_lut[vc][bits] = (uint8_t)m;
            }
        }
        for (qm = 0; qm < 12u; ++qm) {
            uint32_t maxq = tc_astc_quant_levels(qm) - 1u;
            for (ideal = 0; ideal <= 64u; ++ideal) {
                uint32_t q = (ideal * maxq + 32u) / 64u;
                if (q > maxq) q = maxq;
                ctx->wq_q[qm][ideal] = (uint8_t)q;
                ctx->wq_wt[qm][ideal] = (uint8_t)tc_astc_weight_unquant(q, qm);
                {
                    int32_t d = (int32_t)ctx->wq_wt[qm][ideal] - (int32_t)ideal;
                    ctx->wq_err[qm][ideal] = (uint16_t)(d * d);
                }
            }
        }
    }
    tc_astc_build_partition_cache(ctx, 2u, ctx->part2_cache, &ctx->part2_count);
    if (quality > 1)
        tc_astc_build_partition_cache(ctx, 3u, ctx->part3_cache, &ctx->part3_count);
    if (quality > 1)
        tc_astc_build_partition_cache(ctx, 4u, ctx->part4_cache, &ctx->part4_count);
    /* Build every lazily-fillable cache eagerly so the context is strictly
     * read-only during encoding and can be shared across encode threads:
     * all decimation grids this footprint can use, the single-plane
     * candidate lists for every endpoint-end-bit key the quality level
     * requests, and the dual-plane candidate list. */
    {
        uint32_t wx, wy;
        const tc_astc_block_mode_info *cl;
        for (wy = 2; wy <= block_y && wy <= 12u; ++wy) {
            for (wx = 2; wx <= block_x && wx <= 12u; ++wx) {
                if (wx * wy > 64u) continue;
                (void)tc_astc_get_decim_cache(ctx, wx, wy);
            }
        }
        if (quality > 0) {
            (void)tc_astc_get_candidates(ctx, 17u, &cl);
        } else {
            (void)tc_astc_get_candidates(ctx, 33u, &cl); /* luminance */
            (void)tc_astc_get_candidates(ctx, 49u, &cl); /* lum+a / scale */
            (void)tc_astc_get_candidates(ctx, 65u, &cl); /* rgb / scale+a */
            (void)tc_astc_get_candidates(ctx, 81u, &cl); /* rgba */
        }
        ctx->dual_count = tc_astc_build_dual_block_mode_candidates(
            block_x, block_y, 17u, quality, ctx->dual_candidates);
    }
}

static uint32_t tc_astc_get_candidates(
    tc_astc_encode_context *ctx, uint32_t endpoint_end_bit,
    const tc_astc_block_mode_info **out_candidates) {
    uint32_t i, empty = 4u;
    for (i = 0; i < 4u; ++i) {
        if (ctx->candidate_cache[i].valid &&
            ctx->candidate_cache[i].endpoint_end_bit == endpoint_end_bit) {
            *out_candidates = ctx->candidate_cache[i].candidates;
            return ctx->candidate_cache[i].count;
        }
        if (!ctx->candidate_cache[i].valid && empty == 4u) empty = i;
    }
    if (empty == 4u) {
        *out_candidates = NULL;
        return 0;
    }
    ctx->candidate_cache[empty].endpoint_end_bit = (uint8_t)endpoint_end_bit;
    ctx->candidate_cache[empty].count = tc_astc_build_block_mode_candidates(
        ctx->block_x, ctx->block_y, endpoint_end_bit, ctx->quality,
        ctx->candidate_cache[empty].candidates);
    ctx->candidate_cache[empty].valid = 1u;
    *out_candidates = ctx->candidate_cache[empty].candidates;
    return ctx->candidate_cache[empty].count;
}

static uint32_t tc_astc_texel_weights_2d(uint32_t block_x, uint32_t block_y,
                                         uint32_t weight_x, uint32_t weight_y,
                                         uint32_t x, uint32_t y,
                                         uint8_t idx[4], uint8_t contrib[4]) {
    uint32_t n = 0;
    uint32_t x_weight =
        (((1024u + block_x / 2u) / (block_x - 1u)) * x * (weight_x - 1u) + 32u) >>
        6;
    uint32_t y_weight =
        (((1024u + block_y / 2u) / (block_y - 1u)) * y * (weight_y - 1u) + 32u) >>
        6;
    uint32_t x_frac = x_weight & 15u;
    uint32_t y_frac = y_weight & 15u;
    uint32_t x_int = x_weight >> 4;
    uint32_t y_int = y_weight >> 4;
    uint32_t qweight[4];
    uint32_t weight[4];
    uint32_t prod = x_frac * y_frac;
    qweight[0] = x_int + y_int * weight_x;
    qweight[1] = qweight[0] + 1u;
    qweight[2] = qweight[0] + weight_x;
    qweight[3] = qweight[2] + 1u;
    weight[3] = (prod + 8u) >> 4;
    weight[1] = x_frac - weight[3];
    weight[2] = y_frac - weight[3];
    weight[0] = 16u - x_frac - y_frac + weight[3];
    for (prod = 0; prod < 4u; ++prod) {
        if (weight[prod]) {
            idx[n] = (uint8_t)qweight[prod];
            contrib[n] = (uint8_t)weight[prod];
            ++n;
        }
    }
    return n;
}

static const tc_astc_decim_cache_entry *tc_astc_get_decim_cache(
    tc_astc_encode_context *ctx, uint32_t weight_x, uint32_t weight_y) {
    uint32_t i;
    uint32_t slot = (weight_y - 2u) * 11u + (weight_x - 2u);
    if (weight_x < 2u || weight_x > 12u || weight_y < 2u || weight_y > 12u)
        return NULL;
    if (ctx->decim_index[slot])
        return ctx->decim_cache + (ctx->decim_index[slot] - 1u);
    if (ctx->decim_cache_count >= 169u) return NULL;
    ctx->decim_index[slot] = (uint8_t)(ctx->decim_cache_count + 1u);
    {
        tc_astc_decim_cache_entry *entry = ctx->decim_cache + ctx->decim_cache_count;
        entry->block_x = (uint8_t)ctx->block_x;
        entry->block_y = (uint8_t)ctx->block_y;
        entry->weight_x = (uint8_t)weight_x;
        entry->weight_y = (uint8_t)weight_y;
        entry->direct =
            (uint8_t)(weight_x == ctx->block_x && weight_y == ctx->block_y);
        for (i = 0; i < 64u; ++i) entry->weight_contrib[i] = 0;
        for (i = 0; i < ctx->texel_count; ++i) {
            uint32_t x = i % ctx->block_x;
            uint32_t y = i / ctx->block_x;
            uint32_t n = tc_astc_texel_weights_2d(
                ctx->block_x, ctx->block_y, weight_x, weight_y, x, y,
                entry->tw_idx[i], entry->tw_contrib[i]);
            uint32_t j;
            entry->tw_count[i] = (uint8_t)n;
            for (j = 0; j < n; ++j) {
                entry->weight_contrib[entry->tw_idx[i][j]] =
                    (uint16_t)(entry->weight_contrib[entry->tw_idx[i][j]] +
                               entry->tw_contrib[i][j]);
            }
            /* Pad to four taps so the infill loop is branch-free. */
            for (; n < 4u; ++n) {
                entry->tw_idx[i][n] = entry->tw_idx[i][0];
                entry->tw_contrib[i][n] = 0;
            }
        }
        ++ctx->decim_cache_count;
        return entry;
    }
}

/* Texel indices of the extreme values along an axis (0-3: channel, 4: luma). */
static void tc_astc_axis_extremes(const uint8_t block[144][4], uint32_t count,
                                  uint32_t axis, uint32_t *out_lo_i,
                                  uint32_t *out_hi_i) {
    uint32_t i, lo_i = 0, hi_i = 0;
    uint32_t lo_key = 0xffffffffu, hi_key = 0u;
    for (i = 0; i < count; ++i) {
        uint32_t key = tc_astc_axis_value(block[i], axis);
        if (key < lo_key) {
            lo_key = key;
            lo_i = i;
        }
        if (key > hi_key) {
            hi_key = key;
            hi_i = i;
        }
    }
    *out_lo_i = lo_i;
    *out_hi_i = hi_i;
}

static void tc_astc_axis_extremes_rgba(const uint8_t block[144][4],
                                       uint32_t count, uint32_t lo_i[5],
                                       uint32_t hi_i[5]) {
    uint32_t lo_key[4] = {0xffffffffu, 0xffffffffu, 0xffffffffu, 0xffffffffu};
    uint32_t hi_key[4] = {0, 0, 0, 0};
    uint32_t i, c;
    for (i = 0; i < count; ++i) {
        for (c = 0; c < 4u; ++c) {
            uint32_t key = block[i][c];
            if (key < lo_key[c]) {
                lo_key[c] = key;
                lo_i[c] = i;
            }
            if (key > hi_key[c]) {
                hi_key[c] = key;
                hi_i[c] = i;
            }
        }
    }
}

/* Fixed-point reciprocal projection: matches (dot*64 + denom/2) / denom
 * within one step and has a division-free per-texel form that the SIMD
 * kernels reproduce bit-exactly. */
#define TC_ASTC_PROJ_RECIP(denom) \
    ((uint32_t)(((1ull << 31) + (denom) / 2u) / (denom)))

static uint32_t tc_astc_project_weight(uint32_t dot64, uint32_t recip) {
    uint32_t w = (uint32_t)(((uint64_t)dot64 * recip + (1u << 30)) >> 31);
    return w > 64u ? 64u : w;
}

/* ---- ASTC search kernels (scalar reference + SIMD, bit-exact) ------------
 * Two vertical loop shapes dominate the encoder search:
 *   project: ideal weight of each texel on a lo->hi endpoint line
 *   recon:   squared error of interpolating endpoints with given weights
 * The SIMD variants must match the scalar forms exactly; the parity tests
 * compare whole compressed images across backends. */

static void tc_astc_project_ideal_scalar(const uint8_t block[144][4],
                                         uint32_t count, const uint32_t lo[4],
                                         const uint32_t hi[4], uint32_t recip,
                                         uint8_t ideal[144]) {
    uint32_t i, c;
    for (i = 0; i < count; ++i) {
        const uint8_t *p = block[i];
        int32_t dot = 0;
        for (c = 0; c < 4u; ++c) {
            dot += (int32_t)((int32_t)p[c] - (int32_t)lo[c]) *
                   (int32_t)((int32_t)hi[c] - (int32_t)lo[c]);
        }
        ideal[i] = (uint8_t)(dot <= 0 ? 0u
                                      : tc_astc_project_weight(
                                            (uint32_t)dot * 64u, recip));
    }
}

static uint64_t tc_astc_recon_sse_scalar(const uint8_t block[144][4],
                                         uint32_t count, const uint32_t e0[4],
                                         const uint32_t e1[4],
                                         const uint8_t wt[144]) {
    uint32_t i, c;
    uint64_t err = 0;
    for (i = 0; i < count; ++i) {
        uint32_t w = wt[i];
        for (c = 0; c < 4u; ++c) {
            uint32_t recon = (e0[c] * (64u - w) + e1[c] * w + 32u) >> 6;
            int32_t d = (int32_t)block[i][c] - (int32_t)recon;
            err += (uint64_t)(d * d);
        }
    }
    return err;
}

static uint64_t tc_astc_recon_sse_pt_scalar(const uint8_t block[144][4],
                                            uint32_t count,
                                            const uint8_t e0t[144][4],
                                            const uint8_t e1t[144][4],
                                            const uint8_t wt[144]) {
    uint32_t i, c;
    uint64_t err = 0;
    for (i = 0; i < count; ++i) {
        uint32_t w = wt[i];
        for (c = 0; c < 4u; ++c) {
            uint32_t recon =
                ((uint32_t)e0t[i][c] * (64u - w) + (uint32_t)e1t[i][c] * w +
                 32u) >>
                6;
            int32_t d = (int32_t)block[i][c] - (int32_t)recon;
            err += (uint64_t)(d * d);
        }
    }
    return err;
}

#if defined(TC_X86)
TC_TARGET("sse2")
static void tc_astc_project_ideal_sse2(const uint8_t block[144][4],
                                       uint32_t count, const uint32_t lo[4],
                                       const uint32_t hi[4], uint32_t recip,
                                       uint8_t ideal[144]) {
    uint32_t packed_lo = lo[0] | (lo[1] << 8) | (lo[2] << 16) | (lo[3] << 24);
    uint32_t packed_hi = hi[0] | (hi[1] << 8) | (hi[2] << 16) | (hi[3] << 24);
    const __m128i zero = _mm_setzero_si128();
    const __m128i lo16 =
        _mm_unpacklo_epi8(_mm_set1_epi32((int32_t)packed_lo), zero);
    const __m128i hi16 =
        _mm_unpacklo_epi8(_mm_set1_epi32((int32_t)packed_hi), zero);
    const __m128i dir16 = _mm_sub_epi16(hi16, lo16);
    const __m128i vrecip = _mm_set1_epi32((int32_t)recip);
    const __m128i round = _mm_set1_epi64x(1ll << 30);
    const __m128i v64 = _mm_set1_epi32(64);
    uint32_t i = 0;
    for (; i + 4u <= count; i += 4u) {
        __m128i px = _mm_loadu_si128((const __m128i *)block[i]);
        __m128i pa = _mm_unpacklo_epi8(px, zero);
        __m128i pb = _mm_unpackhi_epi8(px, zero);
        __m128i da = _mm_madd_epi16(_mm_sub_epi16(pa, lo16), dir16);
        __m128i db = _mm_madd_epi16(_mm_sub_epi16(pb, lo16), dir16);
        /* fold channel pairs: [t.rg, t.ba] -> per-texel dot */
        __m128i sa = _mm_add_epi32(
            da, _mm_shuffle_epi32(da, _MM_SHUFFLE(2, 3, 0, 1)));
        __m128i sb = _mm_add_epi32(
            db, _mm_shuffle_epi32(db, _MM_SHUFFLE(2, 3, 0, 1)));
        __m128i dots = _mm_castps_si128(
            _mm_shuffle_ps(_mm_castsi128_ps(sa), _mm_castsi128_ps(sb),
                           _MM_SHUFFLE(2, 0, 2, 0)));
        __m128i pos = _mm_cmpgt_epi32(dots, zero);
        __m128i dot64 = _mm_slli_epi32(_mm_and_si128(dots, pos), 6);
        __m128i even = _mm_srli_epi64(
            _mm_add_epi64(_mm_mul_epu32(dot64, vrecip), round), 31);
        __m128i odd = _mm_srli_epi64(
            _mm_add_epi64(_mm_mul_epu32(_mm_srli_si128(dot64, 4), vrecip),
                          round),
            31);
        __m128i w = _mm_castps_si128(
            _mm_shuffle_ps(_mm_castsi128_ps(_mm_unpacklo_epi32(even, odd)),
                           _mm_castsi128_ps(_mm_unpackhi_epi32(even, odd)),
                           _MM_SHUFFLE(1, 0, 1, 0)));
        __m128i over = _mm_cmpgt_epi32(w, v64);
        uint32_t tmp[4];
        w = _mm_or_si128(_mm_and_si128(over, v64), _mm_andnot_si128(over, w));
        _mm_storeu_si128((__m128i *)tmp, w);
        ideal[i + 0u] = (uint8_t)tmp[0];
        ideal[i + 1u] = (uint8_t)tmp[1];
        ideal[i + 2u] = (uint8_t)tmp[2];
        ideal[i + 3u] = (uint8_t)tmp[3];
    }
    if (i < count)
        tc_astc_project_ideal_scalar(block + i, count - i, lo, hi, recip,
                                     ideal + i);
}

TC_TARGET("sse2")
static __m128i tc_astc_sse2_wpair(__m128i w4, int which) {
    /* w4 = [w0 w1 w2 w3 x x x x] (u16); replicate a texel pair across
     * channel lanes: [wA wA wA wA wB wB wB wB]. */
    __m128i s = which
                    ? _mm_shufflelo_epi16(w4, _MM_SHUFFLE(3, 3, 2, 2))
                    : _mm_shufflelo_epi16(w4, _MM_SHUFFLE(1, 1, 0, 0));
    return _mm_unpacklo_epi16(s, s);
}

TC_TARGET("sse2")
static uint64_t tc_astc_recon_sse_sse2(const uint8_t block[144][4],
                                       uint32_t count, const uint32_t e0[4],
                                       const uint32_t e1[4],
                                       const uint8_t wt[144]) {
    uint32_t packed_e0 = e0[0] | (e0[1] << 8) | (e0[2] << 16) | (e0[3] << 24);
    uint32_t packed_e1 = e1[0] | (e1[1] << 8) | (e1[2] << 16) | (e1[3] << 24);
    const __m128i zero = _mm_setzero_si128();
    const __m128i e0_16 =
        _mm_unpacklo_epi8(_mm_set1_epi32((int32_t)packed_e0), zero);
    const __m128i e1_16 =
        _mm_unpacklo_epi8(_mm_set1_epi32((int32_t)packed_e1), zero);
    const __m128i v64 = _mm_set1_epi16(64);
    const __m128i v32 = _mm_set1_epi16(32);
    __m128i acc = _mm_setzero_si128();
    uint32_t i = 0;
    uint64_t err;
    for (; i + 4u <= count; i += 4u) {
        __m128i w4 = _mm_unpacklo_epi8(
            _mm_cvtsi32_si128((int32_t)(wt[i] | (wt[i + 1u] << 8) |
                                        ((uint32_t)wt[i + 2u] << 16) |
                                        ((uint32_t)wt[i + 3u] << 24))),
            zero);
        __m128i px = _mm_loadu_si128((const __m128i *)block[i]);
        __m128i pa = _mm_unpacklo_epi8(px, zero);
        __m128i pb = _mm_unpackhi_epi8(px, zero);
        __m128i wa = tc_astc_sse2_wpair(w4, 0);
        __m128i wb = tc_astc_sse2_wpair(w4, 1);
        __m128i ra = _mm_srli_epi16(
            _mm_add_epi16(
                _mm_add_epi16(_mm_mullo_epi16(e0_16, _mm_sub_epi16(v64, wa)),
                              _mm_mullo_epi16(e1_16, wa)),
                v32),
            6);
        __m128i rb = _mm_srli_epi16(
            _mm_add_epi16(
                _mm_add_epi16(_mm_mullo_epi16(e0_16, _mm_sub_epi16(v64, wb)),
                              _mm_mullo_epi16(e1_16, wb)),
                v32),
            6);
        __m128i da = _mm_sub_epi16(pa, ra);
        __m128i db = _mm_sub_epi16(pb, rb);
        acc = _mm_add_epi32(acc, _mm_madd_epi16(da, da));
        acc = _mm_add_epi32(acc, _mm_madd_epi16(db, db));
    }
    {
        uint32_t lanes[4];
        _mm_storeu_si128((__m128i *)lanes, acc);
        err = (uint64_t)lanes[0] + lanes[1] + lanes[2] + lanes[3];
    }
    if (i < count)
        err += tc_astc_recon_sse_scalar(block + i, count - i, e0, e1, wt + i);
    return err;
}

TC_TARGET("sse2")
static uint64_t tc_astc_recon_sse_pt_sse2(const uint8_t block[144][4],
                                          uint32_t count,
                                          const uint8_t e0t[144][4],
                                          const uint8_t e1t[144][4],
                                          const uint8_t wt[144]) {
    const __m128i zero = _mm_setzero_si128();
    const __m128i v64 = _mm_set1_epi16(64);
    const __m128i v32 = _mm_set1_epi16(32);
    __m128i acc = _mm_setzero_si128();
    uint32_t i = 0;
    uint64_t err;
    for (; i + 4u <= count; i += 4u) {
        __m128i w4 = _mm_unpacklo_epi8(
            _mm_cvtsi32_si128((int32_t)(wt[i] | (wt[i + 1u] << 8) |
                                        ((uint32_t)wt[i + 2u] << 16) |
                                        ((uint32_t)wt[i + 3u] << 24))),
            zero);
        __m128i px = _mm_loadu_si128((const __m128i *)block[i]);
        __m128i q0 = _mm_loadu_si128((const __m128i *)e0t[i]);
        __m128i q1 = _mm_loadu_si128((const __m128i *)e1t[i]);
        __m128i wa = tc_astc_sse2_wpair(w4, 0);
        __m128i wb = tc_astc_sse2_wpair(w4, 1);
        __m128i e0a = _mm_unpacklo_epi8(q0, zero);
        __m128i e0b = _mm_unpackhi_epi8(q0, zero);
        __m128i e1a = _mm_unpacklo_epi8(q1, zero);
        __m128i e1b = _mm_unpackhi_epi8(q1, zero);
        __m128i pa = _mm_unpacklo_epi8(px, zero);
        __m128i pb = _mm_unpackhi_epi8(px, zero);
        __m128i ra = _mm_srli_epi16(
            _mm_add_epi16(
                _mm_add_epi16(_mm_mullo_epi16(e0a, _mm_sub_epi16(v64, wa)),
                              _mm_mullo_epi16(e1a, wa)),
                v32),
            6);
        __m128i rb = _mm_srli_epi16(
            _mm_add_epi16(
                _mm_add_epi16(_mm_mullo_epi16(e0b, _mm_sub_epi16(v64, wb)),
                              _mm_mullo_epi16(e1b, wb)),
                v32),
            6);
        __m128i da = _mm_sub_epi16(pa, ra);
        __m128i db = _mm_sub_epi16(pb, rb);
        acc = _mm_add_epi32(acc, _mm_madd_epi16(da, da));
        acc = _mm_add_epi32(acc, _mm_madd_epi16(db, db));
    }
    {
        uint32_t lanes[4];
        _mm_storeu_si128((__m128i *)lanes, acc);
        err = (uint64_t)lanes[0] + lanes[1] + lanes[2] + lanes[3];
    }
    if (i < count)
        err += tc_astc_recon_sse_pt_scalar(block + i, count - i, e0t + i,
                                           e1t + i, wt + i);
    return err;
}

#if defined(__AVX2__)
/* AVX2 project_ideal: 8 texels per iteration, same semantics as SSE2. */
TC_TARGET("avx2")
static void tc_astc_project_ideal_avx2(const uint8_t block[144][4],
                                        uint32_t count, const uint32_t lo[4],
                                        const uint32_t hi[4], uint32_t recip,
                                        uint8_t ideal[144]) {
    uint32_t packed_lo = lo[0] | (lo[1] << 8) | (lo[2] << 16) | (lo[3] << 24);
    uint32_t packed_hi = hi[0] | (hi[1] << 8) | (hi[2] << 16) | (hi[3] << 24);
    const __m256i zero = _mm256_setzero_si256();
    const __m128i zero128 = _mm_setzero_si128();
    const __m256i lo16 = _mm256_cvtepu8_epi16(_mm_set1_epi32((int32_t)packed_lo));
    const __m256i hi16 = _mm256_cvtepu8_epi16(_mm_set1_epi32((int32_t)packed_hi));
    const __m256i dir16 = _mm256_sub_epi16(hi16, lo16);
    const __m256i vrecip = _mm256_set1_epi32((int32_t)recip);
    const __m256i round = _mm256_set1_epi64x(1ll << 30);
    const __m256i v64 = _mm256_set1_epi32(64);
    uint32_t i = 0;
    for (; i + 8u <= count; i += 8u) {
        __m128i px0 = _mm_loadu_si128((const __m128i *)block[i]);
        __m128i px1 = _mm_loadu_si128((const __m128i *)block[i + 4u]);
        __m256i pa = _mm256_cvtepu8_epi16(px0);
        __m256i pb = _mm256_cvtepu8_epi16(px1);
        __m256i da = _mm256_madd_epi16(_mm256_sub_epi16(pa, lo16), dir16);
        __m256i db = _mm256_madd_epi16(_mm256_sub_epi16(pb, lo16), dir16);
        __m256i sa = _mm256_add_epi32(
            da, _mm256_shuffle_epi32(da, _MM_SHUFFLE(2, 3, 0, 1)));
        __m256i sb = _mm256_add_epi32(
            db, _mm256_shuffle_epi32(db, _MM_SHUFFLE(2, 3, 0, 1)));
        __m128i sa_lo = _mm256_castsi256_si128(sa);
        __m128i sa_hi = _mm256_extracti128_si256(sa, 1);
        __m128i sb_lo = _mm256_castsi256_si128(sb);
        __m128i sb_hi = _mm256_extracti128_si256(sb, 1);
        __m128i dots0 = _mm_unpacklo_epi32(sa_lo, sb_lo);
        __m128i dots1 = _mm_unpackhi_epi32(sa_lo, sb_lo);
        __m128i dots2 = _mm_unpacklo_epi32(sa_hi, sb_hi);
        __m128i dots3 = _mm_unpackhi_epi32(sa_hi, sb_hi);
        __m128i pos0 = _mm_cmpgt_epi32(dots0, zero128);
        __m128i pos1 = _mm_cmpgt_epi32(dots1, zero128);
        __m128i pos2 = _mm_cmpgt_epi32(dots2, zero128);
        __m128i pos3 = _mm_cmpgt_epi32(dots3, zero128);
        __m128i d64_0 = _mm_slli_epi32(_mm_and_si128(dots0, pos0), 6);
        __m128i d64_1 = _mm_slli_epi32(_mm_and_si128(dots1, pos1), 6);
        __m128i d64_2 = _mm_slli_epi32(_mm_and_si128(dots2, pos2), 6);
        __m128i d64_3 = _mm_slli_epi32(_mm_and_si128(dots3, pos3), 6);
        __m128i even0 = _mm_srli_epi64(
            _mm_add_epi64(_mm_mul_epu32(d64_0, _mm256_castsi256_si128(vrecip)),
                          _mm256_castsi256_si128(round)), 31);
        __m128i odd0 = _mm_srli_epi64(
            _mm_add_epi64(_mm_mul_epu32(_mm_srli_si128(d64_0, 4),
                                        _mm256_castsi256_si128(vrecip)),
                          _mm256_castsi256_si128(round)), 31);
        __m128i even1 = _mm_srli_epi64(
            _mm_add_epi64(_mm_mul_epu32(d64_1, _mm256_castsi256_si128(vrecip)),
                          _mm256_castsi256_si128(round)), 31);
        __m128i odd1 = _mm_srli_epi64(
            _mm_add_epi64(_mm_mul_epu32(_mm_srli_si128(d64_1, 4),
                                        _mm256_castsi256_si128(vrecip)),
                          _mm256_castsi256_si128(round)), 31);
        __m128i even2 = _mm_srli_epi64(
            _mm_add_epi64(_mm_mul_epu32(d64_2, _mm256_castsi256_si128(vrecip)),
                          _mm256_castsi256_si128(round)), 31);
        __m128i odd2 = _mm_srli_epi64(
            _mm_add_epi64(_mm_mul_epu32(_mm_srli_si128(d64_2, 4),
                                        _mm256_castsi256_si128(vrecip)),
                          _mm256_castsi256_si128(round)), 31);
        __m128i even3 = _mm_srli_epi64(
            _mm_add_epi64(_mm_mul_epu32(d64_3, _mm256_castsi256_si128(vrecip)),
                          _mm256_castsi256_si128(round)), 31);
        __m128i odd3 = _mm_srli_epi64(
            _mm_add_epi64(_mm_mul_epu32(_mm_srli_si128(d64_3, 4),
                                        _mm256_castsi256_si128(vrecip)),
                          _mm256_castsi256_si128(round)), 31);
        __m128i w0 = _mm_unpacklo_epi32(even0, odd0);
        __m128i w1 = _mm_unpacklo_epi32(even1, odd1);
        __m128i w2 = _mm_unpacklo_epi32(even2, odd2);
        __m128i w3 = _mm_unpacklo_epi32(even3, odd3);
        __m128i v64_128 = _mm256_castsi256_si128(v64);
        __m128i over0 = _mm_cmpgt_epi32(w0, v64_128);
        __m128i over1 = _mm_cmpgt_epi32(w1, v64_128);
        __m128i over2 = _mm_cmpgt_epi32(w2, v64_128);
        __m128i over3 = _mm_cmpgt_epi32(w3, v64_128);
        uint32_t tmp[8];
        _mm_storeu_si128((__m128i *)tmp,
            _mm_or_si128(_mm_and_si128(over0, v64_128),
                         _mm_andnot_si128(over0, w0)));
        _mm_storeu_si128((__m128i *)(tmp + 4),
            _mm_or_si128(_mm_and_si128(over1, v64_128),
                         _mm_andnot_si128(over1, w1)));
        ideal[i + 0u] = (uint8_t)tmp[0];
        ideal[i + 1u] = (uint8_t)tmp[1];
        ideal[i + 2u] = (uint8_t)tmp[2];
        ideal[i + 3u] = (uint8_t)tmp[3];
        ideal[i + 4u] = (uint8_t)tmp[4];
        ideal[i + 5u] = (uint8_t)tmp[5];
        ideal[i + 6u] = (uint8_t)tmp[6];
        ideal[i + 7u] = (uint8_t)tmp[7];
    }
    if (i < count)
        tc_astc_project_ideal_scalar(block + i, count - i, lo, hi, recip,
                                     ideal + i);
}

/* AVX2 recon_sse: 8 texels per iteration. */
TC_TARGET("avx2")
static uint64_t tc_astc_recon_sse_avx2(const uint8_t block[144][4],
                                        uint32_t count, const uint32_t e0[4],
                                        const uint32_t e1[4],
                                        const uint8_t wt[144]) {
    uint32_t packed_e0 = e0[0] | (e0[1] << 8) | (e0[2] << 16) | (e0[3] << 24);
    uint32_t packed_e1 = e1[0] | (e1[1] << 8) | (e1[2] << 16) | (e1[3] << 24);
    const __m256i zero = _mm256_setzero_si256();
    const __m256i e0_16 = _mm256_cvtepu8_epi16(
        _mm_set1_epi32((int32_t)packed_e0));
    const __m256i e1_16 = _mm256_cvtepu8_epi16(
        _mm_set1_epi32((int32_t)packed_e1));
    const __m256i v64 = _mm256_set1_epi16(64);
    const __m256i v32 = _mm256_set1_epi16(32);
    __m256i acc = zero;
    uint32_t i = 0;
    uint64_t err;
    for (; i + 8u <= count; i += 8u) {
        __m256i w8 = _mm256_cvtepu8_epi16(
            _mm_loadl_epi64((const __m128i *)(wt + i)));
        __m128i px0 = _mm_loadu_si128((const __m128i *)block[i]);
        __m128i px1 = _mm_loadu_si128((const __m128i *)block[i + 4u]);
        __m256i pa = _mm256_cvtepu8_epi16(px0);
        __m256i pb = _mm256_cvtepu8_epi16(px1);
        __m256i wa = _mm256_shuffle_epi32(
            _mm256_shufflelo_epi16(w8, _MM_SHUFFLE(1, 1, 0, 0)), 0);
        __m256i wb = _mm256_shuffle_epi32(
            _mm256_shufflelo_epi16(w8, _MM_SHUFFLE(3, 3, 2, 2)),
            _MM_SHUFFLE(3, 2, 3, 2));
        __m256i ra = _mm256_srli_epi16(
            _mm256_adds_epi16(
                _mm256_adds_epi16(
                    _mm256_mullo_epi16(e0_16, _mm256_sub_epi16(v64, wa)),
                    _mm256_mullo_epi16(e1_16, wa)),
                v32), 6);
        __m256i rb = _mm256_srli_epi16(
            _mm256_adds_epi16(
                _mm256_adds_epi16(
                    _mm256_mullo_epi16(e0_16, _mm256_sub_epi16(v64, wb)),
                    _mm256_mullo_epi16(e1_16, wb)),
                v32), 6);
        __m256i da = _mm256_sub_epi16(pa, ra);
        __m256i db = _mm256_sub_epi16(pb, rb);
        acc = _mm256_add_epi32(acc, _mm256_madd_epi16(da, da));
        acc = _mm256_add_epi32(acc, _mm256_madd_epi16(db, db));
    }
    {
        uint32_t lanes[8];
        _mm256_storeu_si256((__m256i *)lanes, acc);
        uint32_t j;
        err = 0;
        for (j = 0; j < 8u; ++j) err += (uint64_t)lanes[j];
    }
    if (i < count)
        err += tc_astc_recon_sse_scalar(block + i, count - i, e0, e1, wt + i);
    return err;
}

/* AVX2 recon_sse_pt: per-texel endpoints, 8 texels per iteration. */
TC_TARGET("avx2")
static uint64_t tc_astc_recon_sse_pt_avx2(const uint8_t block[144][4],
                                           uint32_t count,
                                           const uint8_t e0t[144][4],
                                           const uint8_t e1t[144][4],
                                           const uint8_t wt[144]) {
    const __m256i zero = _mm256_setzero_si256();
    const __m256i v64 = _mm256_set1_epi16(64);
    const __m256i v32 = _mm256_set1_epi16(32);
    __m256i acc = zero;
    uint32_t i = 0;
    uint64_t err;
    for (; i + 8u <= count; i += 8u) {
        __m256i w8 = _mm256_cvtepu8_epi16(
            _mm_loadl_epi64((const __m128i *)(wt + i)));
        __m128i p0 = _mm_loadu_si128((const __m128i *)block[i]);
        __m128i p1 = _mm_loadu_si128((const __m128i *)block[i + 4u]);
        __m128i q0_0 = _mm_loadu_si128((const __m128i *)e0t[i]);
        __m128i q0_1 = _mm_loadu_si128((const __m128i *)e0t[i + 4u]);
        __m128i q1_0 = _mm_loadu_si128((const __m128i *)e1t[i]);
        __m128i q1_1 = _mm_loadu_si128((const __m128i *)e1t[i + 4u]);
        __m256i pa = _mm256_cvtepu8_epi16(p0);
        __m256i pb = _mm256_cvtepu8_epi16(p1);
        __m256i e0a = _mm256_cvtepu8_epi16(q0_0);
        __m256i e0b = _mm256_cvtepu8_epi16(q0_1);
        __m256i e1a = _mm256_cvtepu8_epi16(q1_0);
        __m256i e1b = _mm256_cvtepu8_epi16(q1_1);
        __m256i wa = _mm256_shuffle_epi32(
            _mm256_shufflelo_epi16(w8, _MM_SHUFFLE(1, 1, 0, 0)), 0);
        __m256i wb = _mm256_shuffle_epi32(
            _mm256_shufflelo_epi16(w8, _MM_SHUFFLE(3, 3, 2, 2)),
            _MM_SHUFFLE(3, 2, 3, 2));
        __m256i ra = _mm256_srli_epi16(
            _mm256_adds_epi16(
                _mm256_adds_epi16(
                    _mm256_mullo_epi16(e0a, _mm256_sub_epi16(v64, wa)),
                    _mm256_mullo_epi16(e1a, wa)),
                v32), 6);
        __m256i rb = _mm256_srli_epi16(
            _mm256_adds_epi16(
                _mm256_adds_epi16(
                    _mm256_mullo_epi16(e0b, _mm256_sub_epi16(v64, wb)),
                    _mm256_mullo_epi16(e1b, wb)),
                v32), 6);
        __m256i da = _mm256_sub_epi16(pa, ra);
        __m256i db = _mm256_sub_epi16(pb, rb);
        acc = _mm256_add_epi32(acc, _mm256_madd_epi16(da, da));
        acc = _mm256_add_epi32(acc, _mm256_madd_epi16(db, db));
    }
    {
        uint32_t lanes[8];
        _mm256_storeu_si256((__m256i *)lanes, acc);
        uint32_t j;
        err = 0;
        for (j = 0; j < 8u; ++j) err += (uint64_t)lanes[j];
    }
    if (i < count)
        err += tc_astc_recon_sse_pt_scalar(block + i, count - i, e0t + i,
                                           e1t + i, wt + i);
    return err;
}

static uint64_t TC_TARGET("avx2")
tc_astc_recon_sse_dual_avx2(const uint8_t block[144][4], uint32_t count,
                             const uint32_t e0[4], const uint32_t e1[4],
                             const uint8_t wtc[144], const uint8_t wta[144]) {
    uint32_t packed_e0 = e0[0] | (e0[1] << 8) | (e0[2] << 16) | (e0[3] << 24);
    uint32_t packed_e1 = e1[0] | (e1[1] << 8) | (e1[2] << 16) | (e1[3] << 24);
    const __m256i zero = _mm256_setzero_si256();
    const __m256i e0_16 =
        _mm256_unpacklo_epi8(_mm256_set1_epi32((int32_t)packed_e0), zero);
    const __m256i e1_16 =
        _mm256_unpacklo_epi8(_mm256_set1_epi32((int32_t)packed_e1), zero);
    const __m256i v64 = _mm256_set1_epi16(64);
    const __m256i v32 = _mm256_set1_epi16(32);
    __m256i acc = _mm256_setzero_si256();
    uint32_t i = 0;
    uint64_t err;
    for (; i + 8u <= count; i += 8u) {
        __m256i px = _mm256_loadu_si256((const __m256i *)block[i]);
        __m256i pa = _mm256_unpacklo_epi8(px, zero);
        __m256i pb = _mm256_unpackhi_epi8(px, zero);
        __m256i wa = _mm256_set_epi16(
            (int16_t)wtc[i + 5u], (int16_t)wtc[i + 5u],
            (int16_t)wtc[i + 5u], (int16_t)wta[i + 5u],
            (int16_t)wtc[i + 4u], (int16_t)wtc[i + 4u],
            (int16_t)wtc[i + 4u], (int16_t)wta[i + 4u],
            (int16_t)wtc[i + 1u], (int16_t)wtc[i + 1u],
            (int16_t)wtc[i + 1u], (int16_t)wta[i + 1u],
            (int16_t)wtc[i], (int16_t)wtc[i],
            (int16_t)wtc[i], (int16_t)wta[i]);
        __m256i wb = _mm256_set_epi16(
            (int16_t)wtc[i + 7u], (int16_t)wtc[i + 7u],
            (int16_t)wtc[i + 7u], (int16_t)wta[i + 7u],
            (int16_t)wtc[i + 6u], (int16_t)wtc[i + 6u],
            (int16_t)wtc[i + 6u], (int16_t)wta[i + 6u],
            (int16_t)wtc[i + 3u], (int16_t)wtc[i + 3u],
            (int16_t)wtc[i + 3u], (int16_t)wta[i + 3u],
            (int16_t)wtc[i + 2u], (int16_t)wtc[i + 2u],
            (int16_t)wtc[i + 2u], (int16_t)wta[i + 2u]);
        __m256i ra = _mm256_srli_epi16(
            _mm256_add_epi16(
                _mm256_add_epi16(
                    _mm256_mullo_epi16(e0_16, _mm256_sub_epi16(v64, wa)),
                    _mm256_mullo_epi16(e1_16, wa)),
                v32),
            6);
        __m256i rb = _mm256_srli_epi16(
            _mm256_add_epi16(
                _mm256_add_epi16(
                    _mm256_mullo_epi16(e0_16, _mm256_sub_epi16(v64, wb)),
                    _mm256_mullo_epi16(e1_16, wb)),
                v32),
            6);
        __m256i da = _mm256_sub_epi16(pa, ra);
        __m256i db = _mm256_sub_epi16(pb, rb);
        acc = _mm256_add_epi32(acc, _mm256_madd_epi16(da, da));
        acc = _mm256_add_epi32(acc, _mm256_madd_epi16(db, db));
    }
    {
        uint64_t lanes[4];
        _mm256_storeu_si256((__m256i *)lanes, acc);
        err = lanes[0] + lanes[1] + lanes[2] + lanes[3];
    }
    for (; i < count; ++i) {
        uint32_t c;
        for (c = 0; c < 4u; ++c) {
            uint32_t w = c == 3u ? wta[i] : wtc[i];
            uint32_t recon = (e0[c] * (64u - w) + e1[c] * w + 32u) >> 6;
            int32_t d = (int32_t)block[i][c] - (int32_t)recon;
            err += (uint64_t)(d * d);
        }
    }
    return err;
}

static uint64_t TC_TARGET("avx2")
tc_astc_recon_sse_dual_pt_avx2(const uint8_t block[144][4], uint32_t count,
                                const uint8_t e0t[144][4],
                                const uint8_t e1t[144][4],
                                const uint8_t wtc[144],
                                const uint8_t wta[144]) {
    const __m256i zero = _mm256_setzero_si256();
    const __m256i v64 = _mm256_set1_epi16(64);
    const __m256i v32 = _mm256_set1_epi16(32);
    __m256i acc = _mm256_setzero_si256();
    uint32_t i = 0;
    uint64_t err;
    for (; i + 8u <= count; i += 8u) {
        __m256i px = _mm256_loadu_si256((const __m256i *)block[i]);
        __m256i e0_all = _mm256_loadu_si256((const __m256i *)e0t[i]);
        __m256i e1_all = _mm256_loadu_si256((const __m256i *)e1t[i]);
        __m256i pa = _mm256_unpacklo_epi8(px, zero);
        __m256i pb = _mm256_unpackhi_epi8(px, zero);
        __m256i e0a = _mm256_unpacklo_epi8(e0_all, zero);
        __m256i e0b = _mm256_unpackhi_epi8(e0_all, zero);
        __m256i e1a = _mm256_unpacklo_epi8(e1_all, zero);
        __m256i e1b = _mm256_unpackhi_epi8(e1_all, zero);
        __m256i wa = _mm256_set_epi16(
            (int16_t)wtc[i + 5u], (int16_t)wtc[i + 5u],
            (int16_t)wtc[i + 5u], (int16_t)wta[i + 5u],
            (int16_t)wtc[i + 4u], (int16_t)wtc[i + 4u],
            (int16_t)wtc[i + 4u], (int16_t)wta[i + 4u],
            (int16_t)wtc[i + 1u], (int16_t)wtc[i + 1u],
            (int16_t)wtc[i + 1u], (int16_t)wta[i + 1u],
            (int16_t)wtc[i], (int16_t)wtc[i],
            (int16_t)wtc[i], (int16_t)wta[i]);
        __m256i wb = _mm256_set_epi16(
            (int16_t)wtc[i + 7u], (int16_t)wtc[i + 7u],
            (int16_t)wtc[i + 7u], (int16_t)wta[i + 7u],
            (int16_t)wtc[i + 6u], (int16_t)wtc[i + 6u],
            (int16_t)wtc[i + 6u], (int16_t)wta[i + 6u],
            (int16_t)wtc[i + 3u], (int16_t)wtc[i + 3u],
            (int16_t)wtc[i + 3u], (int16_t)wta[i + 3u],
            (int16_t)wtc[i + 2u], (int16_t)wtc[i + 2u],
            (int16_t)wtc[i + 2u], (int16_t)wta[i + 2u]);
        __m256i ra = _mm256_srli_epi16(
            _mm256_add_epi16(
                _mm256_add_epi16(
                    _mm256_mullo_epi16(e0a, _mm256_sub_epi16(v64, wa)),
                    _mm256_mullo_epi16(e1a, wa)),
                v32),
            6);
        __m256i rb = _mm256_srli_epi16(
            _mm256_add_epi16(
                _mm256_add_epi16(
                    _mm256_mullo_epi16(e0b, _mm256_sub_epi16(v64, wb)),
                    _mm256_mullo_epi16(e1b, wb)),
                v32),
            6);
        __m256i da = _mm256_sub_epi16(pa, ra);
        __m256i db = _mm256_sub_epi16(pb, rb);
        acc = _mm256_add_epi32(acc, _mm256_madd_epi16(da, da));
        acc = _mm256_add_epi32(acc, _mm256_madd_epi16(db, db));
    }
    {
        uint64_t lanes[4];
        _mm256_storeu_si256((__m256i *)lanes, acc);
        err = lanes[0] + lanes[1] + lanes[2] + lanes[3];
    }
    for (; i < count; ++i) {
        uint32_t c;
        for (c = 0; c < 4u; ++c) {
            uint32_t w = c == 3u ? wta[i] : wtc[i];
            uint32_t recon =
                ((uint32_t)e0t[i][c] * (64u - w) + (uint32_t)e1t[i][c] * w + 32u) >> 6;
            int32_t d = (int32_t)block[i][c] - (int32_t)recon;
            err += (uint64_t)(d * d);
        }
    }
    return err;
}

/* AVX2 infill+recon for one candidate in score_batch. Processes 2 texels per
 * iteration using _mm_i32gather_epi32 for the grid-weight lookups. Returns
 * the sum of squared errors. */
static uint64_t TC_TARGET("avx2")
tc_astc_infill_recon_avx2(const uint8_t *grid_wt,
                            const tc_astc_decim_cache_entry *decim,
                            uint32_t count, const uint8_t *ideal) {
    const __m128i v8 = _mm_set1_epi32(8);
    const __m128i v64 = _mm_set1_epi32(64);
    const __m128i zero = _mm_setzero_si128();
    __m128i acc = zero;
    uint32_t i;
    for (i = 0; i + 2u <= count; i += 2u) {
        __m128i idx = _mm_loadl_epi64((const __m128i *)(decim->tw_idx + i));
        __m128i cb = _mm_loadl_epi64((const __m128i *)(decim->tw_contrib + i));
        __m128i idx_lo = _mm_cvtepu8_epi32(idx);
        __m128i idx_hi = _mm_cvtepu8_epi32(_mm_srli_si128(idx, 4));
        __m128i cb_lo = _mm_cvtepu8_epi32(cb);
        __m128i cb_hi = _mm_cvtepu8_epi32(_mm_srli_si128(cb, 4));
        __m128i g_lo = _mm_i32gather_epi32((const int *)grid_wt, idx_lo, 1);
        __m128i g_hi = _mm_i32gather_epi32((const int *)grid_wt, idx_hi, 1);
        __m128i p_lo = _mm_mullo_epi32(g_lo, cb_lo);
        __m128i p_hi = _mm_mullo_epi32(g_hi, cb_hi);
        __m128i sl = _mm_shuffle_epi32(p_lo, _MM_SHUFFLE(2, 3, 0, 1));
        __m128i s2 = _mm_add_epi32(p_lo, sl);
        __m128i s3 = _mm_shuffle_epi32(s2, _MM_SHUFFLE(1, 0, 3, 2));
        __m128i t0 = _mm_add_epi32(s2, s3);
        sl = _mm_shuffle_epi32(p_hi, _MM_SHUFFLE(2, 3, 0, 1));
        s2 = _mm_add_epi32(p_hi, sl);
        s3 = _mm_shuffle_epi32(s2, _MM_SHUFFLE(1, 0, 3, 2));
        __m128i t1 = _mm_add_epi32(s2, s3);
        __m128i sum = _mm_unpacklo_epi32(t0, t1);
        sum = _mm_srli_epi32(_mm_add_epi32(sum, v8), 4);
        sum = _mm_min_epi32(_mm_max_epi32(sum, zero), v64);
        __m128i id = _mm_cvtepu8_epi32(
            _mm_loadl_epi64((const __m128i *)(ideal + i)));
        __m128i d = _mm_sub_epi32(sum, id);
        __m128i err = _mm_mullo_epi32(d, d);
        acc = _mm_add_epi32(acc, err);
    }
    {
        uint32_t lanes[4];
        _mm_storeu_si128((__m128i *)lanes, acc);
        uint64_t err = (uint64_t)lanes[0] + (uint64_t)lanes[1];
        for (; i < count; ++i) {
            const uint8_t *idx = decim->tw_idx[i];
            const uint8_t *cb = decim->tw_contrib[i];
            uint32_t w = (8u + grid_wt[idx[0]] * cb[0] + grid_wt[idx[1]] * cb[1] +
                          grid_wt[idx[2]] * cb[2] + grid_wt[idx[3]] * cb[3]) >> 4;
            int32_t d = (int32_t)w - (int32_t)ideal[i];
            err += (uint64_t)(d * d);
        }
        return err;
    }
}

/* AVX2 weight accumulation for weights_from_ideal. Gathers 4 accumulators
 * per texel (all 4 taps are always padded to 4 entries), multiplies
 * ideal[i] by each contribution, adds, and stores back individually. */
TC_TARGET("avx2")
static void tc_astc_accum_weights_avx2(uint32_t weight_accum[64],
                                        const tc_astc_decim_cache_entry *decim,
                                        uint32_t count,
                                        const uint8_t ideal[144]) {
    uint32_t i;
    for (i = 0; i < count; ++i) {
        __m128i idx = _mm_cvtepu8_epi32(
            _mm_loadl_epi64((const __m128i *)decim->tw_idx[i]));
        __m128i cb = _mm_cvtepu8_epi32(
            _mm_loadl_epi64((const __m128i *)decim->tw_contrib[i]));
        __m128i vacc = _mm_i32gather_epi32(
            (const int *)weight_accum, idx, 4);
        __m128i vval = _mm_set1_epi32((int)ideal[i]);
        __m128i vprod = _mm_mullo_epi32(vval, cb);
        vacc = _mm_add_epi32(vacc, vprod);
        {
            int32_t temp[4];
            _mm_storeu_si128((__m128i *)temp, vacc);
            weight_accum[(uint32_t) _mm_extract_epi32(idx, 0)] = (uint32_t)temp[0];
            weight_accum[(uint32_t) _mm_extract_epi32(idx, 1)] = (uint32_t)temp[1];
            weight_accum[(uint32_t) _mm_extract_epi32(idx, 2)] = (uint32_t)temp[2];
            weight_accum[(uint32_t) _mm_extract_epi32(idx, 3)] = (uint32_t)temp[3];
        }
    }
}
#endif /* __AVX2__ */
#endif /* TC_X86 */

#if defined(__ARM_NEON) || defined(__ARM_NEON__)
static void tc_astc_project_ideal_neon(const uint8_t block[144][4],
                                       uint32_t count, const uint32_t lo[4],
                                       const uint32_t hi[4], uint32_t recip,
                                       uint8_t ideal[144]) {
    uint8_t lob[8], hib[8];
    uint32_t i = 0, c;
    int16x8_t lo16, dir16;
    for (c = 0; c < 4u; ++c) {
        lob[c] = lob[c + 4u] = (uint8_t)lo[c];
        hib[c] = hib[c + 4u] = (uint8_t)hi[c];
    }
    lo16 = vreinterpretq_s16_u16(vmovl_u8(vld1_u8(lob)));
    dir16 = vsubq_s16(vreinterpretq_s16_u16(vmovl_u8(vld1_u8(hib))), lo16);
    for (; i + 2u <= count; i += 2u) {
        int16x8_t p16 = vreinterpretq_s16_u16(vmovl_u8(vld1_u8(block[i])));
        int16x8_t d = vsubq_s16(p16, lo16);
        int32x4_t m = vmull_s16(vget_low_s16(d), vget_low_s16(dir16));
        int32x4_t n = vmull_s16(vget_high_s16(d), vget_high_s16(dir16));
        int32_t d0 = vaddvq_s32(m);
        int32_t d1 = vaddvq_s32(n);
        ideal[i] = (uint8_t)(d0 <= 0 ? 0u
                                     : tc_astc_project_weight(
                                           (uint32_t)d0 * 64u, recip));
        ideal[i + 1u] =
            (uint8_t)(d1 <= 0 ? 0u
                              : tc_astc_project_weight((uint32_t)d1 * 64u,
                                                       recip));
    }
    if (i < count)
        tc_astc_project_ideal_scalar(block + i, count - i, lo, hi, recip,
                                     ideal + i);
}

static uint64_t tc_astc_recon_sse_neon(const uint8_t block[144][4],
                                       uint32_t count, const uint32_t e0[4],
                                       const uint32_t e1[4],
                                       const uint8_t wt[144]) {
    uint8_t e0b[8], e1b[8];
    uint32_t i = 0, c;
    uint16x8_t e0_16, e1_16;
    uint32x4_t acc = vdupq_n_u32(0);
    uint64_t err;
    for (c = 0; c < 4u; ++c) {
        e0b[c] = e0b[c + 4u] = (uint8_t)e0[c];
        e1b[c] = e1b[c + 4u] = (uint8_t)e1[c];
    }
    e0_16 = vmovl_u8(vld1_u8(e0b));
    e1_16 = vmovl_u8(vld1_u8(e1b));
    for (; i + 2u <= count; i += 2u) {
        uint16x8_t p16 = vmovl_u8(vld1_u8(block[i]));
        uint16x8_t w = vcombine_u16(vdup_n_u16(wt[i]), vdup_n_u16(wt[i + 1u]));
        uint16x8_t r = vshrq_n_u16(
            vaddq_u16(vaddq_u16(vmulq_u16(e0_16,
                                          vsubq_u16(vdupq_n_u16(64), w)),
                                vmulq_u16(e1_16, w)),
                      vdupq_n_u16(32)),
            6);
        int16x8_t d = vsubq_s16(vreinterpretq_s16_u16(p16),
                                vreinterpretq_s16_u16(r));
        acc = vaddq_u32(
            acc, vreinterpretq_u32_s32(vmull_s16(vget_low_s16(d),
                                                 vget_low_s16(d))));
        acc = vaddq_u32(
            acc, vreinterpretq_u32_s32(vmull_s16(vget_high_s16(d),
                                                 vget_high_s16(d))));
    }
    err = (uint64_t)vaddvq_u32(acc);
    if (i < count)
        err += tc_astc_recon_sse_scalar(block + i, count - i, e0, e1, wt + i);
    return err;
}

static uint64_t tc_astc_recon_sse_pt_neon(const uint8_t block[144][4],
                                          uint32_t count,
                                          const uint8_t e0t[144][4],
                                          const uint8_t e1t[144][4],
                                          const uint8_t wt[144]) {
    uint32x4_t acc = vdupq_n_u32(0);
    uint32_t i = 0;
    uint64_t err;
    for (; i + 2u <= count; i += 2u) {
        uint16x8_t p16 = vmovl_u8(vld1_u8(block[i]));
        uint16x8_t q0 = vmovl_u8(vld1_u8(e0t[i]));
        uint16x8_t q1 = vmovl_u8(vld1_u8(e1t[i]));
        uint16x8_t w = vcombine_u16(vdup_n_u16(wt[i]), vdup_n_u16(wt[i + 1u]));
        uint16x8_t r = vshrq_n_u16(
            vaddq_u16(vaddq_u16(vmulq_u16(q0, vsubq_u16(vdupq_n_u16(64), w)),
                                vmulq_u16(q1, w)),
                      vdupq_n_u16(32)),
            6);
        int16x8_t d = vsubq_s16(vreinterpretq_s16_u16(p16),
                                vreinterpretq_s16_u16(r));
        acc = vaddq_u32(
            acc, vreinterpretq_u32_s32(vmull_s16(vget_low_s16(d),
                                                 vget_low_s16(d))));
        acc = vaddq_u32(
            acc, vreinterpretq_u32_s32(vmull_s16(vget_high_s16(d),
                                                 vget_high_s16(d))));
    }
    err = (uint64_t)vaddvq_u32(acc);
    if (i < count)
        err += tc_astc_recon_sse_pt_scalar(block + i, count - i, e0t + i,
                                           e1t + i, wt + i);
    return err;
}
#endif /* NEON */

static void tc_astc_project_ideal(const uint8_t block[144][4], uint32_t count,
                                  const uint32_t lo[4], const uint32_t hi[4],
                                  uint32_t recip, uint8_t ideal[144]) {
#if defined(__AVX2__)
    if (tc_cpu_caps() & TC_CPU_AVX2) {
        tc_astc_project_ideal_avx2(block, count, lo, hi, recip, ideal);
        return;
    }
#endif
#if defined(TC_X86)
    if (tc_cpu_caps() & (TC_CPU_SSE2 | TC_CPU_SSE41)) {
        tc_astc_project_ideal_sse2(block, count, lo, hi, recip, ideal);
        return;
    }
#endif
#if defined(__ARM_NEON) || defined(__ARM_NEON__)
    if (tc_cpu_caps() & TC_CPU_NEON) {
        tc_astc_project_ideal_neon(block, count, lo, hi, recip, ideal);
        return;
    }
#endif
    tc_astc_project_ideal_scalar(block, count, lo, hi, recip, ideal);
}

static uint64_t tc_astc_recon_sse(const uint8_t block[144][4], uint32_t count,
                                  const uint32_t e0[4], const uint32_t e1[4],
                                  const uint8_t wt[144]) {
#if defined(__AVX2__)
    if (tc_cpu_caps() & TC_CPU_AVX2)
        return tc_astc_recon_sse_avx2(block, count, e0, e1, wt);
#endif
#if defined(TC_X86)
    if (tc_cpu_caps() & (TC_CPU_SSE2 | TC_CPU_SSE41))
        return tc_astc_recon_sse_sse2(block, count, e0, e1, wt);
#endif
#if defined(__ARM_NEON) || defined(__ARM_NEON__)
    if (tc_cpu_caps() & TC_CPU_NEON)
        return tc_astc_recon_sse_neon(block, count, e0, e1, wt);
#endif
    return tc_astc_recon_sse_scalar(block, count, e0, e1, wt);
}

static uint64_t tc_astc_recon_sse_pt(const uint8_t block[144][4],
                                      uint32_t count, const uint8_t e0t[144][4],
                                      const uint8_t e1t[144][4],
                                      const uint8_t wt[144]) {
#if defined(__AVX2__)
    if (tc_cpu_caps() & TC_CPU_AVX2)
        return tc_astc_recon_sse_pt_avx2(block, count, e0t, e1t, wt);
#endif
#if defined(TC_X86)
    if (tc_cpu_caps() & (TC_CPU_SSE2 | TC_CPU_SSE41))
        return tc_astc_recon_sse_pt_sse2(block, count, e0t, e1t, wt);
#endif
#if defined(__ARM_NEON) || defined(__ARM_NEON__)
    if (tc_cpu_caps() & TC_CPU_NEON)
        return tc_astc_recon_sse_pt_neon(block, count, e0t, e1t, wt);
#endif
    return tc_astc_recon_sse_pt_scalar(block, count, e0t, e1t, wt);
}

#if defined(TC_X86)
TC_TARGET("sse2")
static uint64_t tc_astc_recon_sse_dual_sse2(const uint8_t block[144][4],
                                            uint32_t count,
                                            const uint32_t e0[4],
                                            const uint32_t e1[4],
                                            const uint8_t wtc[144],
                                            const uint8_t wta[144]) {
    uint32_t packed_e0 = e0[0] | (e0[1] << 8) | (e0[2] << 16) | (e0[3] << 24);
    uint32_t packed_e1 = e1[0] | (e1[1] << 8) | (e1[2] << 16) | (e1[3] << 24);
    const __m128i zero = _mm_setzero_si128();
    const __m128i e0_16 =
        _mm_unpacklo_epi8(_mm_set1_epi32((int32_t)packed_e0), zero);
    const __m128i e1_16 =
        _mm_unpacklo_epi8(_mm_set1_epi32((int32_t)packed_e1), zero);
    const __m128i v64 = _mm_set1_epi16(64);
    const __m128i v32 = _mm_set1_epi16(32);
    __m128i acc = _mm_setzero_si128();
    uint32_t i = 0;
    uint64_t err;
    for (; i + 4u <= count; i += 4u) {
        __m128i px = _mm_loadu_si128((const __m128i *)block[i]);
        __m128i pa = _mm_unpacklo_epi8(px, zero);
        __m128i pb = _mm_unpackhi_epi8(px, zero);
        __m128i wa = _mm_set_epi16((int16_t)wta[i + 1u], (int16_t)wtc[i + 1u],
                                   (int16_t)wtc[i + 1u], (int16_t)wtc[i + 1u],
                                   (int16_t)wta[i], (int16_t)wtc[i],
                                   (int16_t)wtc[i], (int16_t)wtc[i]);
        __m128i wb = _mm_set_epi16((int16_t)wta[i + 3u], (int16_t)wtc[i + 3u],
                                   (int16_t)wtc[i + 3u], (int16_t)wtc[i + 3u],
                                   (int16_t)wta[i + 2u], (int16_t)wtc[i + 2u],
                                   (int16_t)wtc[i + 2u], (int16_t)wtc[i + 2u]);
        __m128i ra = _mm_srli_epi16(
            _mm_add_epi16(
                _mm_add_epi16(_mm_mullo_epi16(e0_16, _mm_sub_epi16(v64, wa)),
                              _mm_mullo_epi16(e1_16, wa)),
                v32),
            6);
        __m128i rb = _mm_srli_epi16(
            _mm_add_epi16(
                _mm_add_epi16(_mm_mullo_epi16(e0_16, _mm_sub_epi16(v64, wb)),
                              _mm_mullo_epi16(e1_16, wb)),
                v32),
            6);
        __m128i da = _mm_sub_epi16(pa, ra);
        __m128i db = _mm_sub_epi16(pb, rb);
        acc = _mm_add_epi32(acc, _mm_madd_epi16(da, da));
        acc = _mm_add_epi32(acc, _mm_madd_epi16(db, db));
    }
    {
        uint32_t lanes[4];
        _mm_storeu_si128((__m128i *)lanes, acc);
        err = (uint64_t)lanes[0] + lanes[1] + lanes[2] + lanes[3];
    }
    for (; i < count; ++i) {
        uint32_t c;
        for (c = 0; c < 4u; ++c) {
            uint32_t w = c == 3u ? wta[i] : wtc[i];
            uint32_t recon = (e0[c] * (64u - w) + e1[c] * w + 32u) >> 6;
            int32_t d = (int32_t)block[i][c] - (int32_t)recon;
            err += (uint64_t)(d * d);
        }
    }
    return err;
}
#endif

/* Endpoint line for an axis: the extreme texels' colors plus the projection
 * reciprocal. Returns 0 when the line is degenerate. */
static int tc_astc_axis_line(const uint8_t block[144][4], uint32_t lo_i,
                             uint32_t hi_i, uint32_t lo[4], uint32_t hi[4],
                             uint32_t *recip, uint32_t *out_denom) {
    uint32_t c, denom = 0;
    for (c = 0; c < 4u; ++c) {
        uint32_t d;
        lo[c] = block[lo_i][c];
        hi[c] = block[hi_i][c];
        d = hi[c] > lo[c] ? hi[c] - lo[c] : lo[c] - hi[c];
        denom += d * d;
    }
    if (!denom) return 0;
    *recip = TC_ASTC_PROJ_RECIP(denom);
    if (out_denom) *out_denom = denom;
    return 1;
}

/* Weight-space candidate score, following astcenc's
 * compute_error_of_weight_set: instead of reconstructing every texel
 * (O(texels*4)), score a candidate grid by how much quantization moves
 * the interpolated weights away from the ideal ones, weighted by each
 * grid point's total texel contribution (O(weight_count)). The projection
 * residual and line length are identical across all candidates of one
 * endpoint line, so this ranks candidates correctly at a fraction of the
 * cost; only the shortlisted candidates get exact scoring. */
/* SSE2: accumulate 4 candidates' grid weights from one texel's data.
 * ideal[i] is uint8_t (0-64), contrib is uint8_t (0-16), product fits int16. */
/* Correct per-candidate grid accumulation for 1–4 candidates.
 * SSE2 path uses SIMD for the 4-wide multiply but accumulates to the
 * correct stride-256-separated rows (wa[ci][idx] not wa[0][idx+ci]).
 * Used only when the batch size is 2–4; for n=1 the caller inlines the
 * scalar loop directly. */
static void tacd_accum4(const uint8_t *const ideal[4], uint32_t i,
                         const tc_astc_decim_cache_entry *decim,
                         uint32_t wa[4][64]) {
    uint32_t n2 = decim->tw_count[i], j;
    for (j = 0; j < n2; ++j) {
        uint32_t idx = decim->tw_idx[i][j];
        uint32_t cb = decim->tw_contrib[i][j];
        wa[0][idx] += (uint32_t)ideal[0][i] * cb;
        wa[1][idx] += (uint32_t)ideal[1][i] * cb;
        wa[2][idx] += (uint32_t)ideal[2][i] * cb;
        wa[3][idx] += (uint32_t)ideal[3][i] * cb;
    }
}

static uint64_t tc_astc_infill_recon_scalar(const uint8_t *grid_wt,
                                             const tc_astc_decim_cache_entry *decim,
                                             uint32_t count,
                                             const uint8_t *ideal) {
    uint64_t err = 0;
    uint32_t i;
    for (i = 0; i < count; ++i) {
        const uint8_t *idx = decim->tw_idx[i];
        const uint8_t *cbp = decim->tw_contrib[i];
        uint32_t w = (8u + grid_wt[idx[0u]] * cbp[0u] +
                      grid_wt[idx[1u]] * cbp[1u] +
                      grid_wt[idx[2u]] * cbp[2u] +
                      grid_wt[idx[3u]] * cbp[3u]) >> 4;
        int32_t d = (int32_t)w - (int32_t)ideal[i];
        err += (uint64_t)(d * d);
    }
    return err;
}

#if defined(TC_X86)
static uint64_t tc_astc_infill_recon_sse2(const uint8_t *grid_wt,
                                           const tc_astc_decim_cache_entry *decim,
                                           uint32_t count,
                                           const uint8_t *ideal) {
    const __m128i zero = _mm_setzero_si128();
    const __m128i v64_16 = _mm_set1_epi16(64);
    __m128i acc = zero;
    uint32_t i;
    for (i = 0; i + 2u <= count; i += 2u) {
        uint32_t g[8], cb[8];
        uint32_t t;
        for (t = 0; t < 2u; ++t) {
            const uint8_t *idx = decim->tw_idx[i + t];
            const uint8_t *cbp = decim->tw_contrib[i + t];
            g[t * 4u + 0u] = grid_wt[idx[0u]];
            g[t * 4u + 1u] = grid_wt[idx[1u]];
            g[t * 4u + 2u] = grid_wt[idx[2u]];
            g[t * 4u + 3u] = grid_wt[idx[3u]];
            cb[t * 4u + 0u] = cbp[0u];
            cb[t * 4u + 1u] = cbp[1u];
            cb[t * 4u + 2u] = cbp[2u];
            cb[t * 4u + 3u] = cbp[3u];
        }
        __m128i gv0 = _mm_set_epi16((int16_t)g[3], (int16_t)g[2],
                                     (int16_t)g[1], (int16_t)g[0],
                                     0, 0, 0, 0);
        __m128i cv0 = _mm_set_epi16((int16_t)cb[3], (int16_t)cb[2],
                                     (int16_t)cb[1], (int16_t)cb[0],
                                     0, 0, 0, 0);
        __m128i gv1 = _mm_set_epi16((int16_t)g[7], (int16_t)g[6],
                                     (int16_t)g[5], (int16_t)g[4],
                                     0, 0, 0, 0);
        __m128i cv1 = _mm_set_epi16((int16_t)cb[7], (int16_t)cb[6],
                                     (int16_t)cb[5], (int16_t)cb[4],
                                     0, 0, 0, 0);
        __m128i p0 = _mm_madd_epi16(gv0, cv0);
        __m128i p1 = _mm_madd_epi16(gv1, cv1);
        __m128i s0 = _mm_add_epi32(p0, _mm_shuffle_epi32(p0, _MM_SHUFFLE(1, 0, 3, 2)));
        __m128i s1 = _mm_add_epi32(p1, _mm_shuffle_epi32(p1, _MM_SHUFFLE(1, 0, 3, 2)));
        __m128i sum = _mm_unpacklo_epi32(s0, s1);
        __m128i w32 = _mm_srli_epi32(_mm_add_epi32(sum, _mm_set1_epi32(8)), 4);
        __m128i w16 = _mm_packs_epi32(w32, zero);
        w16 = _mm_min_epi16(_mm_max_epi16(w16, zero), v64_16);
        int16_t id0 = (int16_t)ideal[i];
        int16_t id1 = (int16_t)ideal[i + 1u];
        __m128i id16 = _mm_set_epi16(0, 0, 0, 0, 0, 0, id1, id0);
        __m128i d = _mm_sub_epi16(w16, id16);
        __m128i err = _mm_madd_epi16(d, d);
        acc = _mm_add_epi32(acc, err);
    }
    {
        uint32_t lanes[4];
        _mm_storeu_si128((__m128i *)lanes, acc);
        uint64_t err = (uint64_t)lanes[0];
        for (; i < count; ++i) {
            const uint8_t *idx = decim->tw_idx[i];
            const uint8_t *cbp = decim->tw_contrib[i];
            uint32_t w = (8u + grid_wt[idx[0u]] * cbp[0u] +
                          grid_wt[idx[1u]] * cbp[1u] +
                          grid_wt[idx[2u]] * cbp[2u] +
                          grid_wt[idx[3u]] * cbp[3u]) >> 4;
            int32_t d = (int32_t)w - (int32_t)ideal[i];
            err += (uint64_t)(d * d);
        }
        return err;
    }
}
#endif

#if defined(__AVX2__)
/* 8-wide batch accumulation using AVX2 gather. Processes 8 candidates
 * at once, gathering their scattered accumulators at stride 256 (64×4)
 * and writing back individually (AVX2 lacks scatter). */
TC_TARGET("avx2")
static void tacd_accum8_avx2(const uint8_t *const ideal[8], uint32_t i,
                              const tc_astc_decim_cache_entry *decim,
                              uint32_t wa[8][64]) {
    uint32_t n2 = decim->tw_count[i], j;
    const int *base = (const int *)wa;
    for (j = 0; j < n2; ++j) {
        uint32_t idx = decim->tw_idx[i][j];
        uint32_t cb = decim->tw_contrib[i][j];
        __m256i vprod = _mm256_set1_epi32((int)cb);
        __m256i vi = _mm256_set_epi32((int)ideal[7][i], (int)ideal[6][i],
                                      (int)ideal[5][i], (int)ideal[4][i],
                                      (int)ideal[3][i], (int)ideal[2][i],
                                      (int)ideal[1][i], (int)ideal[0][i]);
        vprod = _mm256_mullo_epi32(vprod, vi);
        __m256i vgidx = _mm256_set_epi32((int)(idx + 7*64), (int)(idx + 6*64),
                                         (int)(idx + 5*64), (int)(idx + 4*64),
                                         (int)(idx + 3*64), (int)(idx + 2*64),
                                         (int)(idx + 1*64), (int)(idx + 0*64));
        __m256i vacc = _mm256_i32gather_epi32(base, vgidx, 4);
        vacc = _mm256_add_epi32(vacc, vprod);
        {   int temp[8];
            _mm256_storeu_si256((__m256i *)temp, vacc);
            wa[0][idx] = (uint32_t)temp[0];
            wa[1][idx] = (uint32_t)temp[1];
            wa[2][idx] = (uint32_t)temp[2];
            wa[3][idx] = (uint32_t)temp[3];
            wa[4][idx] = (uint32_t)temp[4];
            wa[5][idx] = (uint32_t)temp[5];
            wa[6][idx] = (uint32_t)temp[6];
            wa[7][idx] = (uint32_t)temp[7];
        }
    }
}
#endif



/* Batch variant: score `n` candidates that share the same decim entry and quant
 * method. Uses multi-accumulator SIMD to process all candidates' grid
 * accumulations in a single texel pass (shared decim lookups, parallel adds). */
static void tc_astc_score_batch(const tc_astc_encode_context *ctx, uint32_t count,
                                const uint8_t *const ideal_batch[], uint32_t n,
                                uint32_t quant_method,
                                const tc_astc_decim_cache_entry *decim,
                                uint32_t weight_count, uint64_t out_errs[]) {
    const uint8_t *lut_wt = ctx->wq_wt[quant_method];
    uint32_t bi;
    if (decim->direct) {
        const uint16_t *lut_err = ctx->wq_err[quant_method];
        for (bi = 0; bi < n; ++bi) {
            const uint8_t *ideal = ideal_batch[bi];
            uint32_t i;
            uint64_t err = 0;
            for (i = 0; i < count; ++i) err += lut_err[ideal[i]];
            out_errs[bi] = err << 4;
        }
        return;
    }
#if defined(__AVX2__)
    /* AVX2 batch: up to 8 candidates with gather-based accumulation. */
    if (n >= 2u && n <= 8u && (tc_cpu_caps() & TC_CPU_AVX2)) {
        uint32_t wa[8][64];
        uint8_t grid_wt[8][64];
        uint32_t i;
        memset(wa, 0, sizeof(wa));
        for (i = 0; i < count; ++i)
            tacd_accum8_avx2(ideal_batch, i, decim, wa);
        for (bi = 0; bi < n; ++bi) {
            uint64_t err;
            for (i = 0; i < weight_count; ++i) {
                uint32_t contrib = decim->weight_contrib[i];
                uint32_t g = contrib ? (wa[bi][i] + contrib / 2u) / contrib : 0u;
                grid_wt[bi][i] = lut_wt[g];
            }
            err = tc_astc_infill_recon_avx2(grid_wt[bi], decim, count,
                                             ideal_batch[bi]);
            out_errs[bi] = err << 4;
        }
        return;
    }
#endif
    /* SSE2/scalar batch: up to 4 candidates. */
    if (n >= 2u && n <= 4u) {
        uint32_t wa[4][64];
        uint8_t grid_wt[4][64];
        uint32_t i;
        memset(wa, 0, sizeof(wa));
        for (i = 0; i < count; ++i)
            tacd_accum4(ideal_batch, i, decim, wa);
        for (bi = 0; bi < n; ++bi) {
            uint64_t err;
            for (i = 0; i < weight_count; ++i) {
                uint32_t contrib = decim->weight_contrib[i];
                uint32_t g = contrib ? (wa[bi][i] + contrib / 2u) / contrib : 0u;
                grid_wt[bi][i] = lut_wt[g];
            }
#if defined(TC_X86)
            if (tc_cpu_caps() & (TC_CPU_SSE2 | TC_CPU_SSE41))
                err = tc_astc_infill_recon_sse2(grid_wt[bi], decim, count,
                                                 ideal_batch[bi]);
            else
#endif
                err = tc_astc_infill_recon_scalar(grid_wt[bi], decim, count,
                                                   ideal_batch[bi]);
            out_errs[bi] = err << 4;
        }
        return;
    }
    for (bi = 0; bi < n; ++bi) {
        const uint8_t *ideal = ideal_batch[bi];
        uint64_t err;
        uint32_t weight_accum[64];
        uint8_t grid_wt[64];
        uint32_t i;
        for (i = 0; i < weight_count; ++i) weight_accum[i] = 0;
        for (i = 0; i < count; ++i) {
            uint32_t n2 = decim->tw_count[i], j;
            for (j = 0; j < n2; ++j)
                weight_accum[decim->tw_idx[i][j]] += (uint32_t)ideal[i] * decim->tw_contrib[i][j];
        }
        for (i = 0; i < weight_count; ++i) {
            uint32_t contrib = decim->weight_contrib[i];
            uint32_t g = contrib ? (weight_accum[i] + contrib / 2u) / contrib : 0u;
            grid_wt[i] = lut_wt[g];
        }
        err = tc_astc_infill_recon_scalar(grid_wt, decim, count, ideal);
        out_errs[bi] = err << 4;
    }
}

static uint64_t tc_astc_score_from_ideal_all(
    const tc_astc_encode_context *ctx, uint32_t count,
    const uint8_t ideal[144], uint32_t quant_method,
    const tc_astc_decim_cache_entry *decim, uint32_t weight_count) {
    uint64_t err;
    tc_astc_score_batch(ctx, count, &ideal, 1, quant_method, decim, weight_count, &err);
    return err;
}

static uint64_t tc_astc_score_from_ideal(const tc_astc_encode_context *ctx,
                                         uint32_t count,
                                         const uint8_t ideal[144],
                                         const uint8_t *active,
                                         uint32_t quant_method,
                                         const tc_astc_decim_cache_entry *decim,
                                         uint32_t weight_count) {
    const uint8_t *lut_wt = ctx->wq_wt[quant_method];
    uint32_t i;
    uint64_t err = 0;
    if (!active)
        return tc_astc_score_from_ideal_all(ctx, count, ideal, quant_method,
                                            decim, weight_count);
    if (decim->direct) {
        const uint16_t *lut_err = ctx->wq_err[quant_method];
        for (i = 0; i < count; ++i) {
            if (!active[i]) continue;
            err += lut_err[ideal[i]];
        }
        return err << 4; /* keep both paths on one scale */
    }
    {
        uint32_t weight_accum[64];
        uint32_t weight_contrib[64];
        uint8_t grid[64];
        for (i = 0; i < weight_count; ++i) {
            weight_accum[i] = 0;
            weight_contrib[i] = 0;
        }
        for (i = 0; i < count; ++i) {
            uint32_t n = decim->tw_count[i], j;
            if (!active[i]) continue;
            for (j = 0; j < n; ++j) {
                weight_accum[decim->tw_idx[i][j]] +=
                    (uint32_t)ideal[i] * decim->tw_contrib[i][j];
                weight_contrib[decim->tw_idx[i][j]] += decim->tw_contrib[i][j];
            }
        }
        for (i = 0; i < weight_count; ++i) {
            uint32_t g = weight_contrib[i]
                             ? (weight_accum[i] + weight_contrib[i] / 2u) /
                                   weight_contrib[i]
                             : 0u;
            grid[i] = lut_wt[g];
        }
        for (i = 0; i < count; ++i) {
            const uint8_t *idx = decim->tw_idx[i];
            const uint8_t *cb = decim->tw_contrib[i];
            uint32_t w;
            int32_t d;
            if (!active[i]) continue;
            w = (8u + grid[idx[0]] * cb[0] + grid[idx[1]] * cb[1] +
                 grid[idx[2]] * cb[2] + grid[idx[3]] * cb[3]) >>
                4;
            d = (int32_t)w - (int32_t)ideal[i];
            err += (uint64_t)(d * d);
        }
    }
    return err << 4;
}

/* Quantizes precomputed per-texel ideal weights onto a candidate's weight
 * grid and returns the reconstruction SSE against lo/hi. The projection is
 * hoisted out because the ideal weights depend only on the endpoint line,
 * not on the candidate's grid or quantization. */
static void tc_astc_weights_from_ideal(const tc_astc_encode_context *ctx,
                                       uint32_t count,
                                       const uint8_t ideal[144],
                                       uint32_t quant_method,
                                       const tc_astc_decim_cache_entry *decim,
                                       uint32_t weight_count,
                                       uint8_t out_weights[64],
                                       uint8_t wt[144]) {
    const uint8_t *lut_q = ctx->wq_q[quant_method];
    const uint8_t *lut_wt = ctx->wq_wt[quant_method];
    uint32_t i;

    if (decim->direct) {
        for (i = 0; i < count; ++i) {
            out_weights[i] = lut_q[ideal[i]];
            wt[i] = lut_wt[ideal[i]];
        }
    } else {
        uint32_t weight_accum[64];
        uint8_t grid[64];
        for (i = 0; i < weight_count; ++i) {
            weight_accum[i] = 0;
        }
#if defined(__AVX2__)
        if (tc_cpu_caps() & TC_CPU_AVX2) {
            tc_astc_accum_weights_avx2(weight_accum, decim, count, ideal);
        } else
#endif
        {
            for (i = 0; i < count; ++i) {
                uint32_t n = decim->tw_count[i], j;
                for (j = 0; j < n; ++j) {
                    weight_accum[decim->tw_idx[i][j]] +=
                        (uint32_t)ideal[i] * decim->tw_contrib[i][j];
                }
            }
        }
        for (i = 0; i < weight_count; ++i) {
            uint32_t contrib = decim->weight_contrib[i];
            uint32_t v = contrib ? (weight_accum[i] + contrib / 2u) / contrib
                                 : 0u;
            out_weights[i] = lut_q[v];
            grid[i] = lut_wt[v];
        }
        tc_astc_infill_weights_grid(grid, decim, count, wt);
    }
}

static uint64_t tc_astc_fit_from_ideal(const tc_astc_encode_context *ctx,
                                       const uint8_t block[144][4],
                                       uint32_t count,
                                       const uint8_t ideal[144],
                                       const uint32_t lo[4],
                                       const uint32_t hi[4],
                                       uint32_t quant_method,
                                       const tc_astc_decim_cache_entry *decim,
                                       uint32_t weight_count,
                                       uint8_t out_weights[64],
                                       uint8_t out_wt[144]) {
    uint8_t local_wt[144];
    uint8_t *wt = out_wt ? out_wt : local_wt;
    tc_astc_weights_from_ideal(ctx, count, ideal, quant_method, decim,
                               weight_count, out_weights, wt);
    return tc_astc_recon_sse(block, count, lo, hi, wt);
}


static void tc_astc_scramble_weights(uint8_t *weights, uint32_t count,
                                     uint32_t quant_method) {
    uint32_t i;
    for (i = 0; i < count; ++i)
        weights[i] = tc_astc_weight_scramble(weights[i], quant_method);
}

/* Inverts unscrambled quantized weights (q -> maxq - q). The unquantized
 * weight tables are symmetric, so this reproduces exactly the mirrored
 * interpolation needed when endpoint pairs are swapped. */
static void tc_astc_invert_weights(uint8_t *weights, uint32_t count,
                                   uint32_t quant_method) {
    uint32_t i, maxq = tc_astc_quant_levels(quant_method) - 1u;
    for (i = 0; i < count; ++i)
        weights[i] = (uint8_t)(maxq - weights[i]);
}

/* Signed rounded division, den > 0. Avoids C's truncation-toward-zero
 * asymmetry so the integer least-squares solve matches the real solution
 * within one step. */
static int64_t tc_div_round_s64(int64_t num, int64_t den) {
    int64_t half = den / 2;
    if (num >= 0) return (num + half) / den;
    return -((-num + half) / den);
}

/* Expands the quantized decimated weight grid into per-texel interpolated
 * weights [0,64]. */
static void tc_astc_infill_weights(const uint8_t *weights,
                                   const tc_astc_decim_cache_entry *decim,
                                   uint32_t count, uint32_t quant_method,
                                   uint8_t wt[144]) {
    uint8_t grid[64];
    uint32_t i, n;
    if (decim->direct) {
        for (i = 0; i < count; ++i)
            wt[i] = (uint8_t)tc_astc_weight_unquant(weights[i], quant_method);
        return;
    }
    /* Unquantize the (small) weight grid once, then the per-texel infill
     * is four unconditional multiply-adds over bytes. */
    n = (uint32_t)decim->weight_x * decim->weight_y;
    for (i = 0; i < n; ++i)
        grid[i] = (uint8_t)tc_astc_weight_unquant(weights[i], quant_method);
    for (i = 0; i < count; ++i) {
        const uint8_t *idx = decim->tw_idx[i];
        const uint8_t *cb = decim->tw_contrib[i];
        uint32_t sum = 8u + grid[idx[0]] * cb[0] + grid[idx[1]] * cb[1] +
                       grid[idx[2]] * cb[2] + grid[idx[3]] * cb[3];
        wt[i] = (uint8_t)(sum >> 4);
    }
}

static void tc_astc_infill_weights_grid(const uint8_t grid[64],
                                        const tc_astc_decim_cache_entry *decim,
                                        uint32_t count, uint8_t wt[144]) {
    uint32_t i;
    for (i = 0; i < count; ++i) {
        const uint8_t *idx = decim->tw_idx[i];
        const uint8_t *cb = decim->tw_contrib[i];
        uint32_t sum = 8u + grid[idx[0]] * cb[0] + grid[idx[1]] * cb[1] +
                       grid[idx[2]] * cb[2] + grid[idx[3]] * cb[3];
        wt[i] = (uint8_t)(sum >> 4);
    }
}

/* Least-squares endpoint solve: given the per-texel interpolated weights,
 * finds the endpoint pair minimizing the sum of squared errors of
 * lerp(lo, hi, w/64) per channel (2x2 normal equations). Returns 0 when the
 * system is degenerate (all weights equal). Magnitude bound: every term is
 * at most 64 * (144*64*255) * (144*64^2) < 2^47, safely inside int64. */
#if defined(TC_X86)
/* SIMD accumulation of the normal-equation sums. All partial sums fit
 * 32-bit lanes: quadratic terms are at most 144*64^2 < 2^20 and the
 * mixed terms at most 144*64*255 < 2^22. Integer-exact vs the scalar
 * loop, so results stay byte-identical across backends. */
TC_TARGET("sse2")
static void tc_astc_lsq_sums_sse2(const uint8_t block[144][4], uint32_t count,
                                  const uint8_t wt[144], int64_t *out_saa,
                                  int64_t *out_sab, int64_t *out_sbb,
                                  int64_t sap[4], int64_t sbp[4]) {
    const __m128i zero = _mm_setzero_si128();
    const __m128i v64 = _mm_set1_epi16(64);
    __m128i acc_aa = zero, acc_ab = zero, acc_bb = zero;
    __m128i acc_ap02 = zero, acc_ap13 = zero;
    __m128i acc_bp02 = zero, acc_bp13 = zero;
    uint32_t i = 0, c;
    for (; i + 8u <= count; i += 8u) {
        __m128i b16 = _mm_unpacklo_epi8(
            _mm_loadl_epi64((const __m128i *)(wt + i)), zero);
        __m128i a16 = _mm_sub_epi16(v64, b16);
        uint32_t t;
        acc_aa = _mm_add_epi32(acc_aa, _mm_madd_epi16(a16, a16));
        acc_ab = _mm_add_epi32(acc_ab, _mm_madd_epi16(a16, b16));
        acc_bb = _mm_add_epi32(acc_bb, _mm_madd_epi16(b16, b16));
        for (t = 0; t < 8u; t += 2u) {
            __m128i px = _mm_unpacklo_epi8(
                _mm_loadl_epi64((const __m128i *)block[i + t]), zero);
            /* [w 0 w 0 w' 0 w' 0] against [r g b a r' g' b' a'] makes
             * madd produce per-channel products in separate lanes. */
            __m128i bw = _mm_set_epi16(0, (int16_t)wt[i + t + 1u], 0,
                                       (int16_t)wt[i + t + 1u], 0,
                                       (int16_t)wt[i + t], 0,
                                       (int16_t)wt[i + t]);
            __m128i aw = _mm_sub_epi16(_mm_set_epi16(0, 64, 0, 64, 0, 64, 0, 64),
                                       bw);
            __m128i bw_sh = _mm_slli_si128(bw, 2);
            __m128i aw_sh = _mm_slli_si128(aw, 2);
            acc_ap02 = _mm_add_epi32(acc_ap02, _mm_madd_epi16(px, aw));
            acc_ap13 = _mm_add_epi32(acc_ap13, _mm_madd_epi16(px, aw_sh));
            acc_bp02 = _mm_add_epi32(acc_bp02, _mm_madd_epi16(px, bw));
            acc_bp13 = _mm_add_epi32(acc_bp13, _mm_madd_epi16(px, bw_sh));
        }
    }
    {
        int32_t l_aa[4], l_ab[4], l_bb[4], l_ap02[4], l_ap13[4], l_bp02[4],
            l_bp13[4];
        _mm_storeu_si128((__m128i *)l_aa, acc_aa);
        _mm_storeu_si128((__m128i *)l_ab, acc_ab);
        _mm_storeu_si128((__m128i *)l_bb, acc_bb);
        _mm_storeu_si128((__m128i *)l_ap02, acc_ap02);
        _mm_storeu_si128((__m128i *)l_ap13, acc_ap13);
        _mm_storeu_si128((__m128i *)l_bp02, acc_bp02);
        _mm_storeu_si128((__m128i *)l_bp13, acc_bp13);
        *out_saa = (int64_t)l_aa[0] + l_aa[1] + l_aa[2] + l_aa[3];
        *out_sab = (int64_t)l_ab[0] + l_ab[1] + l_ab[2] + l_ab[3];
        *out_sbb = (int64_t)l_bb[0] + l_bb[1] + l_bb[2] + l_bb[3];
        sap[0] = (int64_t)l_ap02[0] + l_ap02[2];
        sap[2] = (int64_t)l_ap02[1] + l_ap02[3];
        sap[1] = (int64_t)l_ap13[0] + l_ap13[2];
        sap[3] = (int64_t)l_ap13[1] + l_ap13[3];
        sbp[0] = (int64_t)l_bp02[0] + l_bp02[2];
        sbp[2] = (int64_t)l_bp02[1] + l_bp02[3];
        sbp[1] = (int64_t)l_bp13[0] + l_bp13[2];
        sbp[3] = (int64_t)l_bp13[1] + l_bp13[3];
    }
    for (; i < count; ++i) {
        int64_t a = 64 - wt[i];
        int64_t b = wt[i];
        *out_saa += a * a;
        *out_sab += a * b;
        *out_sbb += b * b;
        for (c = 0; c < 4u; ++c) {
            sap[c] += a * block[i][c];
            sbp[c] += b * block[i][c];
        }
    }
}

TC_TARGET("sse2")
static void tc_astc_lsq_sums_partition_sse2(
    const uint8_t block[144][4], uint32_t count, const uint8_t partmap[144],
    uint32_t part, const uint8_t wt[144], int64_t *out_saa, int64_t *out_sab,
    int64_t *out_sbb, int64_t sap[4], int64_t sbp[4]) {
    const __m128i zero = _mm_setzero_si128();
    const __m128i v64 = _mm_set1_epi16(64);
    const __m128i vpart = _mm_set1_epi8((char)part);
    __m128i acc_aa = zero, acc_ab = zero, acc_bb = zero;
    __m128i acc_ap02 = zero, acc_ap13 = zero;
    __m128i acc_bp02 = zero, acc_bp13 = zero;
    uint32_t i = 0, c;
    for (; i + 8u <= count; i += 8u) {
        __m128i active8 = _mm_cmpeq_epi8(
            _mm_loadl_epi64((const __m128i *)(partmap + i)), vpart);
        __m128i active16 = _mm_unpacklo_epi8(active8, active8);
        __m128i b16_all = _mm_unpacklo_epi8(
            _mm_loadl_epi64((const __m128i *)(wt + i)), zero);
        __m128i b16 = _mm_and_si128(b16_all, active16);
        __m128i a16 = _mm_and_si128(_mm_sub_epi16(v64, b16_all), active16);
        uint32_t t;
        acc_aa = _mm_add_epi32(acc_aa, _mm_madd_epi16(a16, a16));
        acc_ab = _mm_add_epi32(acc_ab, _mm_madd_epi16(a16, b16));
        acc_bb = _mm_add_epi32(acc_bb, _mm_madd_epi16(b16, b16));
        for (t = 0; t < 8u; t += 2u) {
            uint32_t active0 = partmap[i + t] == part;
            uint32_t active1 = partmap[i + t + 1u] == part;
            int16_t b0 = active0 ? (int16_t)wt[i + t] : 0;
            int16_t b1 = active1 ? (int16_t)wt[i + t + 1u] : 0;
            int16_t a0 = active0 ? (int16_t)(64u - wt[i + t]) : 0;
            int16_t a1 = active1 ? (int16_t)(64u - wt[i + t + 1u]) : 0;
            __m128i px = _mm_unpacklo_epi8(
                _mm_loadl_epi64((const __m128i *)block[i + t]), zero);
            __m128i bw = _mm_set_epi16(0, b1, 0, b1, 0, b0, 0, b0);
            __m128i aw = _mm_set_epi16(0, a1, 0, a1, 0, a0, 0, a0);
            __m128i bw_sh = _mm_slli_si128(bw, 2);
            __m128i aw_sh = _mm_slli_si128(aw, 2);
            acc_ap02 = _mm_add_epi32(acc_ap02, _mm_madd_epi16(px, aw));
            acc_ap13 = _mm_add_epi32(acc_ap13, _mm_madd_epi16(px, aw_sh));
            acc_bp02 = _mm_add_epi32(acc_bp02, _mm_madd_epi16(px, bw));
            acc_bp13 = _mm_add_epi32(acc_bp13, _mm_madd_epi16(px, bw_sh));
        }
    }
    {
        int32_t l_aa[4], l_ab[4], l_bb[4], l_ap02[4], l_ap13[4], l_bp02[4],
            l_bp13[4];
        _mm_storeu_si128((__m128i *)l_aa, acc_aa);
        _mm_storeu_si128((__m128i *)l_ab, acc_ab);
        _mm_storeu_si128((__m128i *)l_bb, acc_bb);
        _mm_storeu_si128((__m128i *)l_ap02, acc_ap02);
        _mm_storeu_si128((__m128i *)l_ap13, acc_ap13);
        _mm_storeu_si128((__m128i *)l_bp02, acc_bp02);
        _mm_storeu_si128((__m128i *)l_bp13, acc_bp13);
        *out_saa = (int64_t)l_aa[0] + l_aa[1] + l_aa[2] + l_aa[3];
        *out_sab = (int64_t)l_ab[0] + l_ab[1] + l_ab[2] + l_ab[3];
        *out_sbb = (int64_t)l_bb[0] + l_bb[1] + l_bb[2] + l_bb[3];
        sap[0] = (int64_t)l_ap02[0] + l_ap02[2];
        sap[2] = (int64_t)l_ap02[1] + l_ap02[3];
        sap[1] = (int64_t)l_ap13[0] + l_ap13[2];
        sap[3] = (int64_t)l_ap13[1] + l_ap13[3];
        sbp[0] = (int64_t)l_bp02[0] + l_bp02[2];
        sbp[2] = (int64_t)l_bp02[1] + l_bp02[3];
        sbp[1] = (int64_t)l_bp13[0] + l_bp13[2];
        sbp[3] = (int64_t)l_bp13[1] + l_bp13[3];
    }
    for (; i < count; ++i) {
        int64_t a, b;
        if (partmap[i] != part) continue;
        a = 64 - wt[i];
        b = wt[i];
        *out_saa += a * a;
        *out_sab += a * b;
        *out_sbb += b * b;
        for (c = 0; c < 4u; ++c) {
            sap[c] += a * block[i][c];
            sbp[c] += b * block[i][c];
        }
    }
}
#endif /* TC_X86 */

#if defined(__ARM_NEON) || defined(__ARM_NEON__)
static void tc_astc_lsq_sums_neon(const uint8_t block[144][4], uint32_t count,
                                  const uint8_t wt[144], int64_t *out_saa,
                                  int64_t *out_sab, int64_t *out_sbb,
                                  int64_t sap[4], int64_t sbp[4]) {
    const uint16x8_t v64 = vdupq_n_u16(64);
    uint32x4_t acc_aa = vdupq_n_u32(0), acc_ab = vdupq_n_u32(0);
    uint32x4_t acc_bb = vdupq_n_u32(0), acc_ap = vdupq_n_u32(0);
    uint32x4_t acc_bp = vdupq_n_u32(0);
    uint32_t i = 0, c;
    for (; i + 8u <= count; i += 8u) {
        uint16x8_t b16 = vmovl_u8(vld1_u8(wt + i));
        uint16x8_t a16 = vsubq_u16(v64, b16);
        uint32_t t;
        acc_aa = vaddq_u32(acc_aa,
                            vmull_u16(vget_low_u16(a16), vget_low_u16(a16)));
        acc_aa = vaddq_u32(acc_aa,
                            vmull_u16(vget_high_u16(a16), vget_high_u16(a16)));
        acc_ab = vaddq_u32(acc_ab,
                            vmull_u16(vget_low_u16(a16), vget_low_u16(b16)));
        acc_ab = vaddq_u32(acc_ab,
                            vmull_u16(vget_high_u16(a16), vget_high_u16(b16)));
        acc_bb = vaddq_u32(acc_bb,
                            vmull_u16(vget_low_u16(b16), vget_low_u16(b16)));
        acc_bb = vaddq_u32(acc_bb,
                            vmull_u16(vget_high_u16(b16), vget_high_u16(b16)));
        for (t = 0; t < 8u; t += 2u) {
            uint16_t a0 = (uint16_t)(64u - wt[i + t]);
            uint16_t a1 = (uint16_t)(64u - wt[i + t + 1u]);
            uint16_t b0 = wt[i + t];
            uint16_t b1 = wt[i + t + 1u];
            uint16x8_t px = vmovl_u8(vld1_u8((const uint8_t *)block[i + t]));
            uint16x8_t av = vcombine_u16(vdup_n_u16(a0), vdup_n_u16(a1));
            uint16x8_t bv = vcombine_u16(vdup_n_u16(b0), vdup_n_u16(b1));
            uint32x4_t ap0 = vmull_u16(vget_low_u16(px), vget_low_u16(av));
            uint32x4_t ap1 = vmull_u16(vget_high_u16(px), vget_high_u16(av));
            uint32x4_t bp0 = vmull_u16(vget_low_u16(px), vget_low_u16(bv));
            uint32x4_t bp1 = vmull_u16(vget_high_u16(px), vget_high_u16(bv));
            acc_ap = vaddq_u32(acc_ap, vaddq_u32(ap0, ap1));
            acc_bp = vaddq_u32(acc_bp, vaddq_u32(bp0, bp1));
        }
    }
    *out_saa = (int64_t)vaddvq_u32(acc_aa);
    *out_sab = (int64_t)vaddvq_u32(acc_ab);
    *out_sbb = (int64_t)vaddvq_u32(acc_bb);
    {
        uint32_t ap_lanes[4], bp_lanes[4];
        vst1q_u32(ap_lanes, acc_ap);
        vst1q_u32(bp_lanes, acc_bp);
        for (c = 0; c < 4u; ++c) {
            sap[c] = (int64_t)ap_lanes[c];
            sbp[c] = (int64_t)bp_lanes[c];
        }
    }
    for (; i < count; ++i) {
        int64_t a = 64 - wt[i];
        int64_t b = wt[i];
        *out_saa += a * a;
        *out_sab += a * b;
        *out_sbb += b * b;
        for (c = 0; c < 4u; ++c) {
            sap[c] += a * block[i][c];
            sbp[c] += b * block[i][c];
        }
    }
}
#endif /* NEON */

#if defined(__AVX2__)
/* AVX2 LSQ sums: 16-wide weight accumulation, 4-texel inner batches.
 * Same integer-exact semantics as the SSE2 and scalar paths. */
TC_TARGET("avx2")
static void tc_astc_lsq_sums_avx2(const uint8_t block[144][4], uint32_t count,
                                   const uint8_t wt[144], int64_t *out_saa,
                                   int64_t *out_sab, int64_t *out_sbb,
                                   int64_t sap[4], int64_t sbp[4]) {
    const __m256i zero256 = _mm256_setzero_si256();
    const __m128i zero128 = _mm_setzero_si128();
    const __m256i v64 = _mm256_set1_epi16(64);
    __m256i acc_aa = zero256, acc_ab = zero256, acc_bb = zero256;
    __m256i acc_ap02 = zero256, acc_ap13 = zero256;
    __m256i acc_bp02 = zero256, acc_bp13 = zero256;
    uint32_t i = 0;
    for (; i + 16u <= count; i += 16u) {
        __m256i b16 = _mm256_cvtepu8_epi16(
            _mm_loadu_si128((const __m128i *)(wt + i)));
        __m256i a16 = _mm256_sub_epi16(v64, b16);
        acc_aa = _mm256_add_epi32(acc_aa, _mm256_madd_epi16(a16, a16));
        acc_ab = _mm256_add_epi32(acc_ab, _mm256_madd_epi16(a16, b16));
        acc_bb = _mm256_add_epi32(acc_bb, _mm256_madd_epi16(b16, b16));
        uint32_t t;
        for (t = 0; t < 16u; t += 4u) {
            __m128i p0 = _mm_loadu_si128((const __m128i *)block[i + t]);
            __m128i p1 = _mm_loadu_si128((const __m128i *)block[i + t + 2u]);
            __m128i px0 = _mm_unpacklo_epi8(p0, zero128);
            __m128i px1 = _mm_unpacklo_epi8(p1, zero128);
            int16_t b0 = (int16_t)wt[i + t], a0 = (int16_t)(64u - wt[i + t]);
            int16_t b1 = (int16_t)wt[i + t + 1u], a1 = (int16_t)(64u - wt[i + t + 1u]);
            int16_t b2 = (int16_t)wt[i + t + 2u], a2 = (int16_t)(64u - wt[i + t + 2u]);
            int16_t b3 = (int16_t)wt[i + t + 3u], a3 = (int16_t)(64u - wt[i + t + 3u]);
            __m128i bw0 = _mm_set_epi16(0, b1, 0, b1, 0, b0, 0, b0);
            __m128i aw0 = _mm_set_epi16(0, a1, 0, a1, 0, a0, 0, a0);
            __m128i bw1 = _mm_set_epi16(0, b3, 0, b3, 0, b2, 0, b2);
            __m128i aw1 = _mm_set_epi16(0, a3, 0, a3, 0, a2, 0, a2);
            __m128i bw0_sh = _mm_slli_si128(bw0, 2);
            __m128i aw0_sh = _mm_slli_si128(aw0, 2);
            __m128i bw1_sh = _mm_slli_si128(bw1, 2);
            __m128i aw1_sh = _mm_slli_si128(aw1, 2);
            /* Accumulate into the LO and HI halves of the 256-bit vector */
            __m256i ap02 = _mm256_set_m128i(
                _mm_madd_epi16(px1, aw1), _mm_madd_epi16(px0, aw0));
            __m256i ap13 = _mm256_set_m128i(
                _mm_madd_epi16(px1, aw1_sh), _mm_madd_epi16(px0, aw0_sh));
            __m256i bp02 = _mm256_set_m128i(
                _mm_madd_epi16(px1, bw1), _mm_madd_epi16(px0, bw0));
            __m256i bp13 = _mm256_set_m128i(
                _mm_madd_epi16(px1, bw1_sh), _mm_madd_epi16(px0, bw0_sh));
            acc_ap02 = _mm256_add_epi32(acc_ap02, ap02);
            acc_ap13 = _mm256_add_epi32(acc_ap13, ap13);
            acc_bp02 = _mm256_add_epi32(acc_bp02, bp02);
            acc_bp13 = _mm256_add_epi32(acc_bp13, bp13);
        }
    }
    {   int32_t l_aa[8], l_ab[8], l_bb[8];
        int32_t l_ap02[8], l_ap13[8], l_bp02[8], l_bp13[8];
        _mm256_storeu_si256((__m256i *)l_aa, acc_aa);
        _mm256_storeu_si256((__m256i *)l_ab, acc_ab);
        _mm256_storeu_si256((__m256i *)l_bb, acc_bb);
        _mm256_storeu_si256((__m256i *)l_ap02, acc_ap02);
        _mm256_storeu_si256((__m256i *)l_ap13, acc_ap13);
        _mm256_storeu_si256((__m256i *)l_bp02, acc_bp02);
        _mm256_storeu_si256((__m256i *)l_bp13, acc_bp13);
        uint32_t j;
        int64_t t_aa = 0, t_ab = 0, t_bb = 0;
        for (j = 0; j < 8u; ++j) {
            t_aa += l_aa[j]; t_ab += l_ab[j]; t_bb += l_bb[j];
        }
        *out_saa = t_aa; *out_sab = t_ab; *out_sbb = t_bb;
        sap[0] = (int64_t)(l_ap02[0] + l_ap02[4] + l_ap02[2] + l_ap02[6]);
        sap[2] = (int64_t)(l_ap02[1] + l_ap02[5] + l_ap02[3] + l_ap02[7]);
        sap[1] = (int64_t)(l_ap13[0] + l_ap13[4] + l_ap13[2] + l_ap13[6]);
        sap[3] = (int64_t)(l_ap13[1] + l_ap13[5] + l_ap13[3] + l_ap13[7]);
        sbp[0] = (int64_t)(l_bp02[0] + l_bp02[4] + l_bp02[2] + l_bp02[6]);
        sbp[2] = (int64_t)(l_bp02[1] + l_bp02[5] + l_bp02[3] + l_bp02[7]);
        sbp[1] = (int64_t)(l_bp13[0] + l_bp13[4] + l_bp13[2] + l_bp13[6]);
        sbp[3] = (int64_t)(l_bp13[1] + l_bp13[5] + l_bp13[3] + l_bp13[7]);
    }
    for (; i < count; ++i) {
        int64_t a = 64 - wt[i];
        int64_t b = wt[i];
        *out_saa += a * a;
        *out_sab += a * b;
        *out_sbb += b * b;
        for (uint32_t c = 0; c < 4u; ++c) {
            sap[c] += a * block[i][c];
            sbp[c] += b * block[i][c];
        }
    }
}

TC_TARGET("avx2")
static void tc_astc_lsq_sums_partition_avx2(
    const uint8_t block[144][4], uint32_t count, const uint8_t partmap[144],
    uint32_t part, const uint8_t wt[144], int64_t *out_saa, int64_t *out_sab,
    int64_t *out_sbb, int64_t sap[4], int64_t sbp[4]) {
    const __m256i zero256 = _mm256_setzero_si256();
    const __m128i zero128 = _mm_setzero_si128();
    const __m256i v64 = _mm256_set1_epi16(64);
    const __m128i vpart = _mm_set1_epi8((char)part);
    __m256i acc_aa = zero256, acc_ab = zero256, acc_bb = zero256;
    __m256i acc_ap02 = zero256, acc_ap13 = zero256;
    __m256i acc_bp02 = zero256, acc_bp13 = zero256;
    uint32_t i = 0;
    for (; i + 16u <= count; i += 16u) {
        __m128i active = _mm_cmpeq_epi8(
            _mm_loadu_si128((const __m128i *)(partmap + i)), vpart);
        __m256i active16 = _mm256_cvtepu8_epi16(active);
        __m256i b16_all = _mm256_cvtepu8_epi16(
            _mm_loadu_si128((const __m128i *)(wt + i)));
        __m256i b16 = _mm256_and_si256(b16_all, active16);
        __m256i a16 = _mm256_and_si256(
            _mm256_sub_epi16(v64, b16_all), active16);
        acc_aa = _mm256_add_epi32(acc_aa, _mm256_madd_epi16(a16, a16));
        acc_ab = _mm256_add_epi32(acc_ab, _mm256_madd_epi16(a16, b16));
        acc_bb = _mm256_add_epi32(acc_bb, _mm256_madd_epi16(b16, b16));
        uint32_t t;
        for (t = 0; t < 16u; t += 4u) {
            uint32_t act0 = partmap[i + t] == part;
            uint32_t act1 = partmap[i + t + 1u] == part;
            uint32_t act2 = partmap[i + t + 2u] == part;
            uint32_t act3 = partmap[i + t + 3u] == part;
            int16_t b0 = act0 ? (int16_t)wt[i + t] : 0;
            int16_t b1 = act1 ? (int16_t)wt[i + t + 1u] : 0;
            int16_t b2 = act2 ? (int16_t)wt[i + t + 2u] : 0;
            int16_t b3 = act3 ? (int16_t)wt[i + t + 3u] : 0;
            int16_t a0 = act0 ? (int16_t)(64u - wt[i + t]) : 0;
            int16_t a1 = act1 ? (int16_t)(64u - wt[i + t + 1u]) : 0;
            int16_t a2 = act2 ? (int16_t)(64u - wt[i + t + 2u]) : 0;
            int16_t a3 = act3 ? (int16_t)(64u - wt[i + t + 3u]) : 0;
            __m128i p0 = _mm_loadu_si128((const __m128i *)block[i + t]);
            __m128i p1 = _mm_loadu_si128((const __m128i *)block[i + t + 2u]);
            __m128i px0 = _mm_unpacklo_epi8(p0, zero128);
            __m128i px1 = _mm_unpacklo_epi8(p1, zero128);
            __m128i bw0 = _mm_set_epi16(0, b1, 0, b1, 0, b0, 0, b0);
            __m128i aw0 = _mm_set_epi16(0, a1, 0, a1, 0, a0, 0, a0);
            __m128i bw1 = _mm_set_epi16(0, b3, 0, b3, 0, b2, 0, b2);
            __m128i aw1 = _mm_set_epi16(0, a3, 0, a3, 0, a2, 0, a2);
            __m128i bw0_sh = _mm_slli_si128(bw0, 2);
            __m128i aw0_sh = _mm_slli_si128(aw0, 2);
            __m128i bw1_sh = _mm_slli_si128(bw1, 2);
            __m128i aw1_sh = _mm_slli_si128(aw1, 2);
            __m256i ap02 = _mm256_set_m128i(
                _mm_madd_epi16(px1, aw1), _mm_madd_epi16(px0, aw0));
            __m256i ap13 = _mm256_set_m128i(
                _mm_madd_epi16(px1, aw1_sh), _mm_madd_epi16(px0, aw0_sh));
            __m256i bp02 = _mm256_set_m128i(
                _mm_madd_epi16(px1, bw1), _mm_madd_epi16(px0, bw0));
            __m256i bp13 = _mm256_set_m128i(
                _mm_madd_epi16(px1, bw1_sh), _mm_madd_epi16(px0, bw0_sh));
            acc_ap02 = _mm256_add_epi32(acc_ap02, ap02);
            acc_ap13 = _mm256_add_epi32(acc_ap13, ap13);
            acc_bp02 = _mm256_add_epi32(acc_bp02, bp02);
            acc_bp13 = _mm256_add_epi32(acc_bp13, bp13);
        }
    }
    {   int32_t l_aa[8], l_ab[8], l_bb[8];
        int32_t l_ap02[8], l_ap13[8], l_bp02[8], l_bp13[8];
        _mm256_storeu_si256((__m256i *)l_aa, acc_aa);
        _mm256_storeu_si256((__m256i *)l_ab, acc_ab);
        _mm256_storeu_si256((__m256i *)l_bb, acc_bb);
        _mm256_storeu_si256((__m256i *)l_ap02, acc_ap02);
        _mm256_storeu_si256((__m256i *)l_ap13, acc_ap13);
        _mm256_storeu_si256((__m256i *)l_bp02, acc_bp02);
        _mm256_storeu_si256((__m256i *)l_bp13, acc_bp13);
        uint32_t j;
        int64_t t_aa = 0, t_ab = 0, t_bb = 0;
        for (j = 0; j < 8u; ++j) {
            t_aa += l_aa[j]; t_ab += l_ab[j]; t_bb += l_bb[j];
        }
        *out_saa = t_aa; *out_sab = t_ab; *out_sbb = t_bb;
        sap[0] = (int64_t)(l_ap02[0] + l_ap02[4] + l_ap02[2] + l_ap02[6]);
        sap[2] = (int64_t)(l_ap02[1] + l_ap02[5] + l_ap02[3] + l_ap02[7]);
        sap[1] = (int64_t)(l_ap13[0] + l_ap13[4] + l_ap13[2] + l_ap13[6]);
        sap[3] = (int64_t)(l_ap13[1] + l_ap13[5] + l_ap13[3] + l_ap13[7]);
        sbp[0] = (int64_t)(l_bp02[0] + l_bp02[4] + l_bp02[2] + l_bp02[6]);
        sbp[2] = (int64_t)(l_bp02[1] + l_bp02[5] + l_bp02[3] + l_bp02[7]);
        sbp[1] = (int64_t)(l_bp13[0] + l_bp13[4] + l_bp13[2] + l_bp13[6]);
        sbp[3] = (int64_t)(l_bp13[1] + l_bp13[5] + l_bp13[3] + l_bp13[7]);
    }
    for (; i < count; ++i) {
        int64_t a, b;
        if (partmap[i] != part) continue;
        a = 64 - wt[i];
        b = wt[i];
        *out_saa += a * a;
        *out_sab += a * b;
        *out_sbb += b * b;
        for (uint32_t c = 0; c < 4u; ++c) {
            sap[c] += a * block[i][c];
            sbp[c] += b * block[i][c];
        }
    }
}
#endif /* __AVX2__ */

static int tc_astc_lsq_endpoints(const uint8_t block[144][4], uint32_t count,
                                 const uint8_t wt[144], uint32_t lo[4],
                                 uint32_t hi[4]) {
    int64_t saa = 0, sab = 0, sbb = 0, det;
    int64_t sap[4] = {0, 0, 0, 0}, sbp[4] = {0, 0, 0, 0};
    uint32_t i, c;
#if defined(TC_X86)
#if defined(__AVX2__)
    if (tc_cpu_caps() & TC_CPU_AVX2) {
        tc_astc_lsq_sums_avx2(block, count, wt, &saa, &sab, &sbb, sap, sbp);
    } else
#endif
    if (tc_cpu_caps() & (TC_CPU_SSE2 | TC_CPU_SSE41)) {
        tc_astc_lsq_sums_sse2(block, count, wt, &saa, &sab, &sbb, sap, sbp);
    } else
#endif
#if defined(__ARM_NEON) || defined(__ARM_NEON__)
    if (tc_cpu_caps() & TC_CPU_NEON) {
        tc_astc_lsq_sums_neon(block, count, wt, &saa, &sab, &sbb, sap, sbp);
    } else
#endif
    for (i = 0; i < count; ++i) {
        int64_t a = 64 - wt[i];
        int64_t b = wt[i];
        saa += a * a;
        sab += a * b;
        sbb += b * b;
        for (c = 0; c < 4u; ++c) {
            sap[c] += a * block[i][c];
            sbp[c] += b * block[i][c];
        }
    }
    det = saa * sbb - sab * sab;
    if (det <= 0) return 0;
    for (c = 0; c < 4u; ++c) {
        int64_t l = tc_div_round_s64((sap[c] * sbb - sbp[c] * sab) * 64, det);
        int64_t h = tc_div_round_s64((sbp[c] * saa - sap[c] * sab) * 64, det);
        lo[c] = (uint32_t)tc_clamp_i32((int32_t)l, 0, 255);
        hi[c] = (uint32_t)tc_clamp_i32((int32_t)h, 0, 255);
    }
    return 1;
}

/* Quantization-aware reconstruction error of a single-partition encoding:
 * round-trips the endpoints through the color quantizer exactly as they
 * will be emitted for this endpoint format, then evaluates the interpolated
 * weights against the source texels. Matches the reference decoder model
 * (CEM 8/12 endpoint swaps are error-neutral thanks to exact weight
 * mirroring, so no swap handling is needed for scoring). */
static uint64_t tc_astc_eval_single_sse(const tc_astc_encode_context *ctx,
                                        const uint8_t block[144][4],
                                        uint32_t count, const uint8_t wt[144],
                                        uint32_t endpoint_format,
                                        const uint32_t lo[4],
                                        const uint32_t hi[4],
                                        uint32_t color_quant_method) {
    uint32_t e0[4], e1[4];
    uint32_t c;
    switch (endpoint_format) {
        case 0u:
            e0[0] = e0[1] = e0[2] =
                tc_astc_color_roundtrip(ctx, color_quant_method, lo[0]);
            e1[0] = e1[1] = e1[2] =
                tc_astc_color_roundtrip(ctx, color_quant_method, hi[0]);
            e0[3] = e1[3] = 255u;
            break;
        case 4u:
            e0[0] = e0[1] = e0[2] =
                tc_astc_color_roundtrip(ctx, color_quant_method, lo[0]);
            e1[0] = e1[1] = e1[2] =
                tc_astc_color_roundtrip(ctx, color_quant_method, hi[0]);
            e0[3] = tc_astc_color_roundtrip(ctx, color_quant_method, lo[3]);
            e1[3] = tc_astc_color_roundtrip(ctx, color_quant_method, hi[3]);
            break;
        case 6u:
        case 10u: {
            uint32_t sum_lo = lo[0] + lo[1] + lo[2];
            uint32_t sum_hi = hi[0] + hi[1] + hi[2];
            uint32_t scale =
                sum_hi ? (sum_lo * 256u + sum_hi / 2u) / sum_hi : 0u;
            if (scale > 255u) scale = 255u;
            scale = tc_astc_color_roundtrip(ctx, color_quant_method, scale);
            for (c = 0; c < 3u; ++c) {
                e1[c] = tc_astc_color_roundtrip(ctx, color_quant_method, hi[c]);
                e0[c] = (e1[c] * scale) >> 8;
            }
            if (endpoint_format == 10u) {
                e0[3] = tc_astc_color_roundtrip(ctx, color_quant_method, lo[3]);
                e1[3] = tc_astc_color_roundtrip(ctx, color_quant_method, hi[3]);
            } else {
                e0[3] = e1[3] = 255u;
            }
            break;
        }
        default: /* CEM 8 / 12: direct per-channel pairs. */
            for (c = 0; c < 3u; ++c) {
                e0[c] = tc_astc_color_roundtrip(ctx, color_quant_method, lo[c]);
                e1[c] = tc_astc_color_roundtrip(ctx, color_quant_method, hi[c]);
            }
            if (endpoint_format == 12u) {
                e0[3] = tc_astc_color_roundtrip(ctx, color_quant_method, lo[3]);
                e1[3] = tc_astc_color_roundtrip(ctx, color_quant_method, hi[3]);
            } else {
                e0[3] = e1[3] = 255u;
            }
            break;
    }
    return tc_astc_recon_sse(block, count, e0, e1, wt);
}

/* Multi-partition variant: every texel interpolates its own partition's
 * endpoint pair with the shared per-texel weights. Endpoints round-trip
 * through the color quantizer with each partition's CEM semantics.
 * Returns UINT64_MAX when a CEM 8/12 pair would decode swapped
 * (blue-contracted), which shared weights cannot compensate. */
static uint64_t tc_astc_eval_partition_sse(
    const tc_astc_encode_context *ctx, const uint8_t block[144][4],
    uint32_t count, const tc_astc_partition_info *pi, uint32_t partition_count,
    const uint8_t formats[4], const uint32_t part_lo[4][4],
    const uint32_t part_hi[4][4], uint32_t color_quant_method,
    const uint8_t wt[144]) {
    uint32_t e0[4][4], e1[4][4];
    uint32_t i, c, p;
    for (p = 0; p < partition_count; ++p) {
        if (formats[p] == 6u || formats[p] == 10u) {
            uint32_t sum_lo = part_lo[p][0] + part_lo[p][1] + part_lo[p][2];
            uint32_t sum_hi = part_hi[p][0] + part_hi[p][1] + part_hi[p][2];
            uint32_t scale =
                sum_hi ? (sum_lo * 256u + sum_hi / 2u) / sum_hi : 0u;
            if (scale > 255u) scale = 255u;
            scale = tc_astc_color_roundtrip(ctx, color_quant_method, scale);
            for (c = 0; c < 3u; ++c) {
                e1[p][c] = tc_astc_color_roundtrip(ctx, color_quant_method,
                                                   part_hi[p][c]);
                e0[p][c] = (e1[p][c] * scale) >> 8;
            }
            if (formats[p] == 10u) {
                e0[p][3] = tc_astc_color_roundtrip(ctx, color_quant_method,
                                                   part_lo[p][3]);
                e1[p][3] = tc_astc_color_roundtrip(ctx, color_quant_method,
                                                   part_hi[p][3]);
            } else {
                e0[p][3] = e1[p][3] = 255u;
            }
        } else { /* CEM 8 / 12 */
            uint32_t s0 = 0, s1 = 0;
            for (c = 0; c < 3u; ++c) {
                e0[p][c] = tc_astc_color_roundtrip(ctx, color_quant_method,
                                                   part_lo[p][c]);
                e1[p][c] = tc_astc_color_roundtrip(ctx, color_quant_method,
                                                   part_hi[p][c]);
                s0 += e0[p][c];
                s1 += e1[p][c];
            }
            if (s0 > s1) return UINT64_MAX;
            if (formats[p] == 12u) {
                e0[p][3] = tc_astc_color_roundtrip(ctx, color_quant_method,
                                                   part_lo[p][3]);
                e1[p][3] = tc_astc_color_roundtrip(ctx, color_quant_method,
                                                   part_hi[p][3]);
            } else {
                e0[p][3] = e1[p][3] = 255u;
            }
        }
    }
    {
        uint8_t e0t[144][4], e1t[144][4];
        for (i = 0; i < count; ++i) {
            uint32_t part = pi->partition_of_texel[i];
            for (c = 0; c < 4u; ++c) {
                e0t[i][c] = (uint8_t)e0[part][c];
                e1t[i][c] = (uint8_t)e1[part][c];
            }
        }
        return tc_astc_recon_sse_pt(block, count,
                                    (const uint8_t(*)[4])e0t,
                                    (const uint8_t(*)[4])e1t, wt);
    }
}

/* Per-partition least-squares endpoint solve restricted to one partition's
 * texels. Returns 0 when degenerate. */
static int tc_astc_lsq_endpoints_partition(const uint8_t block[144][4],
                                           uint32_t count,
                                           const tc_astc_partition_info *pi,
                                           uint32_t part, const uint8_t wt[144],
                                           uint32_t lo[4], uint32_t hi[4]) {
    int64_t saa = 0, sab = 0, sbb = 0, det;
    int64_t sap[4] = {0, 0, 0, 0}, sbp[4] = {0, 0, 0, 0};
    uint32_t i, c;
#if defined(TC_X86)
#if defined(__AVX2__)
    if (tc_cpu_caps() & TC_CPU_AVX2) {
        tc_astc_lsq_sums_partition_avx2(
            block, count, pi->partition_of_texel, part, wt, &saa, &sab, &sbb,
            sap, sbp);
    } else
#endif
    if (tc_cpu_caps() & (TC_CPU_SSE2 | TC_CPU_SSE41)) {
        tc_astc_lsq_sums_partition_sse2(
            block, count, pi->partition_of_texel, part, wt, &saa, &sab, &sbb,
            sap, sbp);
    } else
#endif
    for (i = 0; i < count; ++i) {
        int64_t a, b;
        if (pi->partition_of_texel[i] != part) continue;
        a = 64 - wt[i];
        b = wt[i];
        saa += a * a;
        sab += a * b;
        sbb += b * b;
        for (c = 0; c < 4u; ++c) {
            sap[c] += a * block[i][c];
            sbp[c] += b * block[i][c];
        }
    }
    det = saa * sbb - sab * sab;
    if (det <= 0) return 0;
    for (c = 0; c < 4u; ++c) {
        int64_t l = tc_div_round_s64((sap[c] * sbb - sbp[c] * sab) * 64, det);
        int64_t h = tc_div_round_s64((sbp[c] * saa - sap[c] * sab) * 64, det);
        lo[c] = (uint32_t)tc_clamp_i32((int32_t)l, 0, 255);
        hi[c] = (uint32_t)tc_clamp_i32((int32_t)h, 0, 255);
    }
    return 1;
}

/* Dual-plane variant: RGB follows the color-plane weights, alpha the
 * alpha-plane weights; endpoints round-trip through the color quantizer
 * with CEM 10/12 semantics. */
static uint64_t tc_astc_eval_dual_sse(const tc_astc_encode_context *ctx,
                                      const uint8_t block[144][4],
                                      uint32_t count, const uint8_t wtc[144],
                                      const uint8_t wta[144],
                                      uint32_t endpoint_format,
                                      const uint32_t lo[4],
                                      const uint32_t hi[4],
                                      uint32_t color_quant_method) {
    uint32_t e0[4], e1[4];
    uint32_t i, c;
    uint64_t err = 0;
    if (endpoint_format == 10u) {
        uint32_t sum_lo = lo[0] + lo[1] + lo[2];
        uint32_t sum_hi = hi[0] + hi[1] + hi[2];
        uint32_t scale = sum_hi ? (sum_lo * 256u + sum_hi / 2u) / sum_hi : 0u;
        if (scale > 255u) scale = 255u;
        scale = tc_astc_color_roundtrip(ctx, color_quant_method, scale);
        for (c = 0; c < 3u; ++c) {
            e1[c] = tc_astc_color_roundtrip(ctx, color_quant_method, hi[c]);
            e0[c] = (e1[c] * scale) >> 8;
        }
    } else {
        for (c = 0; c < 3u; ++c) {
            e0[c] = tc_astc_color_roundtrip(ctx, color_quant_method, lo[c]);
            e1[c] = tc_astc_color_roundtrip(ctx, color_quant_method, hi[c]);
        }
    }
    e0[3] = tc_astc_color_roundtrip(ctx, color_quant_method, lo[3]);
    e1[3] = tc_astc_color_roundtrip(ctx, color_quant_method, hi[3]);
#if defined(__AVX2__)
    if (tc_cpu_caps() & TC_CPU_AVX2)
        return tc_astc_recon_sse_dual_avx2(block, count, e0, e1, wtc, wta);
#endif
#if defined(TC_X86)
    if (tc_cpu_caps() & (TC_CPU_SSE2 | TC_CPU_SSE41))
        return tc_astc_recon_sse_dual_sse2(block, count, e0, e1, wtc, wta);
#endif
    for (i = 0; i < count; ++i) {
        for (c = 0; c < 4u; ++c) {
            uint32_t w = c == 3u ? wta[i] : wtc[i];
            uint32_t recon = (e0[c] * (64u - w) + e1[c] * w + 32u) >> 6;
            int32_t d = (int32_t)block[i][c] - (int32_t)recon;
            err += (uint64_t)(d * d);
        }
    }
    return err;
}

static int tc_astc_color_quant_supported(uint32_t quant_method) {
    if (quant_method == 20u) return 1;
    return quant_method >= 4u && quant_method <= 19u &&
           tc_astc_color_pquant_to_uquant[quant_method - 4u] != NULL;
}

static uint8_t tc_astc_quant_color_pquant_slow(uint32_t v, uint32_t quant_method) {
    uint32_t levels = tc_astc_quant_levels(quant_method);
    uint32_t best = 0, best_err = UINT32_MAX, i;
    const uint8_t *table;
    if (quant_method == 20u) return (uint8_t)v;
    table = tc_astc_color_pquant_to_uquant[quant_method - 4u];
    for (i = 0; i < levels; ++i) {
        uint32_t u = table[i];
        uint32_t err = u > v ? u - v : v - u;
        if (err < best_err) {
            best_err = err;
            best = i;
        }
    }
    return (uint8_t)best;
}

static void tc_astc_build_color_pquant_lut(tc_astc_encode_context *ctx) {
    uint32_t q, v;
    for (q = 0; q < 21u; ++q) {
        for (v = 0; v < 256u; ++v) {
            ctx->color_pquant_lut[q][v] =
                tc_astc_color_quant_supported(q)
                    ? tc_astc_quant_color_pquant_slow(v, q)
                    : 0u;
        }
    }
}

static int tc_astc_choose_color_quant(uint32_t value_count, uint32_t bit_budget,
                                      uint32_t *quant_method) {
    int q;
    for (q = 20; q >= 4; --q) {
        uint32_t qm = (uint32_t)q;
        if (!tc_astc_color_quant_supported(qm)) continue;
        if (tc_astc_ise_sequence_bitcount(value_count, qm) <= bit_budget) {
            *quant_method = qm;
            return 1;
        }
    }
    return 0;
}

/* Table-backed variant of tc_astc_choose_color_quant for the hot loops. */
static int tc_astc_lut_color_quant(const tc_astc_encode_context *ctx,
                                   uint32_t value_count, uint32_t bit_budget,
                                   uint32_t *quant_method) {
    uint8_t m;
    if (bit_budget > 111u) bit_budget = 111u;
    m = ctx->color_quant_lut[value_count][bit_budget];
    if (m == 0xffu) return 0;
    *quant_method = m;
    return 1;
}

static void tc_astc_quantize_color_values(const tc_astc_encode_context *ctx,
                                          uint32_t quant_method,
                                          uint32_t value_count,
                                          const uint8_t in_values[8],
                                          uint8_t out_values[8]) {
    uint32_t i;
    for (i = 0; i < value_count; ++i)
        out_values[i] = ctx->color_pquant_lut[quant_method][in_values[i]];
}

/* Unquantized value of an ISE color symbol. */
static uint32_t tc_astc_color_symbol_uquant(uint32_t quant_method, uint32_t sym) {
    if (quant_method >= 20u) return sym;
    return tc_astc_color_pquant_to_uquant[quant_method - 4u][sym];
}

/* Value as the decoder will reconstruct it after quantization. */
static uint32_t tc_astc_color_roundtrip(const tc_astc_encode_context *ctx,
                                        uint32_t quant_method, uint32_t v) {
    return tc_astc_color_symbol_uquant(quant_method,
                                       ctx->color_pquant_lut[quant_method][v]);
}

static uint64_t tc_astc_rgb_sse_from_stats(const uint64_t sum[3],
                                           const uint64_t sumsq[3],
                                           uint32_t count) {
    uint32_t c;
    uint64_t err = 0;
    if (!count) return UINT64_MAX;
    for (c = 0; c < 3u; ++c) {
        err += sumsq[c] - (sum[c] * sum[c] + count / 2u) / count;
    }
    return err;
}

static int tc_astc_block_has_rgb_clusters(const uint8_t block[144][4],
                                          uint32_t count,
                                          uint32_t wanted_count) {
    uint32_t centers[4][3];
    uint32_t cluster_count = 0;
    uint32_t i, c;
    for (i = 0; i < count; ++i) {
        uint32_t best_dist = UINT32_MAX;
        for (c = 0; c < cluster_count; ++c) {
            int32_t dr = (int32_t)block[i][0] - (int32_t)centers[c][0];
            int32_t dg = (int32_t)block[i][1] - (int32_t)centers[c][1];
            int32_t db = (int32_t)block[i][2] - (int32_t)centers[c][2];
            uint32_t dist = (uint32_t)(dr * dr + dg * dg + db * db);
            if (dist < best_dist) best_dist = dist;
        }
        if (cluster_count == 0u || best_dist > 4096u) {
            if (cluster_count >= wanted_count) return 1;
            centers[cluster_count][0] = block[i][0];
            centers[cluster_count][1] = block[i][1];
            centers[cluster_count][2] = block[i][2];
            ++cluster_count;
        }
    }
    return cluster_count >= wanted_count;
}

#define TC_ASTC_PART2_SHORTLIST_MAX 96u

static int tc_astc_find_best_partition(const uint8_t block[144][4],
                                       const tc_astc_partition_info cache[1024],
                                       uint32_t cache_count,
                                       uint32_t partition_count,
                                       const tc_astc_encode_context *ctx,
                                       const tc_astc_partition_info **out_pi) {
    uint32_t i, c, p;
    uint16_t shortlist[TC_ASTC_PART2_SHORTLIST_MAX];
    uint16_t shortlist_mm[TC_ASTC_PART2_SHORTLIST_MAX];
    uint32_t shortlist_count;
    /* Bigger footprints have more distinct useful patterns; scale the
     * number of exactly-scored prefilter survivors with the texel count. */
    uint32_t shortlist_limit =
        TC_ASTC_PART2_SHORTLIST_BASE +
        (ctx->texel_count >> TC_ASTC_PART2_SHORTLIST_SHIFT);
    if (shortlist_limit > TC_ASTC_PART2_SHORTLIST_MAX)
        shortlist_limit = TC_ASTC_PART2_SHORTLIST_MAX;
    uint64_t sum_all[3] = {0, 0, 0}, sumsq_all[3] = {0, 0, 0};
    uint64_t base_err, best_err = UINT64_MAX;
    *out_pi = NULL;
    for (i = 0; i < ctx->texel_count; ++i) {
        for (c = 0; c < 3u; ++c) {
            uint32_t v = block[i][c];
            sum_all[c] += v;
            sumsq_all[c] += (uint64_t)v * v;
        }
    }
    base_err = tc_astc_rgb_sse_from_stats(sum_all, sumsq_all, ctx->texel_count);
    if (base_err < 4096u) return 0;

    /* Two-partition estimation prefilter: cluster the block once (luma
     * threshold + one Lloyd refinement), then rank every seed by bitmap
     * mismatch (XOR + popcount) and exact-score only the closest few.
     * This replaces an exact variance scan over ~half the 1024 seeds. */
    shortlist_count = 0;
    if (partition_count == 2u && cache_count > shortlist_limit) {
        uint64_t bbits[3] = {0, 0, 0};
        uint64_t mask[3] = {0, 0, 0};
        uint32_t bitmap_words = (ctx->texel_count + 63u) / 64u;
        uint32_t t, it, n1 = 0;
        uint16_t worst_of_best = 0xffffu;
        int32_t mean0[4], mean1[4];
        uint8_t assign[144];
        /* RGBA 2-means: farthest-point init, 5 Lloyd iterations (was 2;
         * more iterations tighten the reference bitmap and reduce
         * prefilter mis-rankings on challenging blocks). */
        {
            uint32_t far_t = 0, far_d = 0;
            for (t = 0; t < ctx->texel_count; ++t) {
                uint32_t d = 0;
                for (c = 0; c < 4u; ++c) {
                    int32_t v = (int32_t)block[t][c] - (int32_t)block[0][c];
                    d += (uint32_t)(v * v);
                }
                if (d > far_d) {
                    far_d = d;
                    far_t = t;
                }
                mask[t >> 6] |= 1ull << (t & 63u);
            }
            if (!far_d) goto full_scan; /* solid block */
            for (c = 0; c < 4u; ++c) {
                mean0[c] = block[0][c];
                mean1[c] = block[far_t][c];
            }
        }
        for (it = 0; it < 2u; ++it) {
            uint32_t sum0[4] = {0, 0, 0, 0}, sum1[4] = {0, 0, 0, 0};
            uint32_t c0 = 0, c1 = 0;
            for (t = 0; t < ctx->texel_count; ++t) {
                uint32_t d0 = 0, d1 = 0;
                for (c = 0; c < 4u; ++c) {
                    int32_t v0 = (int32_t)block[t][c] - mean0[c];
                    int32_t v1 = (int32_t)block[t][c] - mean1[c];
                    d0 += (uint32_t)(v0 * v0);
                    d1 += (uint32_t)(v1 * v1);
                }
                assign[t] = d1 < d0;
                if (assign[t]) {
                    for (c = 0; c < 4u; ++c) sum1[c] += block[t][c];
                    ++c1;
                } else {
                    for (c = 0; c < 4u; ++c) sum0[c] += block[t][c];
                    ++c0;
                }
            }
            if (!c0 || !c1) break;
            for (c = 0; c < 4u; ++c) {
                mean0[c] = (int32_t)((sum0[c] + c0 / 2u) / c0);
                mean1[c] = (int32_t)((sum1[c] + c1 / 2u) / c1);
            }
        }
        for (t = 0; t < ctx->texel_count; ++t) {
            if (assign[t]) {
                bbits[t >> 6] |= 1ull << (t & 63u);
                ++n1;
            }
        }
        if (n1 == 0u || n1 == ctx->texel_count) goto full_scan;
        for (i = 0; i < cache_count; ++i) {
            const tc_astc_partition_info *pi = cache + i;
            uint32_t mm = tc_astc_bitmap_mismatch(bbits, pi->bits, mask,
                                                  bitmap_words),
                     pos;
            if (shortlist_count == shortlist_limit && mm >= worst_of_best)
                continue;
            pos = shortlist_count < shortlist_limit ? shortlist_count++
                                                    : shortlist_count - 1u;
            while (pos > 0u && mm < shortlist_mm[pos - 1u]) {
                shortlist_mm[pos] = shortlist_mm[pos - 1u];
                shortlist[pos] = shortlist[pos - 1u];
                --pos;
            }
            shortlist_mm[pos] = (uint16_t)mm;
            shortlist[pos] = (uint16_t)i;
            worst_of_best = shortlist_mm[shortlist_count - 1u];
        }
    }
full_scan:
    {
        /* Batch variance: process up to 4 seeds per texel pass. The structure
         * is set up for future SIMD over the seed axis (4 seeds' accumulators
         * can be updated with one texel's broadcast values). */
        uint32_t total = shortlist_count ? shortlist_count : cache_count;
        for (i = 0; i < total; ) {
            uint32_t batch_n = total - i, s, t;
            uint64_t batch_sum[4][4][3], batch_sumsq[4][4][3];
            const tc_astc_partition_info *batch_pi[4];
            if (batch_n > 4) batch_n = 4;
            memset(batch_sum, 0, sizeof(batch_sum));
            memset(batch_sumsq, 0, sizeof(batch_sumsq));
            for (s = 0; s < batch_n; ++s) {
                uint32_t idx = shortlist_count ? shortlist[i + s] : i + s;
                batch_pi[s] = cache + idx;
            }
            for (t = 0; t < ctx->texel_count; ++t) {
                for (s = 0; s < batch_n; ++s) {
                    uint32_t p2 = batch_pi[s]->partition_of_texel[t];
                    for (c = 0; c < 3u; ++c) {
                        uint32_t v = block[t][c];
                        batch_sum[s][p2][c] += v;
                        batch_sumsq[s][p2][c] += (uint64_t)v * v;
                    }
                }
            }
            for (s = 0; s < batch_n; ++s) {
                uint64_t err = 0;
                for (p = 0; p < partition_count; ++p)
                    err += tc_astc_rgb_sse_from_stats(
                        batch_sum[s][p], batch_sumsq[s][p],
                        batch_pi[s]->partition_texel_count[p]);
                if (err < best_err) { best_err = err; *out_pi = batch_pi[s]; }
            }
            i += batch_n;
        }
    }
    return *out_pi && best_err * 4u < base_err * 3u;
}

static int tc_astc_partition_rgb_scale_ok(const uint32_t lo[3],
                                          const uint32_t hi[3]) {
    uint32_t range = (hi[0] - lo[0]) + (hi[1] - lo[1]) + (hi[2] - lo[2]);
    uint32_t sum_hi = hi[0] + hi[1] + hi[2];
    uint32_t c, scale;
    if (range <= 24u) return 1;
    if (!sum_hi) return 0;
    scale = ((lo[0] + lo[1] + lo[2]) * 256u + sum_hi / 2u) / sum_hi;
    if (scale > 255u) scale = 255u;
    for (c = 0; c < 3u; ++c) {
        uint32_t recon = (hi[c] * scale + 128u) >> 8;
        uint32_t diff = recon > lo[c] ? recon - lo[c] : lo[c] - recon;
        if (diff > 24u) return 0;
    }
    return 1;
}

static uint32_t tc_astc_partition_endpoint_type_code(uint32_t partition_count,
                                                     const uint8_t formats[4]) {
    uint32_t i, bitpos, encoded_type, low_class = 4u;
    for (i = 0; i < partition_count; ++i) {
        uint32_t cls = formats[i] >> 2;
        if (cls < low_class) low_class = cls;
    }
    if (low_class == 3u) low_class = 2u;
    encoded_type = low_class + 1u;
    bitpos = 2u;
    for (i = 0; i < partition_count; ++i) {
        encoded_type |= (((uint32_t)formats[i] >> 2) - low_class) << bitpos;
        ++bitpos;
    }
    for (i = 0; i < partition_count; ++i) {
        encoded_type |= ((uint32_t)formats[i] & 3u) << bitpos;
        bitpos += 2u;
    }
    return encoded_type;
}

static uint32_t tc_astc_partition_endpoint_value_count(uint32_t partition_count,
                                                       const uint8_t formats[4]) {
    uint32_t i, count = 0;
    for (i = 0; i < partition_count; ++i)
        count += (((uint32_t)formats[i] >> 2) + 1u) * 2u;
    return count;
}

static uint32_t tc_astc_emit_partition_endpoint_values(
    const tc_astc_encode_context *ctx, uint32_t quant_method,
    uint32_t partition_count, const uint8_t formats[4],
    const uint32_t part_lo[4][4], const uint32_t part_hi[4][4],
    uint8_t color_values[18]) {
    uint32_t i, n = 0;
    for (i = 0; i < partition_count; ++i) {
        if (formats[i] == 6u) {
            uint32_t sum_lo = part_lo[i][0] + part_lo[i][1] + part_lo[i][2];
            uint32_t sum_hi = part_hi[i][0] + part_hi[i][1] + part_hi[i][2];
            uint32_t scale = sum_hi ? (sum_lo * 256u + sum_hi / 2u) / sum_hi : 0u;
            if (scale > 255u) scale = 255u;
            color_values[n++] = ctx->color_pquant_lut[quant_method][part_hi[i][0]];
            color_values[n++] = ctx->color_pquant_lut[quant_method][part_hi[i][1]];
            color_values[n++] = ctx->color_pquant_lut[quant_method][part_hi[i][2]];
            color_values[n++] = ctx->color_pquant_lut[quant_method][scale];
        } else if (formats[i] == 10u) {
            uint32_t sum_lo = part_lo[i][0] + part_lo[i][1] + part_lo[i][2];
            uint32_t sum_hi = part_hi[i][0] + part_hi[i][1] + part_hi[i][2];
            uint32_t scale = sum_hi ? (sum_lo * 256u + sum_hi / 2u) / sum_hi : 0u;
            if (scale > 255u) scale = 255u;
            color_values[n++] = ctx->color_pquant_lut[quant_method][part_hi[i][0]];
            color_values[n++] = ctx->color_pquant_lut[quant_method][part_hi[i][1]];
            color_values[n++] = ctx->color_pquant_lut[quant_method][part_hi[i][2]];
            color_values[n++] = ctx->color_pquant_lut[quant_method][scale];
            color_values[n++] = ctx->color_pquant_lut[quant_method][part_lo[i][3]];
            color_values[n++] = ctx->color_pquant_lut[quant_method][part_hi[i][3]];
        } else if (formats[i] == 12u) {
            uint32_t c;
            for (c = 0; c < 4u; ++c) {
                color_values[n++] = ctx->color_pquant_lut[quant_method][part_lo[i][c]];
                color_values[n++] = ctx->color_pquant_lut[quant_method][part_hi[i][c]];
            }
        } else {
            color_values[n++] = ctx->color_pquant_lut[quant_method][part_lo[i][0]];
            color_values[n++] = ctx->color_pquant_lut[quant_method][part_hi[i][0]];
            color_values[n++] = ctx->color_pquant_lut[quant_method][part_lo[i][1]];
            color_values[n++] = ctx->color_pquant_lut[quant_method][part_hi[i][1]];
            color_values[n++] = ctx->color_pquant_lut[quant_method][part_lo[i][2]];
            color_values[n++] = ctx->color_pquant_lut[quant_method][part_hi[i][2]];
        }
    }
    return n;
}

/* Fits one shared decimated weight grid against per-partition endpoint
 * lines: each texel's ideal weight comes from projecting onto its own
 * partition's lo->hi diagonal. Returns the reconstruction SSE with the
 * (unquantized) per-partition endpoints. */
/* Per-texel projection onto each texel's own partition line, plus the
 * per-texel (unquantized) endpoint arrays used for fit-time reconstruction.
 * Candidate-independent, so it runs once per partitioned block. */
static int tc_astc_partition_project(const uint8_t block[144][4],
                                     uint32_t count,
                                     const tc_astc_partition_info *pi,
                                     uint32_t partition_count,
                                     const uint32_t part_lo[4][4],
                                     const uint32_t part_hi[4][4],
                                     uint8_t ideal[144], uint8_t active[144],
                                     uint8_t e0t[144][4],
                                     uint8_t e1t[144][4]) {
    uint32_t denom[4], recip[4];
    uint32_t i, c, p;
    int all_active = 1;
    for (p = 0; p < partition_count; ++p) {
        uint32_t d = 0;
        for (c = 0; c < 4u; ++c) {
            uint32_t range = part_hi[p][c] - part_lo[p][c];
            d += range * range;
        }
        denom[p] = d;
        recip[p] = d ? TC_ASTC_PROJ_RECIP(d) : 0u;
    }
    for (i = 0; i < count; ++i) {
        uint32_t part = pi->partition_of_texel[i];
        const uint8_t *px = block[i];
        int32_t dot = 0;
        if (e0t && e1t) {
            for (c = 0; c < 4u; ++c) {
                e0t[i][c] = (uint8_t)part_lo[part][c];
                e1t[i][c] = (uint8_t)part_hi[part][c];
            }
        }
        if (!denom[part]) {
            /* Solid partition: reconstruction ignores the weight, so the
             * texel must not constrain the shared grid either. */
            ideal[i] = 0;
            active[i] = 0;
            all_active = 0;
            continue;
        }
        active[i] = 1;
        for (c = 0; c < 4u; ++c) {
            dot += (int32_t)((int32_t)px[c] - (int32_t)part_lo[part][c]) *
                   (int32_t)((int32_t)part_hi[part][c] -
                             (int32_t)part_lo[part][c]);
        }
        ideal[i] = (uint8_t)(dot <= 0 ? 0u
                                      : tc_astc_project_weight(
                                            (uint32_t)dot * 64u, recip[part]));
    }
    return all_active;
}

/* Partition-path counterpart of tc_astc_fit_from_ideal: quantize the
 * hoisted ideal weights onto a candidate grid and reconstruct against the
 * per-texel endpoint arrays. */
static void tc_astc_weights_from_ideal_pt(const tc_astc_encode_context *ctx,
                                          uint32_t count,
                                          const uint8_t ideal[144],
                                          const uint8_t active[144],
                                          uint32_t quant_method,
                                          const tc_astc_decim_cache_entry *decim,
                                          uint32_t weight_count,
                                          uint8_t out_weights[64],
                                          uint8_t wt[144]) {
    const uint8_t *lut_q = ctx->wq_q[quant_method];
    const uint8_t *lut_wt = ctx->wq_wt[quant_method];
    uint32_t i;
    if (decim->direct) {
        for (i = 0; i < count; ++i) {
            out_weights[i] = lut_q[ideal[i]];
            wt[i] = lut_wt[ideal[i]];
        }
    } else {
        uint32_t weight_accum[64];
        uint8_t grid[64];
        for (i = 0; i < weight_count; ++i) {
            weight_accum[i] = 0;
        }
        if (!active) {
            for (i = 0; i < count; ++i) {
                uint32_t n = decim->tw_count[i], j;
                for (j = 0; j < n; ++j) {
                    weight_accum[decim->tw_idx[i][j]] +=
                        (uint32_t)ideal[i] * decim->tw_contrib[i][j];
                }
            }
            for (i = 0; i < weight_count; ++i) {
                uint32_t contrib = decim->weight_contrib[i];
                uint32_t v = contrib ? (weight_accum[i] + contrib / 2u) / contrib
                                     : 0u;
                out_weights[i] = lut_q[v];
                grid[i] = lut_wt[v];
            }
        } else {
            uint32_t weight_contrib[64];
            for (i = 0; i < weight_count; ++i) weight_contrib[i] = 0;
            for (i = 0; i < count; ++i) {
                uint32_t n = decim->tw_count[i], j;
                if (!active[i]) continue;
                for (j = 0; j < n; ++j) {
                    weight_accum[decim->tw_idx[i][j]] +=
                        (uint32_t)ideal[i] * decim->tw_contrib[i][j];
                    weight_contrib[decim->tw_idx[i][j]] +=
                        decim->tw_contrib[i][j];
                }
            }
            for (i = 0; i < weight_count; ++i) {
                uint32_t v = weight_contrib[i]
                                 ? (weight_accum[i] + weight_contrib[i] / 2u) /
                                       weight_contrib[i]
                                 : 0u;
                out_weights[i] = lut_q[v];
                grid[i] = lut_wt[v];
            }
        }
        tc_astc_infill_weights_grid(grid, decim, count, wt);
    }
}

static uint64_t tc_astc_fit_from_ideal_pt(const tc_astc_encode_context *ctx,
                                          const uint8_t block[144][4],
                                          uint32_t count,
                                          const uint8_t ideal[144],
                                          const uint8_t active[144],
                                          const uint8_t e0t[144][4],
                                          const uint8_t e1t[144][4],
                                          uint32_t quant_method,
                                          const tc_astc_decim_cache_entry *decim,
                                          uint32_t weight_count,
                                          uint8_t out_weights[64],
                                          uint8_t out_wt[144]) {
    uint8_t local_wt[144];
    uint8_t *wt = out_wt ? out_wt : local_wt;
    tc_astc_weights_from_ideal_pt(ctx, count, ideal, active, quant_method,
                                  decim, weight_count, out_weights, wt);
    return tc_astc_recon_sse_pt(block, count, e0t, e1t, wt);
}

static int tc_encode_astc_partition_rgb_block(const uint8_t block[144][4],
                                              uint32_t count,
                                              uint32_t partition_count,
                                              tc_astc_encode_context *ctx,
                                              uint8_t out[16],
                                              uint64_t *out_err) {
    const tc_astc_block_mode_info *candidates;
    const tc_astc_decim_cache_entry *decim;
    uint8_t weightbuf[16], weights[64], color_values[18];
    uint8_t best_wt[144];
    uint8_t endpoint_formats[4];
    uint8_t fit_ideal[144], fit_active[144];
    uint8_t fit_e0t[144][4], fit_e1t[144][4];
    uint32_t cand_count, ci, c, i, bitpos, best = 0xffffffffu;
    uint32_t scanned = 0, scan_cap;
    uint32_t part_lo[4][4], part_hi[4][4];
    const tc_astc_partition_info *pi = NULL;
    const tc_astc_partition_info *cache = ctx->part2_cache;
    uint32_t cache_count = ctx->part2_count;
    uint32_t endpoint_format = partition_count == 4u ? 6u : 8u;
    uint32_t endpoint_values_per_partition = endpoint_format == 6u ? 4u : 6u;
    uint32_t endpoint_value_count = partition_count * endpoint_values_per_partition;
    uint32_t color_quant_method = 14u;
    uint32_t endpoint_highpart_size = 0u;
    int endpoint_formats_matched = 1;
    int fit_all_active;
    int is_opaque = tc_astc_block_is_opaque(block, count);
    int is_luminance = tc_astc_block_is_luminance(block, count);
    int have_best_wt = 0;
    uint64_t best_err = UINT64_MAX;
    if (partition_count == 4u) {
        cache = ctx->part4_cache;
        cache_count = ctx->part4_count;
        if (!tc_astc_block_has_rgb_clusters(block, count, 4u)) return 0;
    } else if (partition_count == 3u) {
        cache = ctx->part3_cache;
        cache_count = ctx->part3_count;
        if (!tc_astc_block_has_rgb_clusters(block, count, 3u)) return 0;
    }
    if (is_luminance || (!is_opaque && (partition_count != 2u || count < 25u)) ||
        !tc_astc_find_best_partition(block, cache, cache_count, partition_count, ctx,
                                     &pi))
        return 0;
    if (!is_opaque) {
        endpoint_format = tc_astc_block_is_rgb_scale(block, count) ? 10u : 12u;
        endpoint_values_per_partition = endpoint_format == 10u ? 6u : 8u;
        endpoint_value_count = partition_count * endpoint_values_per_partition;
    }

    for (i = 0; i < partition_count; ++i) {
        for (c = 0; c < 4u; ++c) {
            part_lo[i][c] = 255u;
            part_hi[i][c] = 0u;
        }
    }
    for (i = 0; i < count; ++i) {
        uint32_t part = pi->partition_of_texel[i];
        for (c = 0; c < 4u; ++c) {
            if (block[i][c] < part_lo[part][c]) part_lo[part][c] = block[i][c];
            if (block[i][c] > part_hi[part][c]) part_hi[part][c] = block[i][c];
        }
    }

    fit_all_active = tc_astc_partition_project(
        block, count, pi, partition_count, (const uint32_t(*)[4])part_lo,
        (const uint32_t(*)[4])part_hi, fit_ideal, fit_active,
        ctx->quality > 1 ? fit_e0t : NULL,
        ctx->quality > 1 ? fit_e1t : NULL);
    cand_count = tc_astc_get_candidates(ctx, 29u, &candidates);
    scan_cap = ctx->quality > 1 ? cand_count : TC_ASTC_MEDIUM_PART_SCAN_CAP;
    /* Batch-scoring loop for quality<=1; quality>1 uses per-candidate exact fit. */
    if (ctx->quality > 1) {
        for (ci = 0; ci < cand_count && scanned < scan_cap; ++ci) {
            uint32_t cand_color_quant;
            uint32_t color_bits_available = 99u - candidates[ci].weight_bits;
            uint64_t err;
            uint8_t cand_weights[64];
            if (candidates[ci].weight_bits >= 99u ||
                !tc_astc_lut_color_quant(ctx, endpoint_value_count,
                                         color_bits_available, &cand_color_quant))
                continue;
            decim = tc_astc_get_decim_cache(ctx, candidates[ci].weight_x,
                                            candidates[ci].weight_y);
            if (!decim) continue;
            ++scanned;
            err = tc_astc_fit_from_ideal_pt(
                ctx, block, count, fit_ideal,
                fit_all_active ? NULL : fit_active,
                (const uint8_t(*)[4])fit_e0t, (const uint8_t(*)[4])fit_e1t,
                candidates[ci].quant_method, decim,
                (uint32_t)candidates[ci].weight_x * candidates[ci].weight_y,
                cand_weights, NULL);
            if (err < best_err) {
                best_err = err;
                best = ci;
                color_quant_method = cand_color_quant;
                memcpy(weights, cand_weights,
                       (uint32_t)candidates[ci].weight_x *
                           candidates[ci].weight_y);
            }
        }
    } else if (fit_all_active) for (ci = 0; ci < cand_count && scanned < scan_cap; ) {
        const tc_astc_decim_cache_entry *batch_decim_p = NULL;
        uint32_t batch_n_p = 0, batch_qm_p = 0, bj;
        uint32_t batch_start = ci;
        while (ci < cand_count && scanned + batch_n_p < scan_cap) {
            uint32_t cand_color_quant;
            uint32_t color_bits_available = 99u - candidates[ci].weight_bits;
            const tc_astc_decim_cache_entry *d;
            if (candidates[ci].weight_bits >= 99u ||
                !tc_astc_lut_color_quant(ctx, endpoint_value_count,
                                         color_bits_available, &cand_color_quant)) {
                ++ci; continue;
            }
            d = tc_astc_get_decim_cache(ctx, candidates[ci].weight_x,
                                        candidates[ci].weight_y);
            if (!d) { ++ci; continue; }
            if (!batch_decim_p) { batch_decim_p = d; batch_qm_p = candidates[ci].quant_method; }
            else if (d != batch_decim_p || candidates[ci].quant_method != batch_qm_p) break;
            ++batch_n_p; ++ci;
        }
        if (batch_n_p > 0) {
            uint64_t batch_errs[32];
            const uint8_t *batch_ideals[32];
            uint32_t wc = (uint32_t)batch_decim_p->weight_x * batch_decim_p->weight_y;
            for (bj = 0; bj < batch_n_p; ++bj) batch_ideals[bj] = fit_ideal;
            tc_astc_score_batch(ctx, count, batch_ideals, batch_n_p,
                                batch_qm_p, batch_decim_p, wc, batch_errs);
            for (bj = 0; bj < batch_n_p; ++bj) {
                uint32_t cj = batch_start + bj;
                scanned++;
                if (batch_errs[bj] < best_err) {
                    best_err = batch_errs[bj];
                    best = cj;
                    { uint32_t cba = 99u - candidates[cj].weight_bits;
                      if (!tc_astc_lut_color_quant(ctx, endpoint_value_count, cba, &color_quant_method))
                        color_quant_method = 20u; }
                }
            }
        }
    } else for (ci = 0; ci < cand_count && scanned < scan_cap; ++ci) {
        /* Non-active-texel mask: can't batch (active mask differs per candidate) */
        uint32_t cand_color_quant;
        uint32_t color_bits_available = 99u - candidates[ci].weight_bits;
        uint64_t err;
        if (candidates[ci].weight_bits >= 99u ||
            !tc_astc_lut_color_quant(ctx, endpoint_value_count,
                                     color_bits_available, &cand_color_quant))
            continue;
        decim = tc_astc_get_decim_cache(ctx, candidates[ci].weight_x,
                                        candidates[ci].weight_y);
        if (!decim) continue;
        ++scanned;
        err = tc_astc_score_from_ideal(ctx, count, fit_ideal, fit_active,
                                       candidates[ci].quant_method, decim,
                                       (uint32_t)candidates[ci].weight_x * candidates[ci].weight_y);
        if (err < best_err) { best_err = err; best = ci; color_quant_method = cand_color_quant; }
    }
    if (best == 0xffffffffu) return 0;
    if (ctx->quality <= 1) {
        decim = tc_astc_get_decim_cache(ctx, candidates[best].weight_x,
                                        candidates[best].weight_y);
        if (!decim) return 0;
        tc_astc_weights_from_ideal_pt(
            ctx, count, fit_ideal, fit_all_active ? NULL : fit_active,
            candidates[best].quant_method, decim,
            (uint32_t)candidates[best].weight_x * candidates[best].weight_y,
            weights, best_wt);
        have_best_wt = 1;
    }
    for (i = 0; i < partition_count; ++i) endpoint_formats[i] = (uint8_t)endpoint_format;
    if (is_opaque && (partition_count == 2u || partition_count == 3u)) {
        uint32_t scale_count = 0u;
        for (i = 0; i < partition_count; ++i) {
            if (tc_astc_partition_rgb_scale_ok(part_lo[i], part_hi[i])) {
                endpoint_formats[i] = 6u;
                ++scale_count;
            } else {
                endpoint_formats[i] = 8u;
            }
        }
        if (scale_count != 0u && scale_count != partition_count) {
            endpoint_formats_matched = 0;
        } else {
            for (i = 0; i < partition_count; ++i)
                endpoint_formats[i] = (uint8_t)endpoint_format;
        }
    }
    endpoint_value_count =
        tc_astc_partition_endpoint_value_count(partition_count, endpoint_formats);
    if (!endpoint_formats_matched) endpoint_highpart_size = 3u * partition_count - 4u;
    {
        uint32_t color_bits_available;
        if (candidates[best].weight_bits + endpoint_highpart_size >= 99u) {
            endpoint_formats_matched = 1;
            endpoint_highpart_size = 0u;
            for (i = 0; i < partition_count; ++i)
                endpoint_formats[i] = (uint8_t)endpoint_format;
            endpoint_value_count = tc_astc_partition_endpoint_value_count(
                partition_count, endpoint_formats);
        }
        color_bits_available = 99u - candidates[best].weight_bits - endpoint_highpart_size;
        if (!tc_astc_choose_color_quant(endpoint_value_count, color_bits_available,
                                        &color_quant_method)) {
            if (!endpoint_formats_matched) {
                endpoint_formats_matched = 1;
                endpoint_highpart_size = 0u;
                for (i = 0; i < partition_count; ++i)
                    endpoint_formats[i] = (uint8_t)endpoint_format;
                endpoint_value_count = tc_astc_partition_endpoint_value_count(
                    partition_count, endpoint_formats);
                color_bits_available =
                    99u - candidates[best].weight_bits - endpoint_highpart_size;
            }
            if (!tc_astc_choose_color_quant(endpoint_value_count, color_bits_available,
                                            &color_quant_method))
                return 0;
        }
    }

    /* Refine each partition's endpoints with a least-squares solve against
     * the shared fitted weights, accepting per partition only when the
     * quantization-aware reconstruction error improves (formats stay as
     * classified from the componentwise bounds). */
    {
        uint8_t wt[144];
        uint64_t err;
        decim = tc_astc_get_decim_cache(ctx, candidates[best].weight_x,
                                        candidates[best].weight_y);
        if (!decim) return 0;
        if (have_best_wt) {
            memcpy(wt, best_wt, count);
        } else {
            tc_astc_infill_weights(weights, decim, count,
                                   candidates[best].quant_method, wt);
        }
        err = tc_astc_eval_partition_sse(
            ctx, block, count, pi, partition_count, endpoint_formats,
            (const uint32_t(*)[4])part_lo, (const uint32_t(*)[4])part_hi,
            color_quant_method, wt);
        for (i = 0; i < partition_count; ++i) {
            uint32_t lsq_lo[4], lsq_hi[4], keep_lo[4], keep_hi[4];
            uint64_t lsq_err;
            if (!tc_astc_lsq_endpoints_partition(block, count, pi, i, wt,
                                                 lsq_lo, lsq_hi))
                continue;
            memcpy(keep_lo, part_lo[i], sizeof(keep_lo));
            memcpy(keep_hi, part_hi[i], sizeof(keep_hi));
            memcpy(part_lo[i], lsq_lo, sizeof(lsq_lo));
            memcpy(part_hi[i], lsq_hi, sizeof(lsq_hi));
            lsq_err = tc_astc_eval_partition_sse(
                ctx, block, count, pi, partition_count, endpoint_formats,
                (const uint32_t(*)[4])part_lo, (const uint32_t(*)[4])part_hi,
                color_quant_method, wt);
            if (lsq_err < err) {
                err = lsq_err;
            } else {
                memcpy(part_lo[i], keep_lo, sizeof(keep_lo));
                memcpy(part_hi[i], keep_hi, sizeof(keep_hi));
            }
        }
        if (err == UINT64_MAX) return 0;
        *out_err = err;
    }

    endpoint_value_count = tc_astc_emit_partition_endpoint_values(
        ctx, color_quant_method, partition_count, endpoint_formats, part_lo, part_hi,
        color_values);

    memset(out, 0, 16u);
    memset(weightbuf, 0, sizeof(weightbuf));
    tc_astc_scramble_weights(weights,
                             (uint32_t)candidates[best].weight_x *
                                 candidates[best].weight_y,
                             candidates[best].quant_method);
    (void)tc_astc_ise_encode_bits(candidates[best].quant_method,
                                  (uint32_t)candidates[best].weight_x *
                                      candidates[best].weight_y,
                                  weights, weightbuf, sizeof(weightbuf), 0u);
    for (i = 0; i < 16u; ++i) out[i] = tc_bitrev8(weightbuf[15u - i]);
    bitpos = 0;
    tc_set_bits(out, &bitpos, candidates[best].block_mode, 11u);
    tc_set_bits(out, &bitpos, partition_count - 1u, 2u);
    tc_set_bits(out, &bitpos, pi->partition_index & 63u, 6u);
    tc_set_bits(out, &bitpos, pi->partition_index >> 6, 4u);
    bitpos = 23u;
    if (endpoint_formats_matched) {
        tc_set_bits(out, &bitpos, ((uint32_t)endpoint_formats[0]) << 2, 6u);
    } else {
        uint32_t endpoint_type =
            tc_astc_partition_endpoint_type_code(partition_count, endpoint_formats);
        uint32_t endpoint_highpart_pos =
            128u - candidates[best].weight_bits - endpoint_highpart_size;
        tc_set_bits(out, &bitpos, endpoint_type & 63u, 6u);
        bitpos = endpoint_highpart_pos;
        tc_set_bits(out, &bitpos, endpoint_type >> 6, endpoint_highpart_size);
    }
    (void)tc_astc_ise_encode_bits(color_quant_method, endpoint_value_count,
                                  color_values, out, 16u, 29u);
    return 1;
}

static int tc_encode_astc_dual_rgba_block(const uint8_t block[144][4],
                                          uint32_t count,
                                          int quality,
                                          tc_astc_encode_context *ctx,
                                          uint8_t out[16],
                                          uint64_t *out_err) {
    const tc_astc_block_mode_info *candidates;
    const tc_astc_decim_cache_entry *decim;
    uint8_t weightbuf[16], weights[128], color_weights[64], alpha_weights[64];
    uint8_t values[8];
    uint32_t cand_count, ci, c, i, bitpos, best = 0xffffffffu;
    uint32_t color_lo[4], color_hi[4], alpha_lo[4], alpha_hi[4];
    uint32_t line_color_lo[4], line_color_hi[4], line_alpha_lo[4],
        line_alpha_hi[4];
    uint32_t luma_lo_i, luma_hi_i, alpha_lo_i, alpha_hi_i;
    uint32_t scanned = 0, scan_cap;
    uint32_t color_recip, alpha_recip;
    uint8_t color_ideal[144], alpha_ideal[144];
    uint32_t alpha_min = 255u, alpha_max = 0u;
    int is_rgb_scale;
    uint32_t endpoint_format;
    uint32_t endpoint_value_count;
    uint32_t color_quant_method = 20u;
    uint64_t best_err = UINT64_MAX;
    if (quality < 1 || tc_astc_block_is_opaque(block, count) ||
        tc_astc_block_is_luminance(block, count))
        return 0;
    is_rgb_scale = tc_astc_block_is_rgb_scale(block, count);
    if (quality < 2 && is_rgb_scale) return 0;
    endpoint_format = is_rgb_scale ? 10u : 12u;
    endpoint_value_count = is_rgb_scale ? 6u : 8u;
    for (i = 0; i < count; ++i) {
        if (block[i][3] < alpha_min) alpha_min = block[i][3];
        if (block[i][3] > alpha_max) alpha_max = block[i][3];
    }
    if (alpha_max - alpha_min < 48u) return 0;

    tc_astc_axis_extremes(block, count, 4u, &luma_lo_i, &luma_hi_i);
    tc_astc_axis_extremes(block, count, 3u, &alpha_lo_i, &alpha_hi_i);
    if (!tc_astc_axis_line(block, luma_lo_i, luma_hi_i, line_color_lo,
                           line_color_hi, &color_recip, NULL) ||
        !tc_astc_axis_line(block, alpha_lo_i, alpha_hi_i, line_alpha_lo,
                           line_alpha_hi, &alpha_recip, NULL))
        return 0;
    tc_astc_project_ideal(block, count, line_color_lo, line_color_hi,
                          color_recip, color_ideal);
    tc_astc_project_ideal(block, count, line_alpha_lo, line_alpha_hi,
                          alpha_recip, alpha_ideal);
    candidates = ctx->dual_candidates;
    cand_count = ctx->dual_count;
    scan_cap = cand_count; /* dual candidates are few; no cap needed */
    for (ci = 0; ci < cand_count && scanned < scan_cap; ++ci) {
        uint64_t color_err, alpha_err, err;
        uint32_t cand_color_quant_method;
        uint32_t color_bits_available;
        uint32_t weight_count =
            (uint32_t)candidates[ci].weight_x * candidates[ci].weight_y;
        if (candidates[ci].weight_bits >= 109u) continue;
        color_bits_available = 109u - candidates[ci].weight_bits;
        if (!tc_astc_lut_color_quant(ctx, endpoint_value_count,
                                     color_bits_available,
                                     &cand_color_quant_method))
            continue;
        decim = tc_astc_get_decim_cache(ctx, candidates[ci].weight_x,
                                        candidates[ci].weight_y);
        if (!decim) continue;
        ++scanned;
        {
            uint8_t cand_color_weights[64], cand_alpha_weights[64];
            color_err = tc_astc_fit_from_ideal(
                ctx, block, count, color_ideal, line_color_lo, line_color_hi,
                candidates[ci].quant_method, decim, weight_count,
                cand_color_weights, NULL);
            alpha_err = tc_astc_fit_from_ideal(
                ctx, block, count, alpha_ideal, line_alpha_lo, line_alpha_hi,
                candidates[ci].quant_method, decim, weight_count,
                cand_alpha_weights, NULL);
            err = color_err + alpha_err;
            if (err < best_err) {
                best_err = err;
                best = ci;
                memcpy(color_weights, cand_color_weights, weight_count);
                memcpy(alpha_weights, cand_alpha_weights, weight_count);
                color_quant_method = cand_color_quant_method;
            }
        }
    }
    if (best == 0xffffffffu) return 0;
    memcpy(color_lo, line_color_lo, sizeof(color_lo));
    memcpy(color_hi, line_color_hi, sizeof(color_hi));
    memcpy(alpha_lo, line_alpha_lo, sizeof(alpha_lo));
    memcpy(alpha_hi, line_alpha_hi, sizeof(alpha_hi));

    /* Quantization-aware error of the emitted dual-plane block, with a
     * least-squares refinement pass over both planes' endpoints. */
    {
        const tc_astc_decim_cache_entry *bd = tc_astc_get_decim_cache(
            ctx, candidates[best].weight_x, candidates[best].weight_y);
        uint8_t wtc[144], wta[144];
        uint32_t cur[2][4], lsq_lo[4], lsq_hi[4];
        uint64_t err;
        if (!bd) return 0;
        tc_astc_infill_weights(color_weights, bd, count,
                               candidates[best].quant_method, wtc);
        tc_astc_infill_weights(alpha_weights, bd, count,
                               candidates[best].quant_method, wta);
        for (c = 0; c < 4u; ++c) {
            cur[0][c] = c == 3u ? alpha_lo[3] : color_lo[c];
            cur[1][c] = c == 3u ? alpha_hi[3] : color_hi[c];
        }
        err = tc_astc_eval_dual_sse(ctx, block, count, wtc, wta,
                                    endpoint_format, cur[0], cur[1],
                                    color_quant_method);
        if (tc_astc_lsq_endpoints(block, count, wtc, lsq_lo, lsq_hi)) {
            uint32_t try_lo[4], try_hi[4];
            uint64_t lsq_err;
            memcpy(try_lo, cur[0], sizeof(try_lo));
            memcpy(try_hi, cur[1], sizeof(try_hi));
            for (c = 0; c < 3u; ++c) {
                try_lo[c] = lsq_lo[c];
                try_hi[c] = lsq_hi[c];
            }
            lsq_err = tc_astc_eval_dual_sse(ctx, block, count, wtc, wta,
                                            endpoint_format, try_lo, try_hi,
                                            color_quant_method);
            if (lsq_err < err) {
                err = lsq_err;
                for (c = 0; c < 3u; ++c) {
                    color_lo[c] = try_lo[c];
                    color_hi[c] = try_hi[c];
                }
                memcpy(cur[0], try_lo, sizeof(try_lo));
                memcpy(cur[1], try_hi, sizeof(try_hi));
            }
        }
        if (tc_astc_lsq_endpoints(block, count, wta, lsq_lo, lsq_hi)) {
            uint32_t try_lo[4], try_hi[4];
            uint64_t lsq_err;
            memcpy(try_lo, cur[0], sizeof(try_lo));
            memcpy(try_hi, cur[1], sizeof(try_hi));
            try_lo[3] = lsq_lo[3];
            try_hi[3] = lsq_hi[3];
            lsq_err = tc_astc_eval_dual_sse(ctx, block, count, wtc, wta,
                                            endpoint_format, try_lo, try_hi,
                                            color_quant_method);
            if (lsq_err < err) {
                err = lsq_err;
                alpha_lo[3] = try_lo[3];
                alpha_hi[3] = try_hi[3];
            }
        }
        *out_err = err;
    }

    if (endpoint_format == 10u) {
        uint32_t sum_lo = color_lo[0] + color_lo[1] + color_lo[2];
        uint32_t sum_hi = color_hi[0] + color_hi[1] + color_hi[2];
        uint32_t scale = sum_hi ? (sum_lo * 256u + sum_hi / 2u) / sum_hi : 0u;
        if (scale > 255u) scale = 255u;
        values[0] = (uint8_t)color_hi[0];
        values[1] = (uint8_t)color_hi[1];
        values[2] = (uint8_t)color_hi[2];
        values[3] = (uint8_t)scale;
        values[4] = (uint8_t)alpha_lo[3];
        values[5] = (uint8_t)alpha_hi[3];
    } else {
        for (c = 0; c < 3u; ++c) {
            values[c * 2u + 0u] = (uint8_t)color_lo[c];
            values[c * 2u + 1u] = (uint8_t)color_hi[c];
        }
        values[6] = (uint8_t)alpha_lo[3];
        values[7] = (uint8_t)alpha_hi[3];
    }
    {
        uint32_t weight_count =
            (uint32_t)candidates[best].weight_x * candidates[best].weight_y;
        /* CEM 12: keep the emitted pair in ascending quantized-RGB-sum order
         * so the decoder takes the direct (non-blue-contract) path; both
         * planes' weights mirror to compensate for the endpoint swap. */
        if (endpoint_format == 12u) {
            uint32_t s_lo = 0, s_hi = 0;
            for (c = 0; c < 3u; ++c) {
                s_lo += tc_astc_color_roundtrip(ctx, color_quant_method,
                                                values[c * 2u]);
                s_hi += tc_astc_color_roundtrip(ctx, color_quant_method,
                                                values[c * 2u + 1u]);
            }
            if (s_lo > s_hi) {
                for (c = 0; c < 4u; ++c) {
                    uint8_t t = values[c * 2u];
                    values[c * 2u] = values[c * 2u + 1u];
                    values[c * 2u + 1u] = t;
                }
                tc_astc_invert_weights(color_weights, weight_count,
                                       candidates[best].quant_method);
                tc_astc_invert_weights(alpha_weights, weight_count,
                                       candidates[best].quant_method);
            }
        }
        tc_astc_scramble_weights(color_weights, weight_count,
                                 candidates[best].quant_method);
        tc_astc_scramble_weights(alpha_weights, weight_count,
                                 candidates[best].quant_method);
        /* Dual-plane weights are interleaved per texel: plane 0 (color) in
         * the even ISE positions, plane 1 (alpha) in the odd ones. */
        for (i = 0; i < weight_count; ++i) {
            weights[i * 2u] = color_weights[i];
            weights[i * 2u + 1u] = alpha_weights[i];
        }
        memset(out, 0, 16u);
        memset(weightbuf, 0, sizeof(weightbuf));
        (void)tc_astc_ise_encode_bits(candidates[best].quant_method,
                                      weight_count * 2u, weights, weightbuf,
                                      sizeof(weightbuf), 0u);
    }
    for (i = 0; i < 16u; ++i) out[i] = tc_bitrev8(weightbuf[15u - i]);
    bitpos = 0;
    tc_set_bits(out, &bitpos, candidates[best].block_mode, 11u);
    tc_set_bits(out, &bitpos, 0u, 2u);
    tc_set_bits(out, &bitpos, endpoint_format, 4u);
    bitpos = 128u - candidates[best].weight_bits - 2u;
    tc_set_bits(out, &bitpos, 3u, 2u);

    {
        uint8_t packed_values[8];
        tc_astc_quantize_color_values(ctx, color_quant_method,
                                      endpoint_value_count, values,
                                      packed_values);
        (void)tc_astc_ise_encode_bits(color_quant_method, endpoint_value_count,
                                      packed_values, out, 16u, 17u);
    }
    return 1;
}

static void tc_encode_astc_ldr_block(const uint8_t block[144][4],
                                     uint32_t count,
                                     int quality,
                                     tc_astc_encode_context *ctx,
                                     uint8_t out[16]) {
    uint8_t weightbuf[16];
    uint8_t weights[64];
    uint8_t values[8];
    uint32_t bitpos;
    uint32_t i, c, axis, ci;
    uint32_t lo[4], hi[4];
    uint32_t best_block_mode = 66u;
    uint32_t best_quant_method = 2u;
    uint32_t best_weight_count = 16u;
    const tc_astc_decim_cache_entry *best_decim = NULL;
    int is_opaque = tc_astc_block_is_opaque(block, count);
    int is_luminance = tc_astc_block_is_luminance(block, count);
    int is_rgb_scale = !is_luminance && tc_astc_block_is_rgb_scale(block, count);
    uint32_t endpoint_format = is_luminance && is_opaque
                                   ? 0u
                                   : (is_luminance
                                          ? 4u
                                          : (is_rgb_scale ? (is_opaque ? 6u : 10u)
                                                          : (is_opaque ? 8u : 12u)));
    uint32_t endpoint_value_count = endpoint_format == 0u
                                        ? 2u
                                        : (endpoint_format == 4u || endpoint_format == 6u
                                               ? 4u
                                               : (endpoint_format == 8u || endpoint_format == 10u ? 6u : 8u));
    uint32_t endpoint_end_bit = 17u + endpoint_value_count * 8u;
    uint32_t candidate_endpoint_end_bit = quality > 0 ? 17u : endpoint_end_bit;
    uint32_t color_quant_method = 20u;
    const tc_astc_block_mode_info *candidates;
    uint32_t candidate_count;
    uint16_t selected_candidates[64];
    uint64_t selected_errors[64];
    uint32_t selected_count = 0;
    uint32_t selected_limit =
        quality > 1
            ? 48u
            : (quality > 0
                   ? ((is_opaque && count == 36u)
                          ? TC_ASTC_MEDIUM_OPAQUE_SELECTED_LIMIT
                          : TC_ASTC_MEDIUM_SELECTED_LIMIT)
                   : 2u);
    uint64_t best_err = UINT64_MAX;

    uint8_t path_out[16];
    uint8_t cand_out[16];
    uint32_t axis_lo_i[5], axis_hi_i[5];
    uint32_t axis_lo[5][4], axis_hi[5][4], axis_recip[5];
    uint32_t scan_cap;
    uint8_t axis_ideal[5][144];
    uint8_t axis_ready[5] = {0, 0, 0, 0, 0};
    uint8_t axis_projected[5] = {0, 0, 0, 0, 0};
    int axis_valid[5];
    uint64_t path_err = UINT64_MAX, cand_err;
    int have_path = 0;
    int best_from_lsq = 0;

    memset(out, 0, 16u);
    memset(weightbuf, 0, 16u);
    memset(path_out, 0, 16u);
    /* Endpoint lines and projected ideal weights depend only on the axis, not
     * on the candidate's weight grid. The luma axis drives pass 1; RGB axes are
     * built lazily only if the reduced-effort tier reaches the full-axis sweep. */
    tc_astc_axis_extremes(block, count, 4u, &axis_lo_i[4], &axis_hi_i[4]);
    axis_valid[4] = tc_astc_axis_line(block, axis_lo_i[4], axis_hi_i[4],
                                      axis_lo[4], axis_hi[4], &axis_recip[4],
                                      NULL);
    axis_ready[4] = 1u;
    candidate_count = tc_astc_get_candidates(ctx, candidate_endpoint_end_bit, &candidates);
    /* Reduced-effort levels only fit the top-ranked viable candidates
     * (candidates whose color budget cannot fit do not use up the cap). */
    scan_cap =
        quality > 1
            ? candidate_count
            : (quality > 0
                   ? ((is_opaque && count == 36u) ? TC_ASTC_MEDIUM_OPAQUE_SCAN_CAP
                                                  : TC_ASTC_MEDIUM_SCAN_CAP)
                   : 3u);
    if (selected_limit > candidate_count) selected_limit = candidate_count;

    if (axis_valid[4]) {
        uint32_t scanned = 0;
        tc_astc_project_ideal(block, count, axis_lo[4], axis_hi[4],
                              axis_recip[4], axis_ideal[4]);
        axis_projected[4] = 1u;
        /* Batch-scoring loop: group consecutive candidates by shared decim
         * entry and quant method, then score the group in one call. */
        for (ci = 0; ci < candidate_count && scanned < scan_cap; ) {
            const tc_astc_decim_cache_entry *batch_decim = NULL;
            uint32_t batch_n = 0, batch_qm = 0, bj, pos;
            /* Fast path: quality>1 uses exact fit (per-candidate, can't batch) */
            if (quality > 1) {
                uint32_t cand_color_quant;
                uint64_t err;
                if (quality > 0) {
                    uint32_t color_bits_available = 111u - candidates[ci].weight_bits;
                    if (candidates[ci].weight_bits >= 111u ||
                        !tc_astc_lut_color_quant(ctx, endpoint_value_count,
                                                 color_bits_available, &cand_color_quant)) {
                        ++ci; continue;
                    }
                }
                batch_decim = tc_astc_get_decim_cache(ctx, candidates[ci].weight_x,
                                                      candidates[ci].weight_y);
                if (!batch_decim) { ++ci; continue; }
                ++scanned;
                {
                    uint8_t cand_weights[64];
                    err = tc_astc_fit_from_ideal(
                        ctx, block, count, axis_ideal[4], axis_lo[4], axis_hi[4],
                        candidates[ci].quant_method, batch_decim,
                        (uint32_t)candidates[ci].weight_x * candidates[ci].weight_y,
                        cand_weights, NULL);
                }
                if (selected_count < selected_limit) pos = selected_count++;
                else if (selected_count && err < selected_errors[selected_count - 1u]) pos = selected_count - 1u;
                else { ++ci; continue; }
                while (pos > 0u && err < selected_errors[pos - 1u]) { selected_errors[pos] = selected_errors[pos - 1u]; selected_candidates[pos] = selected_candidates[pos - 1u]; --pos; }
                selected_errors[pos] = err; selected_candidates[pos] = (uint16_t)ci;
                ++ci;
                continue;
            }
            /* Medium/fast: batch-score consecutive candidates with same decim+qm */
            {
                uint32_t batch_start = ci;
                while (ci < candidate_count && scanned + batch_n < scan_cap) {
                    uint32_t cand_color_quant;
                    const tc_astc_decim_cache_entry *d;
                    if (quality > 0) {
                        uint32_t color_bits_available = 111u - candidates[ci].weight_bits;
                        if (candidates[ci].weight_bits >= 111u ||
                            !tc_astc_lut_color_quant(ctx, endpoint_value_count,
                                                     color_bits_available, &cand_color_quant)) {
                            ++ci; continue;
                        }
                    }
                    d = tc_astc_get_decim_cache(ctx, candidates[ci].weight_x,
                                                candidates[ci].weight_y);
                    if (!d) { ++ci; continue; }
                    if (!batch_decim) {
                        batch_decim = d;
                        batch_qm = candidates[ci].quant_method;
                    } else if (d != batch_decim || candidates[ci].quant_method != batch_qm) {
                        break;
                    }
                    ++batch_n; ++ci;
                }
                if (batch_n > 0) {
                    uint64_t batch_errs[32];
                    const uint8_t *batch_ideals[32];
                    uint32_t wc = (uint32_t)batch_decim->weight_x * batch_decim->weight_y;
                    for (bj = 0; bj < batch_n; ++bj)
                        batch_ideals[bj] = axis_ideal[4];
                    tc_astc_score_batch(ctx, count, batch_ideals, batch_n,
                                        batch_qm, batch_decim, wc, batch_errs);
                    for (bj = 0; bj < batch_n; ++bj) {
                        uint64_t err = batch_errs[bj];
                        uint32_t cj = batch_start + bj;
                        scanned++;
                        if (selected_count < selected_limit) pos = selected_count++;
                        else if (selected_count && err < selected_errors[selected_count - 1u]) pos = selected_count - 1u;
                        else continue;
                        while (pos > 0u && err < selected_errors[pos - 1u]) { selected_errors[pos] = selected_errors[pos - 1u]; selected_candidates[pos] = selected_candidates[pos - 1u]; --pos; }
                        selected_errors[pos] = err; selected_candidates[pos] = (uint16_t)cj;
                    }
                }
            }
        }
    }

    for (ci = 0; ci < selected_count; ++ci) {
        uint32_t candidate_index = selected_candidates[ci];
        const tc_astc_decim_cache_entry *decim;
        uint32_t cand_color_quant_method = 20u;
        if (quality > 0) {
            uint32_t color_bits_available =
                111u - candidates[candidate_index].weight_bits;
            if (candidates[candidate_index].weight_bits >= 111u ||
                !tc_astc_lut_color_quant(ctx, endpoint_value_count,
                                         color_bits_available,
                                         &cand_color_quant_method))
                continue;
        } else {
            cand_color_quant_method = 20u;
        }
        decim = tc_astc_get_decim_cache(ctx, candidates[candidate_index].weight_x,
                                        candidates[candidate_index].weight_y);
        if (!decim) continue;
        /* Channel-axis endpoint seeding is expensive; at medium quality
         * only the best-ranked shortlist entries get the full axis sweep,
         * the rest rely on least-squares refinement of the luma fit. */
        uint32_t medium_full_axis_limit =
            (is_opaque && count == 36u) ? TC_ASTC_MEDIUM_OPAQUE_FULL_AXIS_LIMIT
                                        : TC_ASTC_MEDIUM_FULL_AXIS_LIMIT;
        int full_axes =
            quality > 1 || (quality == 1 && ci < medium_full_axis_limit);
        for (axis = full_axes ? 0u : 4u; axis < 5u; ++axis) {
            uint32_t cand_lo[4], cand_hi[4], lsq_lo[4], lsq_hi[4];
            uint8_t cand_weights[64];
            uint8_t wt[144];
            uint64_t err;
            int cand_from_lsq = 0;
            if (!axis_ready[axis]) {
                if (axis < 4u) {
                    uint32_t a;
                    tc_astc_axis_extremes_rgba(block, count, axis_lo_i,
                                               axis_hi_i);
                    for (a = 0; a < 4u; ++a) {
                        axis_valid[a] = tc_astc_axis_line(
                            block, axis_lo_i[a], axis_hi_i[a], axis_lo[a],
                            axis_hi[a], &axis_recip[a], NULL);
                        axis_ready[a] = 1u;
                    }
                } else {
                    tc_astc_axis_extremes(block, count, axis, &axis_lo_i[axis],
                                          &axis_hi_i[axis]);
                    axis_valid[axis] =
                        tc_astc_axis_line(block, axis_lo_i[axis],
                                          axis_hi_i[axis], axis_lo[axis],
                                          axis_hi[axis], &axis_recip[axis],
                                          NULL);
                    axis_ready[axis] = 1u;
                }
            }
            if (!axis_valid[axis]) continue;
            if (axis_lo_i[axis] == axis_lo_i[4] &&
                axis_hi_i[axis] == axis_hi_i[4] && axis != 4u)
                continue; /* same endpoint pair as the luma axis */
            if (!axis_projected[axis]) {
                tc_astc_project_ideal(block, count, axis_lo[axis], axis_hi[axis],
                                      axis_recip[axis], axis_ideal[axis]);
                axis_projected[axis] = 1u;
            }
            memcpy(cand_lo, axis_lo[axis], sizeof(cand_lo));
            memcpy(cand_hi, axis_hi[axis], sizeof(cand_hi));
            /* Score with the endpoints as they will actually decode
             * (quantized), and try replacing the axis-extreme endpoints
             * with the least-squares solution for the fitted weights. */
            /* With an identity color quantizer and a direct endpoint
             * format the fit error already is the decoded error. */
            if (cand_color_quant_method != 20u ||
                (endpoint_format != 8u && endpoint_format != 12u)) {
                tc_astc_weights_from_ideal(
                    ctx, count, axis_ideal[axis],
                    candidates[candidate_index].quant_method, decim,
                    (uint32_t)candidates[candidate_index].weight_x *
                        candidates[candidate_index].weight_y,
                    cand_weights, wt);
                err = tc_astc_eval_single_sse(ctx, block, count, wt,
                                              endpoint_format, cand_lo,
                                              cand_hi,
                                              cand_color_quant_method);
            } else {
                err = tc_astc_fit_from_ideal(
                    ctx, block, count, axis_ideal[axis], cand_lo, cand_hi,
                    candidates[candidate_index].quant_method, decim,
                    (uint32_t)candidates[candidate_index].weight_x *
                        candidates[candidate_index].weight_y,
                    cand_weights, wt);
            }
            if (tc_astc_lsq_endpoints(block, count, wt, lsq_lo, lsq_hi)) {
                uint64_t lsq_err = tc_astc_eval_single_sse(
                    ctx, block, count, wt, endpoint_format, lsq_lo, lsq_hi,
                    cand_color_quant_method);
                if (lsq_err < err) {
                    err = lsq_err;
                    memcpy(cand_lo, lsq_lo, sizeof(cand_lo));
                    memcpy(cand_hi, lsq_hi, sizeof(cand_hi));
                    cand_from_lsq = 1;
                }
            }
            if (err < best_err) {
                best_err = err;
                best_block_mode = candidates[candidate_index].block_mode;
                best_quant_method = candidates[candidate_index].quant_method;
                best_weight_count =
                    (uint32_t)candidates[candidate_index].weight_x *
                    candidates[candidate_index].weight_y;
                color_quant_method = cand_color_quant_method;
                best_decim = decim;
                best_from_lsq = cand_from_lsq;
                memcpy(lo, cand_lo, sizeof(lo));
                memcpy(hi, cand_hi, sizeof(hi));
                memcpy(weights, cand_weights, best_weight_count);
            }
            if (!full_axes) break;
        }
    }

    /* Iterative refinement, on the winner only (astcenc's
     * ideal-endpoints-and-weights loop): re-project the texels onto the
     * refined endpoint line, refit the weights to that projection, then
     * re-solve the endpoints; accept while the decoded error improves. */
    if (best_err != UINT64_MAX && best_decim && best_from_lsq) {
        uint32_t round, refine_rounds =
                            quality > 1 ? 2u
                                        : (quality > 0
                                               ? TC_ASTC_MEDIUM_REFINE_ROUNDS
                                               : 2u);
        for (round = 0; round < refine_rounds; ++round) {
            uint8_t it_ideal[144], it_weights[64];
            uint8_t wt[144];
            uint32_t it_lo[4], it_hi[4];
            uint32_t denom = 0, recip;
            uint64_t it_err;
            int improved = 0;
            for (c = 0; c < 4u; ++c) {
                uint32_t dd = hi[c] > lo[c] ? hi[c] - lo[c] : lo[c] - hi[c];
                denom += dd * dd;
            }
            if (!denom) break;
            recip = TC_ASTC_PROJ_RECIP(denom);
            tc_astc_project_ideal(block, count, lo, hi, recip, it_ideal);
            tc_astc_weights_from_ideal(ctx, count, it_ideal, best_quant_method,
                                       best_decim, best_weight_count,
                                       it_weights, wt);
            it_err = tc_astc_eval_single_sse(ctx, block, count, wt,
                                             endpoint_format, lo, hi,
                                             color_quant_method);
            if (it_err < best_err) {
                best_err = it_err;
                memcpy(weights, it_weights, best_weight_count);
                improved = 1;
            }
            if (tc_astc_lsq_endpoints(block, count, wt, it_lo, it_hi)) {
                uint64_t lsq_err = tc_astc_eval_single_sse(
                    ctx, block, count, wt, endpoint_format, it_lo, it_hi,
                    color_quant_method);
                if (lsq_err < best_err) {
                    best_err = lsq_err;
                    memcpy(lo, it_lo, sizeof(lo));
                    memcpy(hi, it_hi, sizeof(hi));
                    memcpy(weights, it_weights, best_weight_count);
                    improved = 1;
                }
            }
            if (!improved) break;
        }
    }

    /* Partition and dual-plane searches run only when the single-partition
     * result still has meaningful error (astcenc's early-out): smooth
     * blocks skip the whole partition machinery. Fewer partitions are
     * tried first and win ties (more bits left for color precision). */
    {
        uint32_t partition_err_scale =
            (quality > 1)
                ? 4u
                : (quality > 0
                       ? ((is_opaque && count == 36u)
                              ? TC_ASTC_MEDIUM_OPAQUE_PARTITION_ERR_SCALE
                              : TC_ASTC_MEDIUM_PARTITION_ERR_SCALE)
                       : 256u);
    if (best_err > (uint64_t)count * partition_err_scale) {
        if (quality > 0 &&
            tc_encode_astc_partition_rgb_block(block, count, 2u, ctx, cand_out,
                                               &cand_err) &&
            cand_err < path_err) {
            path_err = cand_err;
            memcpy(path_out, cand_out, 16u);
            have_path = 1;
        }
        if (quality > 1 &&
            tc_encode_astc_partition_rgb_block(block, count, 3u, ctx, cand_out,
                                               &cand_err) &&
            cand_err < path_err) {
            path_err = cand_err;
            memcpy(path_out, cand_out, 16u);
            have_path = 1;
        }
        if (quality > 1 &&
            tc_encode_astc_partition_rgb_block(block, count, 4u, ctx, cand_out,
                                               &cand_err) &&
            cand_err < path_err) {
            path_err = cand_err;
            memcpy(path_out, cand_out, 16u);
            have_path = 1;
        }
        if (tc_encode_astc_dual_rgba_block(block, count, quality, ctx,
                                           cand_out, &cand_err) &&
            cand_err < path_err) {
            path_err = cand_err;
            memcpy(path_out, cand_out, 16u);
            have_path = 1;
        }
    }
    }

    if (best_err == UINT64_MAX) {
        uint32_t sum[4] = {0, 0, 0, 0};
        if (have_path) {
            memcpy(out, path_out, 16u);
            return;
        }
        for (i = 0; i < count; ++i) {
            for (c = 0; c < 4u; ++c) sum[c] += block[i][c];
        }
        tc_astc_write_const_from_sum(sum, count, out);
        return;
    }
    if (have_path && path_err < best_err) {
        memcpy(out, path_out, 16u);
        return;
    }

    values[0] = (uint8_t)lo[0];
    values[1] = (uint8_t)hi[0];
    if (endpoint_format == 4u) {
        values[2] = (uint8_t)lo[3];
        values[3] = (uint8_t)hi[3];
    } else if (endpoint_format == 6u || endpoint_format == 10u) {
        uint32_t sum_lo = lo[0] + lo[1] + lo[2];
        uint32_t sum_hi = hi[0] + hi[1] + hi[2];
        uint32_t scale = sum_hi ? (sum_lo * 256u + sum_hi / 2u) / sum_hi : 0u;
        if (scale > 255u) scale = 255u;
        values[0] = (uint8_t)hi[0];
        values[1] = (uint8_t)hi[1];
        values[2] = (uint8_t)hi[2];
        values[3] = (uint8_t)scale;
        if (endpoint_format == 10u) {
            values[4] = (uint8_t)lo[3];
            values[5] = (uint8_t)hi[3];
        }
    } else if (endpoint_format != 0u) {
        values[2] = (uint8_t)lo[1];
        values[3] = (uint8_t)hi[1];
        values[4] = (uint8_t)lo[2];
        values[5] = (uint8_t)hi[2];
        if (endpoint_format == 12u) {
            values[6] = (uint8_t)lo[3];
            values[7] = (uint8_t)hi[3];
        }
    }
    /* CEM 8/12: a decoder swaps the endpoints and applies blue-contract when
     * the first endpoint's quantized RGB sum is larger, so emit the pair in
     * ascending-sum order and mirror the weights to compensate. */
    if (endpoint_format == 8u || endpoint_format == 12u) {
        uint32_t s_lo = tc_astc_color_roundtrip(ctx, color_quant_method, values[0]) +
                        tc_astc_color_roundtrip(ctx, color_quant_method, values[2]) +
                        tc_astc_color_roundtrip(ctx, color_quant_method, values[4]);
        uint32_t s_hi = tc_astc_color_roundtrip(ctx, color_quant_method, values[1]) +
                        tc_astc_color_roundtrip(ctx, color_quant_method, values[3]) +
                        tc_astc_color_roundtrip(ctx, color_quant_method, values[5]);
        if (s_lo > s_hi) {
            uint32_t pair_count = endpoint_format == 12u ? 4u : 3u;
            for (i = 0; i < pair_count; ++i) {
                uint8_t t = values[i * 2u];
                values[i * 2u] = values[i * 2u + 1u];
                values[i * 2u + 1u] = t;
            }
            tc_astc_invert_weights(weights, best_weight_count,
                                   best_quant_method);
        }
    }

    tc_astc_scramble_weights(weights, best_weight_count, best_quant_method);
    (void)tc_astc_ise_encode_bits(best_quant_method, best_weight_count, weights,
                                  weightbuf, sizeof(weightbuf), 0u);
    for (i = 0; i < 16u; ++i) out[i] = tc_bitrev8(weightbuf[15u - i]);

    bitpos = 0;
    tc_set_bits(out, &bitpos, best_block_mode, 11u);
    tc_set_bits(out, &bitpos, 0u, 2u);   /* one partition. */
    tc_set_bits(out, &bitpos, endpoint_format, 4u);

    {
        uint8_t packed_values[8];
        tc_astc_quantize_color_values(ctx, color_quant_method,
                                      endpoint_value_count, values,
                                      packed_values);
        (void)tc_astc_ise_encode_bits(color_quant_method, endpoint_value_count,
                                      packed_values, out, 16u, 17u);
    }
}

/* --- ASTC HDR CEM 11 single-subset block encoder (UASTC HDR 4x4) ---------
 * Reuses the LDR block machinery (block-mode tables, weight quant/scramble,
 * ISE codec, bit layout). Fixed config: 4x4 weight grid, weight range 8
 * (3-bit), single partition, CEM 11 (HDR RGB direct) endpoints stored at
 * colour quant 256 (so the 8-bit majcomp==3 endpoint bytes survive exactly:
 * colour bits available = 111 - 48 weight bits = 63 >= 48, and 256 is the max
 * colour quant). Endpoints come from the LNS-domain bounding box; each texel's
 * weight is its projection onto the endpoint line. Input `lns` is per-texel
 * 16-bit LNS RGB (see tc_astc_float_to_lns16). */
static uint32_t tc_astc_hdr_find_block_mode_4x4(void) {
    uint32_t bm;
    for (bm = 0; bm < 2048u; ++bm) {
        tc_astc_block_mode_info info;
        if (tc_astc_decode_block_mode_2d(bm, &info) && info.weight_x == 4u &&
            info.weight_y == 4u && info.dual_plane == 0u &&
            info.quant_method == 5u)
            return bm;
    }
    return 0;
}

/* Shared read-only encode context for the HDR path (colour quant LUTs). */
static tc_astc_encode_context tc_hdr_ctx;
static int tc_hdr_ctx_ready = 0;
static void tc_hdr_ctx_ensure(void) {
    if (!tc_hdr_ctx_ready) {
        tc_astc_encode_context_init(&tc_hdr_ctx, 4u, 4u, 0);
        tc_hdr_ctx_ready = 1;
    }
}

/* quant_color equivalent: value as the decoder reconstructs it at a colour
 * quant level. Exposed so the CEM 11 endpoint packer (texcomp_astc_hdr.c) can
 * quantise endpoints to sub-256 levels (needed for 2-subset budgets). */
int tc_astc_hdr_color_roundtrip(uint32_t level, int value) {
    if (value < 0) value = 0;
    if (value > 255) value = 255;
    if (level >= 20u) return value; /* 256 levels == identity */
    tc_hdr_ctx_ensure();
    return (int)tc_astc_color_roundtrip(&tc_hdr_ctx, level, (uint32_t)value);
}

/* 4x4 single-plane grid at a given weight quant_method. */
static uint32_t tc_astc_hdr_find_grid_mode(uint32_t qm) {
    uint32_t bm;
    for (bm = 0; bm < 2048u; ++bm) {
        tc_astc_block_mode_info info;
        if (tc_astc_decode_block_mode_2d(bm, &info) && info.weight_x == 4u &&
            info.weight_y == 4u && info.dual_plane == 0u &&
            info.quant_method == qm)
            return bm;
    }
    return 0;
}

/* 4x4 dual-plane grid at a given weight quant_method. */
static uint32_t tc_astc_find_dual_grid_mode(uint32_t qm) {
    uint32_t bm;
    for (bm = 0; bm < 2048u; ++bm) {
        tc_astc_block_mode_info info;
        if (tc_astc_decode_block_mode_2d(bm, &info) && info.weight_x == 4u &&
            info.weight_y == 4u && info.dual_plane == 1u &&
            info.quant_method == qm)
            return bm;
    }
    return 0;
}
/* HDR dual uses weight range 3 (trit, quant_method 1). */
static uint32_t tc_astc_hdr_find_dual_block_mode(void) {
    return tc_astc_find_dual_grid_mode(1u);
}

/* Quantise an ideal [0,64] weight to the nearest symbol of a weight range. */
static uint8_t tc_astc_hdr_quant_weight(int w64, uint32_t qm, uint32_t levels) {
    uint32_t q, best = 0, bestd = 1u << 30;
    for (q = 0; q < levels; ++q) {
        int d = (int)tc_astc_weight_unquant(q, qm) - w64;
        uint32_t ad = (uint32_t)(d < 0 ? -d : d);
        if (ad < bestd) {
            bestd = ad;
            best = q;
        }
    }
    return (uint8_t)best;
}

/* Least-squares HDR endpoints in the LNS domain: given per-texel interpolation
 * weights wt[i] (0..64), solve the two LNS endpoints per channel that minimize
 * the LNS-domain reconstruction error (the same 2x2 normal equations as the LDR
 * solver, but over 16-bit LNS values). Endpoints are clamped to [0,65535].
 * A degenerate (zero-determinant) channel keeps its incoming endpoints. */
static void tc_astc_hdr_lsq(const int lns[16][3], const uint8_t wt[16],
                            int e0[3], int e1[3]) {
    int c, i;
    for (c = 0; c < 3; ++c) {
        int64_t saa = 0, sab = 0, sbb = 0, sap = 0, sbp = 0, det, l, h;
        for (i = 0; i < 16; ++i) {
            int64_t a = 64 - wt[i], b = wt[i], p = lns[i][c];
            saa += a * a;
            sab += a * b;
            sbb += b * b;
            sap += a * p;
            sbp += b * p;
        }
        det = saa * sbb - sab * sab;
        if (det <= 0) continue;
        l = tc_div_round_s64((sap * sbb - sbp * sab) * 64, det);
        h = tc_div_round_s64((sbp * saa - sap * sab) * 64, det);
        if (l < 0) l = 0;
        if (l > 65535) l = 65535;
        if (h < 0) h = 0;
        if (h > 65535) h = 65535;
        e0[c] = (int)l;
        e1[c] = (int)h;
    }
}

/* 4-channel (RGBA) LNS least-squares, for the CEM 15 HDR+alpha path. Same 2x2
 * normal equations as tc_astc_hdr_lsq, over all four LNS channels. */
static void tc_astc_hdr_lsq4(const int lns[16][4], const uint8_t wt[16],
                             int e0[4], int e1[4]) {
    int c, i;
    for (c = 0; c < 4; ++c) {
        int64_t saa = 0, sab = 0, sbb = 0, sap = 0, sbp = 0, det, l, h;
        for (i = 0; i < 16; ++i) {
            int64_t a = 64 - wt[i], b = wt[i], p = lns[i][c];
            saa += a * a;
            sab += a * b;
            sbb += b * b;
            sap += a * p;
            sbp += b * p;
        }
        det = saa * sbb - sab * sab;
        if (det <= 0) continue;
        l = tc_div_round_s64((sap * sbb - sbp * sab) * 64, det);
        h = tc_div_round_s64((sbp * saa - sap * sab) * 64, det);
        if (l < 0) l = 0;
        if (l > 65535) l = 65535;
        if (h < 0) h = 0;
        if (h > 65535) h = 65535;
        e0[c] = (int)l;
        e1[c] = (int)h;
    }
}

/* Partition-aware LNS least-squares: as tc_astc_hdr_lsq, but summing only the
 * texels assigned to `subset` (for the 2-subset HDR path). */
static void tc_astc_hdr_lsq_part(const int lns[16][3], const uint8_t wt[16],
                                 const uint8_t *pot, int subset, int e0[3],
                                 int e1[3]) {
    int c, i;
    for (c = 0; c < 3; ++c) {
        int64_t saa = 0, sab = 0, sbb = 0, sap = 0, sbp = 0, det, l, h;
        for (i = 0; i < 16; ++i) {
            int64_t a, b, p;
            if (pot[i] != subset) continue;
            a = 64 - wt[i];
            b = wt[i];
            p = lns[i][c];
            saa += a * a;
            sab += a * b;
            sbb += b * b;
            sap += a * p;
            sbp += b * p;
        }
        det = saa * sbb - sab * sab;
        if (det <= 0) continue;
        l = tc_div_round_s64((sap * sbb - sbp * sab) * 64, det);
        h = tc_div_round_s64((sbp * saa - sap * sab) * 64, det);
        if (l < 0) l = 0;
        if (l > 65535) l = 65535;
        if (h < 0) h = 0;
        if (h > 65535) h = 65535;
        e0[c] = (int)l;
        e1[c] = (int)h;
    }
}

uint64_t tc_encode_astc_hdr_cem11_block(const int lns[16][3], uint8_t out[16]) {
    static uint32_t bm_single = 0xffffffffu, bm_dual = 0xffffffffu;
    uint8_t weightbuf[16], packed[8], sw[16];
    uint8_t v[8];
    int e0[3], e1[3], qe0[3], qe1[3], dir[3];
    int i, c, ccs;
    int64_t sse_single = 0, best_dual_sse = -1;
    int best_ccs = -1;
    uint8_t dp1[16], dp2[16];
    uint32_t bitpos;

    if (bm_single == 0xffffffffu) bm_single = tc_astc_hdr_find_block_mode_4x4();
    if (bm_dual == 0xffffffffu) bm_dual = tc_astc_hdr_find_dual_block_mode();
    tc_hdr_ctx_ensure();

    /* endpoints: per-channel LNS bounding box (min -> e0, max -> e1). */
    for (c = 0; c < 3; ++c) {
        e0[c] = lns[0][c];
        e1[c] = lns[0][c];
    }
    for (i = 1; i < 16; ++i)
        for (c = 0; c < 3; ++c) {
            if (lns[i][c] < e0[c]) e0[c] = lns[i][c];
            if (lns[i][c] > e1[c]) e1[c] = lns[i][c];
        }

    /* Single-plane fit with LNS least-squares refinement: round 0 uses the
     * bbox endpoints; each further round re-solves the endpoints against the
     * current weights, re-packs, and keeps the lowest-SSE quantized result. */
    {
        uint8_t best_v[8] = {0}, best_sw[16] = {0};
        int64_t best_sse = -1;
        int round;
        for (round = 0; round < 3; ++round) {
            uint8_t wt[16];
            int64_t sse = 0, dl = 0;
            int q0[3], q1[3], d[3];
            tc_astc_cem11_pack(e0, e1, 20, v);
            tc_astc_cem11_unpack(v, q0, q1);
            for (c = 0; c < 3; ++c) {
                d[c] = q1[c] - q0[c];
                dl += (int64_t)d[c] * d[c];
            }
            for (i = 0; i < 16; ++i) {
                int w64 = 0, uq;
                if (dl > 0) {
                    int64_t dot = 0;
                    for (c = 0; c < 3; ++c)
                        dot += ((int64_t)lns[i][c] - q0[c]) * d[c];
                    if (dot < 0) dot = 0;
                    if (dot > dl) dot = dl;
                    w64 = (int)((dot * 64 + dl / 2) / dl);
                }
                sw[i] = tc_astc_hdr_quant_weight(w64, 5u, 8u);
                uq = (int)tc_astc_weight_unquant(sw[i], 5u);
                wt[i] = (uint8_t)uq;
                for (c = 0; c < 3; ++c) {
                    int64_t rec = q0[c] + ((int64_t)d[c] * uq) / 64;
                    int64_t e = rec - lns[i][c];
                    sse += e * e;
                }
            }
            if (best_sse < 0 || sse < best_sse) {
                best_sse = sse;
                memcpy(best_v, v, sizeof(best_v));
                memcpy(best_sw, sw, sizeof(best_sw));
            } else {
                break;
            }
            if (round + 1 < 3) tc_astc_hdr_lsq(lns, wt, e0, e1);
        }
        memcpy(v, best_v, sizeof(v));
        memcpy(sw, best_sw, sizeof(sw));
        sse_single = best_sse;
    }
    /* qe0/qe1/dir for the dual-plane path use the winning single endpoints. */
    tc_astc_cem11_unpack(v, qe0, qe1);
    for (c = 0; c < 3; ++c) {
        dir[c] = qe1[c] - qe0[c];
    }

    /* dual plane: one channel gets its own weight plane. Try each candidate
     * separate channel; weights are coarser (range 3 / trit) because the two
     * interleaved planes cost ~2x the weight bits, but decoupling a channel
     * wins big on anti-correlated / single-channel-independent blocks. */
    for (ccs = 0; ccs < 3; ++ccs) {
        int64_t d1 = 0, d2 = (int64_t)dir[ccs] * dir[ccs], dsse = 0;
        uint8_t t1[16], t2[16];
        for (c = 0; c < 3; ++c)
            if (c != ccs) d1 += (int64_t)dir[c] * dir[c];
        for (i = 0; i < 16; ++i) {
            int w1 = 0, w2 = 0, u1, u2;
            if (d1 > 0) {
                int64_t dot = 0;
                for (c = 0; c < 3; ++c)
                    if (c != ccs)
                        dot += ((int64_t)lns[i][c] - qe0[c]) * dir[c];
                if (dot < 0) dot = 0;
                if (dot > d1) dot = d1;
                w1 = (int)((dot * 64 + d1 / 2) / d1);
            }
            if (d2 > 0) {
                int64_t dot = ((int64_t)lns[i][ccs] - qe0[ccs]) * dir[ccs];
                if (dot < 0) dot = 0;
                if (dot > d2) dot = d2;
                w2 = (int)((dot * 64 + d2 / 2) / d2);
            }
            t1[i] = tc_astc_hdr_quant_weight(w1, 1u, 3u);
            t2[i] = tc_astc_hdr_quant_weight(w2, 1u, 3u);
            u1 = (int)tc_astc_weight_unquant(t1[i], 1u);
            u2 = (int)tc_astc_weight_unquant(t2[i], 1u);
            for (c = 0; c < 3; ++c) {
                int uq = (c == ccs) ? u2 : u1;
                int64_t rec = qe0[c] + ((int64_t)dir[c] * uq) / 64;
                int64_t e = rec - lns[i][c];
                dsse += e * e;
            }
        }
        if (best_ccs < 0 || dsse < best_dual_sse) {
            best_dual_sse = dsse;
            best_ccs = ccs;
            memcpy(dp1, t1, 16u);
            memcpy(dp2, t2, 16u);
        }
    }

    memset(out, 0, 16u);
    memset(weightbuf, 0, sizeof(weightbuf));

    if (best_ccs >= 0 && best_dual_sse < sse_single) {
        uint8_t dualw[32];
        uint32_t wbits = tc_astc_ise_sequence_bitcount(32u, 1u);
        tc_astc_scramble_weights(dp1, 16u, 1u);
        tc_astc_scramble_weights(dp2, 16u, 1u);
        for (i = 0; i < 16; ++i) {
            dualw[i * 2] = dp1[i];      /* plane 0 (shared) -> even */
            dualw[i * 2 + 1] = dp2[i];  /* plane 1 (ccs)    -> odd  */
        }
        (void)tc_astc_ise_encode_bits(1u, 32u, dualw, weightbuf,
                                      sizeof(weightbuf), 0u);
        for (i = 0; i < 16; ++i) out[i] = tc_bitrev8(weightbuf[15 - i]);
        bitpos = 0;
        tc_set_bits(out, &bitpos, bm_dual, 11u);
        tc_set_bits(out, &bitpos, 0u, 2u);
        tc_set_bits(out, &bitpos, 11u, 4u);
        bitpos = 128u - wbits - 2u; /* colour component selector below weights */
        tc_set_bits(out, &bitpos, (uint32_t)best_ccs, 2u);
        tc_astc_quantize_color_values(&tc_hdr_ctx, 20u, 6u, v, packed);
        (void)tc_astc_ise_encode_bits(20u, 6u, packed, out, 16u, 17u);
        return (uint64_t)best_dual_sse;
    }

    tc_astc_scramble_weights(sw, 16u, 5u);
    (void)tc_astc_ise_encode_bits(5u, 16u, sw, weightbuf, sizeof(weightbuf), 0u);
    for (i = 0; i < 16; ++i) out[i] = tc_bitrev8(weightbuf[15 - i]);
    bitpos = 0;
    tc_set_bits(out, &bitpos, bm_single, 11u);
    tc_set_bits(out, &bitpos, 0u, 2u);
    tc_set_bits(out, &bitpos, 11u, 4u);
    tc_astc_quantize_color_values(&tc_hdr_ctx, 20u, 6u, v, packed);
    (void)tc_astc_ise_encode_bits(20u, 6u, packed, out, 16u, 17u);
    return (uint64_t)sse_single;
}

/* Single-subset CEM 15 (HDR RGB direct + HDR alpha) block. CEM 15 stores 8
 * endpoint values (RGB direct = 6, HDR alpha = 2), so at colour quant 256 they
 * need 64 colour bits; that forces weight range 6 (QUANT_6, quant_method 4,
 * ~42 weight bits -> 111-42=69 colour bits available). Range 8 (48 weight bits)
 * would leave only 63, one short of 64, and the decoder would derive a lower
 * colour quant and corrupt the 8-bit endpoint bytes. Endpoints come from the
 * RGBA LNS bounding box with least-squares refinement; each texel's weight is
 * its projection onto the 4D endpoint line. Input `lns` is per-texel 16-bit LNS
 * RGBA (see tc_astc_float_to_lns16). Returns the LNS-domain SSE. */
uint64_t tc_encode_astc_hdr_cem15_block(const int lns[16][4], uint8_t out[16]) {
    static uint32_t bm = 0xffffffffu, dbm = 0xffffffffu;
    uint8_t weightbuf[16], packed[8], sw[16], v[8];
    int e0[4], e1[4];
    int i, c, round;
    int64_t best_sse = -1;
    uint8_t best_v[8] = {0}, best_sw[16] = {0};
    uint32_t bitpos;

    if (bm == 0xffffffffu) bm = tc_astc_hdr_find_grid_mode(4u); /* QUANT_6 */
    if (dbm == 0xffffffffu) dbm = tc_astc_find_dual_grid_mode(1u); /* QUANT_3 trit */
    tc_hdr_ctx_ensure();

    for (c = 0; c < 4; ++c) {
        e0[c] = lns[0][c];
        e1[c] = lns[0][c];
    }
    for (i = 1; i < 16; ++i)
        for (c = 0; c < 4; ++c) {
            if (lns[i][c] < e0[c]) e0[c] = lns[i][c];
            if (lns[i][c] > e1[c]) e1[c] = lns[i][c];
        }

    for (round = 0; round < 3; ++round) {
        uint8_t wt[16];
        int64_t sse = 0, dl = 0;
        int q0[4], q1[4], d[4];
        tc_astc_cem15_pack(e0, e1, e0[3], e1[3], 20, v);
        tc_astc_cem15_unpack(v, q0, q1);
        for (c = 0; c < 4; ++c) {
            d[c] = q1[c] - q0[c];
            dl += (int64_t)d[c] * d[c];
        }
        for (i = 0; i < 16; ++i) {
            int w64 = 0, uq;
            if (dl > 0) {
                int64_t dot = 0;
                for (c = 0; c < 4; ++c)
                    dot += ((int64_t)lns[i][c] - q0[c]) * d[c];
                if (dot < 0) dot = 0;
                if (dot > dl) dot = dl;
                w64 = (int)((dot * 64 + dl / 2) / dl);
            }
            sw[i] = tc_astc_hdr_quant_weight(w64, 4u, 6u);
            uq = (int)tc_astc_weight_unquant(sw[i], 4u);
            wt[i] = (uint8_t)uq;
            for (c = 0; c < 4; ++c) {
                int64_t rec = q0[c] + ((int64_t)d[c] * uq) / 64;
                int64_t e = rec - lns[i][c];
                sse += e * e;
            }
        }
        if (best_sse < 0 || sse < best_sse) {
            best_sse = sse;
            memcpy(best_v, v, sizeof(best_v));
            memcpy(best_sw, sw, sizeof(best_sw));
        } else {
            break;
        }
        if (round + 1 < 3) tc_astc_hdr_lsq4(lns, wt, e0, e1);
    }

    /* Dual-plane path: CCS=3 (alpha independent plane), qm=1 (trit, 3 levels).
     * The CEM 15 single-plane path above (qm=4) has finer weights (6 levels) but
     * shares one plane for RGBA; dual-plane gives alpha its own weight at coarser
     * quant (3 levels). Only try if the SSE might improve (alpha-heavy blocks). */
    {
        uint8_t dualw[32], dual_packed[8];
        int dq0[4], dq1[4], dd[4];
        int64_t ddl = 0, dual_sse;
        tc_astc_cem15_pack(e0, e1, e0[3], e1[3], 20, v);
        tc_astc_cem15_unpack(v, dq0, dq1);
        for (c = 0; c < 4; ++c) { dd[c] = dq1[c] - dq0[c]; ddl += (int64_t)dd[c] * dd[c]; }
        memset(dualw, 0, sizeof(dualw));
        if (ddl > 0) {
            for (i = 0; i < 16; ++i) {
                int64_t dot_rgb = 0, dot_a = 0;
                int c2;
                for (c2 = 0; c2 < 3; ++c2)
                    dot_rgb += ((int64_t)lns[i][c2] - dq0[c2]) * dd[c2];
                dot_a = ((int64_t)lns[i][3] - dq0[3]) * dd[3];
                { /* w0 = RGB weight */
                    int64_t d = dot_rgb;
                    if (d < 0) d = 0;
                    if (d > ddl) d = ddl;
                    dualw[i * 2] = tc_astc_hdr_quant_weight((int)((d * 64 + ddl / 2) / ddl), 1u, 3u);
                }
                { /* w1 = alpha weight */
                    int64_t d = dot_a;
                    if (d < 0) d = 0;
                    if (d > ddl) d = ddl;
                    dualw[i * 2 + 1] = tc_astc_hdr_quant_weight((int)((d * 64 + ddl / 2) / ddl), 1u, 3u);
                }
            }
            dual_sse = 0;
            for (i = 0; i < 16; ++i) {
                int uq0 = (int)tc_astc_weight_unquant(dualw[i * 2], 1u);
                int uq1 = (int)tc_astc_weight_unquant(dualw[i * 2 + 1], 1u);
                for (c = 0; c < 3; ++c) {
                    int64_t rec = dq0[c] + ((int64_t)dd[c] * uq0) / 64;
                    int64_t e = rec - lns[i][c];
                    dual_sse += e * e;
                }
                {
                    int64_t rec = dq0[3] + ((int64_t)dd[3] * uq1) / 64;
                    int64_t e = rec - lns[i][3];
                    dual_sse += e * e;
                }
            }
            if (dual_sse < best_sse) {
                best_sse = dual_sse;
                memcpy(best_v, v, sizeof(best_v));
                memset(out, 0, 16u);
                memset(weightbuf, 0, sizeof(weightbuf));
                tc_astc_scramble_weights(dualw, 32u, 1u);
                (void)tc_astc_ise_encode_bits(1u, 32u, dualw, weightbuf, sizeof(weightbuf), 0u);
                for (i = 0; i < 16; ++i) out[i] = tc_bitrev8(weightbuf[15 - i]);
                bitpos = 0;
                tc_set_bits(out, &bitpos, dbm, 11u);
                tc_set_bits(out, &bitpos, 0u, 2u);
                tc_set_bits(out, &bitpos, 15u, 4u); /* CEM 15 */
                tc_astc_quantize_color_values(&tc_hdr_ctx, 20u, 8u, v, dual_packed);
                (void)tc_astc_ise_encode_bits(20u, 8u, dual_packed, out, 16u, 17u);
                /* CCS stored at 128 - weight_bits - 2 (weight_bits for 32 syms at qm1) */
                bitpos = 128u - tc_astc_ise_sequence_bitcount(32u, 1u) - 2u;
                tc_set_bits(out, &bitpos, 3u, 2u); /* CCS = 3 (alpha) */
                return (uint64_t)best_sse;
            }
        }
    }

    memcpy(v, best_v, sizeof(v));
    memcpy(sw, best_sw, sizeof(sw));

    memset(out, 0, 16u);
    memset(weightbuf, 0, sizeof(weightbuf));
    tc_astc_scramble_weights(sw, 16u, 4u);
    (void)tc_astc_ise_encode_bits(4u, 16u, sw, weightbuf, sizeof(weightbuf), 0u);
    for (i = 0; i < 16; ++i) out[i] = tc_bitrev8(weightbuf[15 - i]);
    bitpos = 0;
    tc_set_bits(out, &bitpos, bm, 11u);
    tc_set_bits(out, &bitpos, 0u, 2u);
    tc_set_bits(out, &bitpos, 15u, 4u); /* CEM 15 */
    tc_astc_quantize_color_values(&tc_hdr_ctx, 20u, 8u, v, packed);
    (void)tc_astc_ise_encode_bits(20u, 8u, packed, out, 16u, 17u);
    return (uint64_t)best_sse;
}

/* Single-subset CEM 7 (HDR RGB base+scale) block. Fits base+scale to the LNS
 * bounding box, refines by least squares, and stores just 4 endpoint values at
 * colour quant 256 -- the 2 values saved vs CEM 11's direct RGB pay for a finer
 * 4x4 range-16 (4-bit) weight grid instead of CEM 11's range-8 (3-bit). So it
 * wins on smooth blocks whose two endpoints differ by a near-uniform luminance
 * shift, where the extra weight precision beats direct RGB endpoints. Returns
 * the LNS-domain SSE. */
#define TC_HDR_CEM7_WQM 8u /* weight quant_method 8 = range 16 (4-bit) */
uint64_t tc_encode_astc_hdr_cem7_block(const int lns[16][3], uint8_t out[16]) {
    static uint32_t bm_single = 0xffffffffu;
    uint8_t weightbuf[16], packed[8], sw[16], v[8];
    uint8_t best_v[8] = {0}, best_sw[16] = {0};
    int e0[3], e1[3], i, c, round;
    int64_t best_sse = -1;
    uint32_t bitpos;

    if (bm_single == 0xffffffffu)
        bm_single = tc_astc_hdr_find_grid_mode(TC_HDR_CEM7_WQM);
    if (!bm_single) return (uint64_t)-1;
    tc_hdr_ctx_ensure();

    for (c = 0; c < 3; ++c) {
        e0[c] = lns[0][c];
        e1[c] = lns[0][c];
    }
    for (i = 1; i < 16; ++i)
        for (c = 0; c < 3; ++c) {
            if (lns[i][c] < e0[c]) e0[c] = lns[i][c];
            if (lns[i][c] > e1[c]) e1[c] = lns[i][c];
        }
    for (round = 0; round < 3; ++round) {
        uint8_t wt[16];
        int64_t sse = 0, dl = 0;
        int q0[3], q1[3], d[3];
        tc_astc_cem7_pack(e0, e1, 20, v);
        tc_astc_cem7_unpack(v, q0, q1);
        for (c = 0; c < 3; ++c) {
            d[c] = q1[c] - q0[c];
            dl += (int64_t)d[c] * d[c];
        }
        for (i = 0; i < 16; ++i) {
            int w64 = 0, uq;
            if (dl > 0) {
                int64_t dot = 0;
                for (c = 0; c < 3; ++c)
                    dot += ((int64_t)lns[i][c] - q0[c]) * d[c];
                if (dot < 0) dot = 0;
                if (dot > dl) dot = dl;
                w64 = (int)((dot * 64 + dl / 2) / dl);
            }
            sw[i] = tc_astc_hdr_quant_weight(w64, TC_HDR_CEM7_WQM, 16u);
            uq = (int)tc_astc_weight_unquant(sw[i], TC_HDR_CEM7_WQM);
            wt[i] = (uint8_t)uq;
            for (c = 0; c < 3; ++c) {
                int64_t rec = q0[c] + ((int64_t)d[c] * uq) / 64;
                int64_t e = rec - lns[i][c];
                sse += e * e;
            }
        }
        if (best_sse < 0 || sse < best_sse) {
            best_sse = sse;
            memcpy(best_v, v, 4u);
            memcpy(best_sw, sw, 16u);
        } else {
            break;
        }
        if (round + 1 < 3) tc_astc_hdr_lsq(lns, wt, e0, e1);
    }
    memcpy(v, best_v, 4u);
    memcpy(sw, best_sw, 16u);

    memset(out, 0, 16u);
    memset(weightbuf, 0, sizeof(weightbuf));
    tc_astc_scramble_weights(sw, 16u, TC_HDR_CEM7_WQM);
    (void)tc_astc_ise_encode_bits(TC_HDR_CEM7_WQM, 16u, sw, weightbuf, sizeof(weightbuf), 0u);
    for (i = 0; i < 16; ++i) out[i] = tc_bitrev8(weightbuf[15 - i]);
    bitpos = 0;
    tc_set_bits(out, &bitpos, bm_single, 11u);
    tc_set_bits(out, &bitpos, 0u, 2u);
    tc_set_bits(out, &bitpos, 7u, 4u); /* CEM 7 */
    tc_astc_quantize_color_values(&tc_hdr_ctx, 20u, 4u, v, packed);
    (void)tc_astc_ise_encode_bits(20u, 4u, packed, out, 16u, 17u);
    return (uint64_t)best_sse;
}

/* Two-subset CEM 11 block. Each subset gets its own HDR endpoint pair (12
 * values total), stored at a lower colour quant (to fit the 4x4 budget) via
 * the quant-aware packer; a 10-bit partition index selects the region split.
 * Returns the reconstruction SSE, or UINT64_MAX if no usable partition/quant.
 * Layout: [11 mode][2 part-count][10 partition idx][6 multi-CEM][12-value
 * endpoint ISE @bit29][per-texel weights reversed from top]. */
uint64_t tc_encode_astc_hdr_cem11_2subset_block(const int lns[16][3],
                                                uint8_t out[16]) {
    static uint32_t bm = 0xffffffffu;
    uint8_t proxy[144][4];
    const tc_astc_partition_info *pi = NULL;
    uint8_t weightbuf[16], weights[16], ev[16], packed[16];
    int qe0[2][3], qe1[2][3];
    int i, c, s, cnt[2];
    uint32_t clevel, bitpos;
    uint64_t sse = 0;

    /* Weight range 4 (2-bit): each region is fairly uniform, so coarse weights
     * are cheap and the freed bits buy higher-precision endpoints (12 values
     * fit at a better colour quant). */
    if (bm == 0xffffffffu) bm = tc_astc_hdr_find_grid_mode(2u);
    tc_hdr_ctx_ensure();

    memset(proxy, 0, sizeof(proxy));
    for (i = 0; i < 16; ++i) {
        for (c = 0; c < 3; ++c) {
            int p = lns[i][c] >> 8;
            proxy[i][c] = (uint8_t)(p < 0 ? 0 : (p > 255 ? 255 : p));
        }
        proxy[i][3] = 255u;
    }
    if (!tc_astc_find_best_partition((const uint8_t(*)[4])proxy,
                                     tc_hdr_ctx.part2_cache,
                                     tc_hdr_ctx.part2_count, 2u, &tc_hdr_ctx,
                                     &pi))
        return (uint64_t)-1;
    /* colour quant for 12 endpoint values; weight_bits = 48 (4x4 range 8). */
    if (!tc_astc_lut_color_quant(&tc_hdr_ctx, 12u, 99u - 32u, &clevel))
        return (uint64_t)-1;

    /* Per-subset bbox endpoints, then per-subset LNS least-squares refinement
     * over a few rounds (keep the lowest-SSE quantized result). */
    {
        int rlo[2][3] = {{0, 0, 0}, {0, 0, 0}};
        int rhi[2][3] = {{0, 0, 0}, {0, 0, 0}};
        uint8_t best_ev[16] = {0}, best_w[16] = {0};
        int64_t best_sse = -1;
        int round;
        cnt[0] = cnt[1] = 0;
        for (s = 0; s < 2; ++s) {
            int have = 0;
            for (i = 0; i < 16; ++i) {
                if (pi->partition_of_texel[i] != s) continue;
                for (c = 0; c < 3; ++c) {
                    if (!have || lns[i][c] < rlo[s][c]) rlo[s][c] = lns[i][c];
                    if (!have || lns[i][c] > rhi[s][c]) rhi[s][c] = lns[i][c];
                }
                have = 1;
                cnt[s]++;
            }
        }
        if (cnt[0] == 0 || cnt[1] == 0) return (uint64_t)-1;
        for (round = 0; round < 3; ++round) {
            uint8_t wt[16];
            int64_t rsse = 0;
            for (s = 0; s < 2; ++s) {
                uint8_t vv[6];
                tc_astc_cem11_pack(rlo[s], rhi[s], (int)clevel, vv);
                tc_astc_cem11_unpack(vv, qe0[s], qe1[s]);
                for (c = 0; c < 6; ++c) ev[s * 6 + c] = vv[c];
            }
            for (i = 0; i < 16; ++i) {
                int s2 = pi->partition_of_texel[i], w64 = 0, uq, d[3];
                int64_t dl = 0, dot = 0;
                for (c = 0; c < 3; ++c) {
                    d[c] = qe1[s2][c] - qe0[s2][c];
                    dl += (int64_t)d[c] * d[c];
                }
                if (dl > 0) {
                    for (c = 0; c < 3; ++c)
                        dot += ((int64_t)lns[i][c] - qe0[s2][c]) * d[c];
                    if (dot < 0) dot = 0;
                    if (dot > dl) dot = dl;
                    w64 = (int)((dot * 64 + dl / 2) / dl);
                }
                weights[i] = tc_astc_hdr_quant_weight(w64, 2u, 4u);
                uq = (int)tc_astc_weight_unquant(weights[i], 2u);
                wt[i] = (uint8_t)uq;
                for (c = 0; c < 3; ++c) {
                    int64_t rec = qe0[s2][c] + ((int64_t)d[c] * uq) / 64;
                    int64_t e = rec - lns[i][c];
                    rsse += e * e;
                }
            }
            if (best_sse < 0 || rsse < best_sse) {
                best_sse = rsse;
                memcpy(best_ev, ev, 12u);
                memcpy(best_w, weights, 16u);
            } else {
                break;
            }
            if (round + 1 < 3)
                for (s = 0; s < 2; ++s)
                    tc_astc_hdr_lsq_part(lns, wt, pi->partition_of_texel, s,
                                         rlo[s], rhi[s]);
        }
        memcpy(ev, best_ev, 12u);
        memcpy(weights, best_w, 16u);
        sse = (uint64_t)best_sse;
    }

    memset(out, 0, 16u);
    memset(weightbuf, 0, sizeof(weightbuf));
    tc_astc_scramble_weights(weights, 16u, 2u);
    (void)tc_astc_ise_encode_bits(2u, 16u, weights, weightbuf, sizeof(weightbuf),
                                  0u);
    for (i = 0; i < 16; ++i) out[i] = tc_bitrev8(weightbuf[15 - i]);
    bitpos = 0;
    tc_set_bits(out, &bitpos, bm, 11u);
    tc_set_bits(out, &bitpos, 1u, 2u); /* partition count - 1 */
    tc_set_bits(out, &bitpos, (uint32_t)(pi->partition_index & 63u), 6u);
    tc_set_bits(out, &bitpos, (uint32_t)(pi->partition_index >> 6), 4u);
    bitpos = 23u;
    tc_set_bits(out, &bitpos, 11u << 2, 6u); /* multi-CEM: all-same, CEM 11 */
    tc_astc_quantize_color_values(&tc_hdr_ctx, clevel, 12u, ev, packed);
    (void)tc_astc_ise_encode_bits(clevel, 12u, packed, out, 16u, 29u);
    return sse;
}

/* --- UASTC LDR 4x4 (constrained ASTC LDR) -------------------------------
 * Encode a 4x4 block using only the UASTC LDR single-subset modes: CEM 8
 * (opaque RGB) modes 0/1/5/18 or CEM 12 (RGBA) modes 10/12/14, i.e. a fixed
 * 4x4 weight grid at weight range {8,5,2,11}/{8,5,2}, with the endpoint range
 * derived by the standard ASTC bit budget (which yields the UASTC endpoint
 * range for each mode). Constant blocks use the solid (void-extent) mode 8.
 * The multi-subset, dual-plane and CEM 4 modes are handled by
 * tc_uastc_encode_msubset / tc_uastc_encode_dual / tc_uastc_encode_cem4. */
static int tc_uastc_encode_mode(const uint8_t block[144][4], uint32_t count,
                                const tc_astc_encode_context *ctx, uint32_t cem,
                                uint32_t nv, uint32_t qm,
                                const uint32_t lo[4], const uint32_t hi[4],
                                uint8_t out[16], uint64_t *out_sse) {
    uint8_t weightbuf[16], values[8], sym[8], wsym[16];
    uint8_t best_sym[8], best_wsym[16];
    uint32_t qe0[4], qe1[4], er, weight_bits, color_bits, bm, bitpos;
    uint8_t wt[144];
    uint32_t nch = (cem == 12u) ? 4u : 3u;
    uint32_t rlo[4], rhi[4];
    uint64_t best = (uint64_t)-1;
    uint32_t i, c, round;

    bm = tc_astc_hdr_find_grid_mode(qm);
    if (!bm) return 0;
    weight_bits = tc_astc_ise_sequence_bitcount(16u, qm);
    if (weight_bits >= 111u) return 0;
    color_bits = 111u - weight_bits;
    if (!tc_astc_lut_color_quant(ctx, nv, color_bits, &er)) return 0;

    for (c = 0; c < 4u; ++c) {
        rlo[c] = lo[c];
        rhi[c] = hi[c];
    }
    /* Round 0 uses the bounding-box endpoints; each further round re-solves the
     * endpoints by least squares against the current weights (astcenc's
     * ideal-endpoints loop). Keep the lowest-SSE quantized result. */
    for (round = 0; round < 3u; ++round) {
        int dir[4];
        int64_t dl = 0;
        uint64_t sse;
        values[0] = (uint8_t)rlo[0];
        values[1] = (uint8_t)rhi[0];
        values[2] = (uint8_t)rlo[1];
        values[3] = (uint8_t)rhi[1];
        values[4] = (uint8_t)rlo[2];
        values[5] = (uint8_t)rhi[2];
        if (cem == 12u) {
            values[6] = (uint8_t)rlo[3];
            values[7] = (uint8_t)rhi[3];
        }
        tc_astc_quantize_color_values(ctx, er, nv, values, sym);
        for (c = 0; c < nch; ++c) {
            qe0[c] = tc_astc_color_symbol_uquant(er, sym[c * 2u]);
            qe1[c] = tc_astc_color_symbol_uquant(er, sym[c * 2u + 1u]);
            dir[c] = (int)qe1[c] - (int)qe0[c];
            dl += (int64_t)dir[c] * dir[c];
        }
        if (cem != 12u) {
            qe0[3] = 255u;
            qe1[3] = 255u;
        }
        for (i = 0; i < count; ++i) {
            int w64 = 0;
            if (dl > 0) {
                int64_t dot = 0;
                for (c = 0; c < nch; ++c)
                    dot += ((int64_t)block[i][c] - (int)qe0[c]) * dir[c];
                if (dot < 0) dot = 0;
                if (dot > dl) dot = dl;
                w64 = (int)((dot * 64 + dl / 2) / dl);
            }
            wsym[i] =
                tc_astc_hdr_quant_weight(w64, qm, tc_astc_quant_levels(qm));
            wt[i] = (uint8_t)tc_astc_weight_unquant(wsym[i], qm);
        }
        sse = tc_astc_recon_sse(block, count, qe0, qe1, wt);
        if (sse < best) {
            best = sse;
            memcpy(best_sym, sym, nv);
            memcpy(best_wsym, wsym, 16u);
        } else {
            break;
        }
        if (round + 1u < 3u &&
            !tc_astc_lsq_endpoints(block, count, wt, rlo, rhi))
            break;
    }
    if (best == (uint64_t)-1) return 0;
    *out_sse = best;
    memcpy(sym, best_sym, nv);
    memcpy(wsym, best_wsym, 16u);

    /* ascending-sum ordering to avoid the decoder's blue-contract path. */
    if (tc_astc_color_symbol_uquant(er, sym[0]) +
            tc_astc_color_symbol_uquant(er, sym[2]) +
            tc_astc_color_symbol_uquant(er, sym[4]) >
        tc_astc_color_symbol_uquant(er, sym[1]) +
            tc_astc_color_symbol_uquant(er, sym[3]) +
            tc_astc_color_symbol_uquant(er, sym[5])) {
        uint32_t pc = (cem == 12u) ? 4u : 3u, k;
        for (k = 0; k < pc; ++k) {
            uint8_t t = sym[k * 2u];
            sym[k * 2u] = sym[k * 2u + 1u];
            sym[k * 2u + 1u] = t;
        }
        tc_astc_invert_weights(wsym, 16u, qm);
    }

    memset(out, 0, 16u);
    memset(weightbuf, 0, sizeof(weightbuf));
    tc_astc_scramble_weights(wsym, 16u, qm);
    (void)tc_astc_ise_encode_bits(qm, 16u, wsym, weightbuf, sizeof(weightbuf),
                                  0u);
    for (i = 0; i < 16u; ++i) out[i] = tc_bitrev8(weightbuf[15u - i]);
    bitpos = 0;
    tc_set_bits(out, &bitpos, bm, 11u);
    tc_set_bits(out, &bitpos, 0u, 2u);
    tc_set_bits(out, &bitpos, cem, 4u);
    (void)tc_astc_ise_encode_bits(er, nv, sym, out, 16u, 17u);
    return 1;
}

/* UASTC CEM 4 (luminance+alpha) modes 15/16/17: RGB reconstruct from a single
 * luminance channel, plus alpha. Values are (L0,L1,A0,A1) per subset with no
 * blue-contract, so no endpoint swap is needed. `nsub` selects mode 15 (1) or
 * 16 (2); `dual` selects mode 17 (single subset, alpha on plane 2). */
static uint32_t tc_uastc_lum(const uint8_t px[4]) {
    return ((uint32_t)px[0] + px[1] + px[2]) / 3u;
}
static int tc_uastc_encode_cem4(const uint8_t block[144][4], uint32_t count,
                                const tc_astc_encode_context *ctx, uint32_t nsub,
                                int dual, uint32_t qm, uint8_t out[16],
                                uint64_t *out_sse) {
    const tc_astc_partition_info *pi = NULL;
    uint8_t weightbuf[16], sym[12], wsym[16], vals[8], dualw[32];
    uint8_t best_p1[16], best_p2[16], best_sym[12], best_wsym[16], wt[144];
    uint8_t blk_lum[144][4];
    uint32_t qe0[3][4], qe1[3][4], er, weight_bits, color_bits, bm, bitpos;
    uint32_t nvw = dual ? 32u : 16u, wcount = dual ? 32u : 16u;
    uint32_t rlo[3][4], rhi[3][4];
    uint32_t best_ccs = 99u, i, c, s, round, rounds;
    int dir[3][4];
    int64_t dl[3] = {0, 0, 0};
    uint64_t best_sse = (uint64_t)-1, best = (uint64_t)-1, sse = 0;

    bm = dual ? tc_astc_find_dual_grid_mode(qm) : tc_astc_hdr_find_grid_mode(qm);
    if (!bm) return 0;
    weight_bits = tc_astc_ise_sequence_bitcount(nvw, qm);
    if (dual) {
        if (weight_bits + 2u >= 109u) return 0;
        color_bits = 109u - weight_bits;
    } else {
        uint32_t cfg = (nsub > 1u) ? 99u : 111u;
        if (weight_bits >= cfg) return 0;
        color_bits = cfg - weight_bits;
    }
    if (!tc_astc_lut_color_quant(ctx, nsub * 4u, color_bits, &er)) return 0;
    if (nsub > 1u &&
        !tc_astc_find_best_partition(block, ctx->part2_cache, ctx->part2_count,
                                     nsub, ctx, &pi))
        return 0;
    /* Luminance block (RGB collapsed to one channel) is what CEM 4 can
     * represent, and the target for the least-squares endpoint solve. */
    for (i = 0; i < count; ++i) {
        uint8_t L = (uint8_t)tc_uastc_lum(block[i]);
        blk_lum[i][0] = blk_lum[i][1] = blk_lum[i][2] = L;
        blk_lum[i][3] = block[i][3];
    }
    for (s = 0; s < nsub; ++s) {
        uint32_t Llo = 255u, Lhi = 0, Alo = 255u, Ahi = 0;
        int have = 0;
        for (i = 0; i < count; ++i) {
            uint32_t L;
            if (pi && pi->partition_of_texel[i] != s) continue;
            L = tc_uastc_lum(block[i]);
            if (L < Llo) Llo = L;
            if (L > Lhi) Lhi = L;
            if (block[i][3] < Alo) Alo = block[i][3];
            if (block[i][3] > Ahi) Ahi = block[i][3];
            have = 1;
        }
        if (!have) return 0;
        rlo[s][0] = rlo[s][1] = rlo[s][2] = Llo;
        rlo[s][3] = Alo;
        rhi[s][0] = rhi[s][1] = rhi[s][2] = Lhi;
        rhi[s][3] = Ahi;
    }
    /* Dual plane keeps the bbox endpoints (a per-plane solve is not worth the
     * complexity for this niche mode); single/2-subset refine by least squares
     * against the luminance block over a few rounds. */
    rounds = dual ? 1u : 3u;
    for (round = 0; round < rounds; ++round) {
        for (s = 0; s < nsub; ++s) {
            vals[0] = (uint8_t)rlo[s][0];
            vals[1] = (uint8_t)rhi[s][0];
            vals[2] = (uint8_t)rlo[s][3];
            vals[3] = (uint8_t)rhi[s][3];
            tc_astc_quantize_color_values(ctx, er, 4u, vals, sym + s * 4u);
            qe0[s][0] = qe0[s][1] = qe0[s][2] =
                tc_astc_color_symbol_uquant(er, sym[s * 4u]);
            qe1[s][0] = qe1[s][1] = qe1[s][2] =
                tc_astc_color_symbol_uquant(er, sym[s * 4u + 1u]);
            qe0[s][3] = tc_astc_color_symbol_uquant(er, sym[s * 4u + 2u]);
            qe1[s][3] = tc_astc_color_symbol_uquant(er, sym[s * 4u + 3u]);
            dl[s] = 0;
            for (c = 0; c < 4u; ++c) {
                dir[s][c] = (int)qe1[s][c] - (int)qe0[s][c];
                dl[s] += (int64_t)dir[s][c] * dir[s][c];
            }
        }
        if (dual) break; /* dual scoring/assembly handled below */
        sse = 0;
        for (i = 0; i < count; ++i) {
            uint32_t s2 = pi ? pi->partition_of_texel[i] : 0u;
            int w64 = 0, uq;
            if (dl[s2] > 0) {
                int64_t dot = 0;
                for (c = 0; c < 4u; ++c)
                    dot += ((int64_t)block[i][c] - (int)qe0[s2][c]) * dir[s2][c];
                if (dot < 0) dot = 0;
                if (dot > dl[s2]) dot = dl[s2];
                w64 = (int)((dot * 64 + dl[s2] / 2) / dl[s2]);
            }
            wsym[i] =
                tc_astc_hdr_quant_weight(w64, qm, tc_astc_quant_levels(qm));
            uq = (int)tc_astc_weight_unquant(wsym[i], qm);
            wt[i] = (uint8_t)uq;
            for (c = 0; c < 4u; ++c) {
                int rec =
                    ((int)qe0[s2][c] * (64 - uq) + (int)qe1[s2][c] * uq + 32) >> 6;
                int e = (int)block[i][c] - rec;
                sse += (uint64_t)(e * e);
            }
        }
        if (sse < best) {
            best = sse;
            memcpy(best_sym, sym, nsub * 4u);
            memcpy(best_wsym, wsym, 16u);
        } else {
            break;
        }
        if (round + 1u < rounds) {
            for (s = 0; s < nsub; ++s) {
                if (pi)
                    (void)tc_astc_lsq_endpoints_partition(blk_lum, count, pi, s,
                                                          wt, rlo[s], rhi[s]);
                else
                    (void)tc_astc_lsq_endpoints(blk_lum, count, wt, rlo[s],
                                                rhi[s]);
            }
        }
    }

    if (dual) {
        uint32_t ccs;
        for (ccs = 0; ccs < 4u; ++ccs) {
            int64_t d1 = 0, d2 = (int64_t)dir[0][ccs] * dir[0][ccs];
            uint8_t t1[16], t2[16];
            uint64_t dsse = 0;
            for (c = 0; c < 4u; ++c)
                if (c != ccs) d1 += (int64_t)dir[0][c] * dir[0][c];
            for (i = 0; i < count; ++i) {
                int w1 = 0, w2 = 0, u1, u2;
                if (d1 > 0) {
                    int64_t dot = 0;
                    for (c = 0; c < 4u; ++c)
                        if (c != ccs)
                            dot += ((int64_t)block[i][c] - (int)qe0[0][c]) *
                                   dir[0][c];
                    if (dot < 0) dot = 0;
                    if (dot > d1) dot = d1;
                    w1 = (int)((dot * 64 + d1 / 2) / d1);
                }
                if (d2 > 0) {
                    int64_t dot =
                        ((int64_t)block[i][ccs] - (int)qe0[0][ccs]) * dir[0][ccs];
                    if (dot < 0) dot = 0;
                    if (dot > d2) dot = d2;
                    w2 = (int)((dot * 64 + d2 / 2) / d2);
                }
                t1[i] = tc_astc_hdr_quant_weight(w1, qm, tc_astc_quant_levels(qm));
                t2[i] = tc_astc_hdr_quant_weight(w2, qm, tc_astc_quant_levels(qm));
                u1 = (int)tc_astc_weight_unquant(t1[i], qm);
                u2 = (int)tc_astc_weight_unquant(t2[i], qm);
                for (c = 0; c < 4u; ++c) {
                    int uq = (c == ccs) ? u2 : u1;
                    int rec =
                        ((int)qe0[0][c] * (64 - uq) + (int)qe1[0][c] * uq + 32) >>
                        6;
                    int e = (int)block[i][c] - rec;
                    dsse += (uint64_t)(e * e);
                }
            }
            if (best_ccs == 99u || dsse < best_sse) {
                best_sse = dsse;
                best_ccs = ccs;
                memcpy(best_p1, t1, 16u);
                memcpy(best_p2, t2, 16u);
            }
        }
        sse = best_sse;
    } else {
        if (best == (uint64_t)-1) return 0;
        sse = best;
        memcpy(sym, best_sym, nsub * 4u);
        memcpy(wsym, best_wsym, 16u);
    }
    *out_sse = sse;

    memset(out, 0, 16u);
    memset(weightbuf, 0, sizeof(weightbuf));
    if (dual) {
        tc_astc_scramble_weights(best_p1, 16u, qm);
        tc_astc_scramble_weights(best_p2, 16u, qm);
        for (i = 0; i < 16u; ++i) {
            dualw[i * 2u] = best_p1[i];
            dualw[i * 2u + 1u] = best_p2[i];
        }
        (void)tc_astc_ise_encode_bits(qm, 32u, dualw, weightbuf,
                                      sizeof(weightbuf), 0u);
    } else {
        tc_astc_scramble_weights(wsym, 16u, qm);
        (void)tc_astc_ise_encode_bits(qm, 16u, wsym, weightbuf,
                                      sizeof(weightbuf), 0u);
    }
    for (i = 0; i < 16u; ++i) out[i] = tc_bitrev8(weightbuf[15u - i]);
    (void)wcount;
    bitpos = 0;
    tc_set_bits(out, &bitpos, bm, 11u);
    if (nsub > 1u) {
        tc_set_bits(out, &bitpos, nsub - 1u, 2u);
        tc_set_bits(out, &bitpos, (uint32_t)pi->partition_index & 63u, 6u);
        tc_set_bits(out, &bitpos, (uint32_t)pi->partition_index >> 6, 4u);
        bitpos = 23u;
        tc_set_bits(out, &bitpos, 4u << 2, 6u); /* multi-CEM: all-same CEM 4 */
        (void)tc_astc_ise_encode_bits(er, nsub * 4u, sym, out, 16u, 29u);
    } else {
        tc_set_bits(out, &bitpos, 0u, 2u);
        tc_set_bits(out, &bitpos, 4u, 4u); /* CEM 4 */
        if (dual) {
            bitpos = 128u - weight_bits - 2u;
            tc_set_bits(out, &bitpos, best_ccs, 2u);
        }
        (void)tc_astc_ise_encode_bits(er, 4u, sym, out, 16u, 17u);
    }
    return 1;
}

/* UASTC multi-subset modes (nsub 2: CEM 8 modes 2/4/7, CEM 12 mode 9; nsub 3:
 * CEM 8 mode 3). Partition-select on the real 8-bit block, per-subset
 * bounding-box endpoints, one weight per texel. Modes where a quantized subset
 * would trip the decoder's blue-contract are skipped (bbox min/max keep
 * endpoints in ascending-sum order almost always). */
static int tc_uastc_encode_msubset(const uint8_t block[144][4], uint32_t count,
                                   const tc_astc_encode_context *ctx,
                                   uint32_t cem, uint32_t nsub, uint32_t qm,
                                   uint8_t out[16], uint64_t *out_sse) {
    const tc_astc_partition_info *pi = NULL;
    const tc_astc_partition_info *cache =
        (nsub == 3u) ? ctx->part3_cache : ctx->part2_cache;
    uint32_t cache_count = (nsub == 3u) ? ctx->part3_count : ctx->part2_count;
    uint8_t weightbuf[16], sym[24], wsym[16], vals[8];
    uint8_t best_sym[24], best_wsym[16], wt[144];
    uint32_t qe0[3][4], qe1[3][4], er, weight_bits, color_bits, bm, bitpos;
    uint32_t nvs = (cem == 12u) ? 8u : 6u, nch = (cem == 12u) ? 4u : 3u;
    uint32_t rlo[3][4], rhi[3][4];
    uint64_t best = (uint64_t)-1;
    uint32_t i, c, s, round;

    bm = tc_astc_hdr_find_grid_mode(qm);
    if (!bm) return 0;
    weight_bits = tc_astc_ise_sequence_bitcount(16u, qm);
    if (weight_bits >= 99u) return 0;
    color_bits = 99u - weight_bits;
    if (!tc_astc_lut_color_quant(ctx, nsub * nvs, color_bits, &er)) return 0;
    if (!tc_astc_find_best_partition(block, cache, cache_count, nsub, ctx, &pi))
        return 0;
    for (s = 0; s < nsub; ++s) {
        uint32_t lo[4] = {255u, 255u, 255u, 255u}, hi[4] = {0, 0, 0, 0};
        int have = 0;
        for (i = 0; i < count; ++i) {
            if (pi->partition_of_texel[i] != s) continue;
            for (c = 0; c < 4u; ++c) {
                if (block[i][c] < lo[c]) lo[c] = block[i][c];
                if (block[i][c] > hi[c]) hi[c] = block[i][c];
            }
            have = 1;
        }
        if (!have) return 0;
        for (c = 0; c < 4u; ++c) {
            rlo[s][c] = lo[c];
            rhi[s][c] = hi[c];
        }
    }
    /* Round 0 = bbox endpoints; later rounds re-solve each subset's endpoints
     * by least squares against the shared weights and keep the best. */
    for (round = 0; round < 3u; ++round) {
        int dir[3][4], valid = 1;
        int64_t dl[3] = {0, 0, 0};
        uint64_t sse = 0;
        for (s = 0; s < nsub; ++s) {
            vals[0] = (uint8_t)rlo[s][0];
            vals[1] = (uint8_t)rhi[s][0];
            vals[2] = (uint8_t)rlo[s][1];
            vals[3] = (uint8_t)rhi[s][1];
            vals[4] = (uint8_t)rlo[s][2];
            vals[5] = (uint8_t)rhi[s][2];
            if (cem == 12u) {
                vals[6] = (uint8_t)rlo[s][3];
                vals[7] = (uint8_t)rhi[s][3];
            }
            tc_astc_quantize_color_values(ctx, er, nvs, vals, sym + s * nvs);
            for (c = 0; c < nch; ++c) {
                qe0[s][c] =
                    tc_astc_color_symbol_uquant(er, sym[s * nvs + c * 2u]);
                qe1[s][c] =
                    tc_astc_color_symbol_uquant(er, sym[s * nvs + c * 2u + 1u]);
                dir[s][c] = (int)qe1[s][c] - (int)qe0[s][c];
                dl[s] += (int64_t)dir[s][c] * dir[s][c];
            }
            if (cem != 12u) {
                qe0[s][3] = 255u;
                qe1[s][3] = 255u;
            }
            if (qe0[s][0] + qe0[s][1] + qe0[s][2] >
                qe1[s][0] + qe1[s][1] + qe1[s][2]) {
                valid = 0;
                break;
            }
        }
        if (!valid) break;
        for (i = 0; i < count; ++i) {
            uint32_t s2 = pi->partition_of_texel[i];
            int w64 = 0, uq;
            if (dl[s2] > 0) {
                int64_t dot = 0;
                for (c = 0; c < nch; ++c)
                    dot += ((int64_t)block[i][c] - (int)qe0[s2][c]) * dir[s2][c];
                if (dot < 0) dot = 0;
                if (dot > dl[s2]) dot = dl[s2];
                w64 = (int)((dot * 64 + dl[s2] / 2) / dl[s2]);
            }
            wsym[i] =
                tc_astc_hdr_quant_weight(w64, qm, tc_astc_quant_levels(qm));
            uq = (int)tc_astc_weight_unquant(wsym[i], qm);
            wt[i] = (uint8_t)uq;
            for (c = 0; c < nch; ++c) {
                int rec =
                    ((int)qe0[s2][c] * (64 - uq) + (int)qe1[s2][c] * uq + 32) >>
                    6;
                int e = (int)block[i][c] - rec;
                sse += (uint64_t)(e * e);
            }
        }
        if (sse < best) {
            best = sse;
            memcpy(best_sym, sym, nsub * nvs);
            memcpy(best_wsym, wsym, 16u);
        } else {
            break;
        }
        if (round + 1u < 3u)
            for (s = 0; s < nsub; ++s)
                (void)tc_astc_lsq_endpoints_partition(block, count, pi, s, wt,
                                                      rlo[s], rhi[s]);
    }
    if (best == (uint64_t)-1) return 0;
    *out_sse = best;
    memcpy(sym, best_sym, nsub * nvs);
    memcpy(wsym, best_wsym, 16u);

    memset(out, 0, 16u);
    memset(weightbuf, 0, sizeof(weightbuf));
    tc_astc_scramble_weights(wsym, 16u, qm);
    (void)tc_astc_ise_encode_bits(qm, 16u, wsym, weightbuf, sizeof(weightbuf),
                                  0u);
    for (i = 0; i < 16u; ++i) out[i] = tc_bitrev8(weightbuf[15u - i]);
    bitpos = 0;
    tc_set_bits(out, &bitpos, bm, 11u);
    tc_set_bits(out, &bitpos, nsub - 1u, 2u);
    tc_set_bits(out, &bitpos, (uint32_t)pi->partition_index & 63u, 6u);
    tc_set_bits(out, &bitpos, (uint32_t)pi->partition_index >> 6, 4u);
    bitpos = 23u;
    tc_set_bits(out, &bitpos, cem << 2, 6u);
    (void)tc_astc_ise_encode_bits(er, nsub * nvs, sym, out, 16u, 29u);
    return 1;
}

/* UASTC dual-plane modes (CEM 8: 6, CEM 12: 11/13). Single subset, one channel
 * (the CCS) carried on a second weight plane; picks the best CCS by SSE. */
static int tc_uastc_encode_dual(const uint8_t block[144][4], uint32_t count,
                                const tc_astc_encode_context *ctx, uint32_t cem,
                                uint32_t qm, uint8_t out[16], uint64_t *out_sse) {
    uint8_t weightbuf[16], sym[8], vals[8], dualw[32];
    uint8_t best_p1[16], best_p2[16], best_sym[8];
    uint8_t best_wt1[144], best_wt2[144];
    uint32_t qe0[4], qe1[4], er, weight_bits, color_bits, bm, bitpos, ccs;
    uint32_t nv = (cem == 12u) ? 8u : 6u, nch = (cem == 12u) ? 4u : 3u;
    uint32_t nccs = nch, best_ccs = 99u;
    uint32_t rlo[4], rhi[4];
    uint32_t i, c, round;
    uint64_t best = (uint64_t)-1;

    bm = tc_astc_find_dual_grid_mode(qm);
    if (!bm) return 0;
    weight_bits = tc_astc_ise_sequence_bitcount(32u, qm);
    if (weight_bits + 2u >= 109u) return 0;
    color_bits = 109u - weight_bits; /* 128 - 17 (config) - 2 (ccs) */
    if (!tc_astc_lut_color_quant(ctx, nv, color_bits, &er)) return 0;
    {
        uint32_t lo[4] = {255u, 255u, 255u, 255u}, hi[4] = {0, 0, 0, 0};
        for (i = 0; i < count; ++i)
            for (c = 0; c < 4u; ++c) {
                if (block[i][c] < lo[c]) lo[c] = block[i][c];
                if (block[i][c] > hi[c]) hi[c] = block[i][c];
            }
        for (c = 0; c < 4u; ++c) {
            rlo[c] = lo[c];
            rhi[c] = hi[c];
        }
    }
    /* Round 0 = bbox; later rounds re-solve the (shared) endpoints by least
     * squares -- the RGB channels from the colour plane's weights and the CCS
     * channel from the alpha plane's weights -- then re-quantize. */
    for (round = 0; round < 3u; ++round) {
        uint32_t rbest_ccs = 99u;
        uint64_t rbest = (uint64_t)-1;
        uint8_t rp1[16] = {0}, rp2[16] = {0}, rwt1[144] = {0}, rwt2[144] = {0};
        vals[0] = (uint8_t)rlo[0];
        vals[1] = (uint8_t)rhi[0];
        vals[2] = (uint8_t)rlo[1];
        vals[3] = (uint8_t)rhi[1];
        vals[4] = (uint8_t)rlo[2];
        vals[5] = (uint8_t)rhi[2];
        if (cem == 12u) {
            vals[6] = (uint8_t)rlo[3];
            vals[7] = (uint8_t)rhi[3];
        }
        tc_astc_quantize_color_values(ctx, er, nv, vals, sym);
        for (c = 0; c < nch; ++c) {
            qe0[c] = tc_astc_color_symbol_uquant(er, sym[c * 2u]);
            qe1[c] = tc_astc_color_symbol_uquant(er, sym[c * 2u + 1u]);
        }
        if (cem != 12u) {
            qe0[3] = 255u;
            qe1[3] = 255u;
        }
        if (qe0[0] + qe0[1] + qe0[2] > qe1[0] + qe1[1] + qe1[2]) break;
        for (ccs = 0; ccs < nccs; ++ccs) {
            int dir[4];
            int64_t d1 = 0, d2;
            uint8_t t1[16], t2[16], wt1[144], wt2[144];
            uint64_t dsse = 0;
            for (c = 0; c < nch; ++c) {
                dir[c] = (int)qe1[c] - (int)qe0[c];
                if (c != ccs) d1 += (int64_t)dir[c] * dir[c];
            }
            d2 = (int64_t)dir[ccs] * dir[ccs];
            for (i = 0; i < count; ++i) {
                int w1 = 0, w2 = 0, u1, u2;
                if (d1 > 0) {
                    int64_t dot = 0;
                    for (c = 0; c < nch; ++c)
                        if (c != ccs)
                            dot += ((int64_t)block[i][c] - (int)qe0[c]) * dir[c];
                    if (dot < 0) dot = 0;
                    if (dot > d1) dot = d1;
                    w1 = (int)((dot * 64 + d1 / 2) / d1);
                }
                if (d2 > 0) {
                    int64_t dot =
                        ((int64_t)block[i][ccs] - (int)qe0[ccs]) * dir[ccs];
                    if (dot < 0) dot = 0;
                    if (dot > d2) dot = d2;
                    w2 = (int)((dot * 64 + d2 / 2) / d2);
                }
                t1[i] = tc_astc_hdr_quant_weight(w1, qm, tc_astc_quant_levels(qm));
                t2[i] = tc_astc_hdr_quant_weight(w2, qm, tc_astc_quant_levels(qm));
                u1 = (int)tc_astc_weight_unquant(t1[i], qm);
                u2 = (int)tc_astc_weight_unquant(t2[i], qm);
                wt1[i] = (uint8_t)u1;
                wt2[i] = (uint8_t)u2;
                for (c = 0; c < nch; ++c) {
                    int uq = (c == ccs) ? u2 : u1;
                    int rec =
                        ((int)qe0[c] * (64 - uq) + (int)qe1[c] * uq + 32) >> 6;
                    int e = (int)block[i][c] - rec;
                    dsse += (uint64_t)(e * e);
                }
            }
            if (rbest_ccs == 99u || dsse < rbest) {
                rbest = dsse;
                rbest_ccs = ccs;
                memcpy(rp1, t1, 16u);
                memcpy(rp2, t2, 16u);
                memcpy(rwt1, wt1, count);
                memcpy(rwt2, wt2, count);
            }
        }
        if (rbest < best) {
            best = rbest;
            best_ccs = rbest_ccs;
            memcpy(best_sym, sym, nv);
            memcpy(best_p1, rp1, 16u);
            memcpy(best_p2, rp2, 16u);
            memcpy(best_wt1, rwt1, count);
            memcpy(best_wt2, rwt2, count);
        } else {
            break;
        }
        if (round + 1u < 3u) {
            uint32_t lo1[4], hi1[4], lo2[4], hi2[4];
            if (!tc_astc_lsq_endpoints(block, count, best_wt1, lo1, hi1) ||
                !tc_astc_lsq_endpoints(block, count, best_wt2, lo2, hi2))
                break;
            for (c = 0; c < 4u; ++c) {
                if (c == best_ccs) {
                    rlo[c] = lo2[c];
                    rhi[c] = hi2[c];
                } else {
                    rlo[c] = lo1[c];
                    rhi[c] = hi1[c];
                }
            }
        }
    }
    if (best == (uint64_t)-1) return 0;
    *out_sse = best;
    memcpy(sym, best_sym, nv);

    memset(out, 0, 16u);
    memset(weightbuf, 0, sizeof(weightbuf));
    tc_astc_scramble_weights(best_p1, 16u, qm);
    tc_astc_scramble_weights(best_p2, 16u, qm);
    for (i = 0; i < 16u; ++i) {
        dualw[i * 2u] = best_p1[i];
        dualw[i * 2u + 1u] = best_p2[i];
    }
    (void)tc_astc_ise_encode_bits(qm, 32u, dualw, weightbuf, sizeof(weightbuf),
                                  0u);
    for (i = 0; i < 16u; ++i) out[i] = tc_bitrev8(weightbuf[15u - i]);
    bitpos = 0;
    tc_set_bits(out, &bitpos, bm, 11u);
    tc_set_bits(out, &bitpos, 0u, 2u);
    tc_set_bits(out, &bitpos, cem, 4u);
    bitpos = 128u - weight_bits - 2u;
    tc_set_bits(out, &bitpos, best_ccs, 2u);
    (void)tc_astc_ise_encode_bits(er, nv, sym, out, 16u, 17u);
    return 1;
}

static void tc_encode_uastc_ldr_block(const uint8_t block[144][4],
                                      uint32_t count,
                                      const tc_astc_encode_context *ctx,
                                      uint8_t out[16]) {
    static const uint32_t wr_opaque[4] = {8u, 5u, 2u, 11u};
    static const uint32_t wr_alpha[3] = {8u, 5u, 2u};
    uint32_t lo[4] = {255u, 255u, 255u, 255u}, hi[4] = {0, 0, 0, 0};
    uint32_t i, c, n, opaque, cem, nv, best_found = 0;
    const uint32_t *wr;
    uint64_t best_sse = (uint64_t)-1;
    uint8_t tmp[16];

    if (tc_astc_block_is_solid(block, count)) {
        uint32_t sum[4] = {0, 0, 0, 0};
        for (i = 0; i < count; ++i)
            for (c = 0; c < 4u; ++c) sum[c] += block[i][c];
        tc_astc_write_const_from_sum(sum, count, out);
        return;
    }
    opaque = (uint32_t)tc_astc_block_is_opaque(block, count);
    cem = opaque ? 8u : 12u;
    nv = opaque ? 6u : 8u;
    wr = opaque ? wr_opaque : wr_alpha;
    n = opaque ? 4u : 3u;
    for (i = 0; i < count; ++i)
        for (c = 0; c < 4u; ++c) {
            if (block[i][c] < lo[c]) lo[c] = block[i][c];
            if (block[i][c] > hi[c]) hi[c] = block[i][c];
        }
    for (i = 0; i < n; ++i) {
        uint64_t sse;
        if (tc_uastc_encode_mode(block, count, ctx, cem, nv, wr[i], lo, hi, tmp,
                                 &sse) &&
            (!best_found || sse < best_sse)) {
            best_sse = sse;
            best_found = 1;
            memcpy(out, tmp, 16u);
        }
    }
    /* 2-subset modes (CEM 8: weight range 5/2; CEM 12: weight range 2). */
    {
        static const uint32_t sub2_opaque[2] = {5u, 2u};
        static const uint32_t sub2_alpha[1] = {2u};
        const uint32_t *sw = opaque ? sub2_opaque : sub2_alpha;
        uint32_t sn = opaque ? 2u : 1u;
        for (i = 0; i < sn; ++i) {
            uint64_t sse;
            if (tc_uastc_encode_msubset(block, count, ctx, cem, 2u, sw[i], tmp,
                                        &sse) &&
                (!best_found || sse < best_sse)) {
                best_sse = sse;
                best_found = 1;
                memcpy(out, tmp, 16u);
            }
        }
    }
    /* 3-subset mode (CEM 8: weight range 2, mode 3). */
    if (opaque) {
        uint64_t sse;
        if (tc_uastc_encode_msubset(block, count, ctx, cem, 3u, 2u, tmp, &sse) &&
            (!best_found || sse < best_sse)) {
            best_sse = sse;
            best_found = 1;
            memcpy(out, tmp, 16u);
        }
    }
    /* dual-plane modes (CEM 8: weight range 2; CEM 12: weight range 2/0). */
    {
        static const uint32_t dual_opaque[1] = {2u};
        static const uint32_t dual_alpha[2] = {2u, 0u};
        const uint32_t *dw = opaque ? dual_opaque : dual_alpha;
        uint32_t dn = opaque ? 1u : 2u;
        for (i = 0; i < dn; ++i) {
            uint64_t sse;
            if (tc_uastc_encode_dual(block, count, ctx, cem, dw[i], tmp, &sse) &&
                (!best_found || sse < best_sse)) {
                best_sse = sse;
                best_found = 1;
                memcpy(out, tmp, 16u);
            }
        }
    }
    /* CEM 4 (luminance+alpha) modes 15/16/17, for non-opaque blocks: single
     * subset (wr 8), 2-subset (wr 2) and dual-plane (wr 2). SSE decides against
     * the CEM 12 modes, so these only win on grayscale-with-alpha content. */
    if (!opaque) {
        uint64_t sse;
        if (tc_uastc_encode_cem4(block, count, ctx, 1u, 0, 8u, tmp, &sse) &&
            (!best_found || sse < best_sse)) {
            best_sse = sse;
            best_found = 1;
            memcpy(out, tmp, 16u);
        }
        if (tc_uastc_encode_cem4(block, count, ctx, 2u, 0, 2u, tmp, &sse) &&
            (!best_found || sse < best_sse)) {
            best_sse = sse;
            best_found = 1;
            memcpy(out, tmp, 16u);
        }
        if (tc_uastc_encode_cem4(block, count, ctx, 1u, 1, 2u, tmp, &sse) &&
            (!best_found || sse < best_sse)) {
            best_sse = sse;
            best_found = 1;
            memcpy(out, tmp, 16u);
        }
    }
    if (!best_found) {
        uint32_t sum[4] = {0, 0, 0, 0};
        for (i = 0; i < count; ++i)
            for (c = 0; c < 4u; ++c) sum[c] += block[i][c];
        tc_astc_write_const_from_sum(sum, count, out);
    }
}

#if defined(TC_X86)
TC_TARGET("sse2")
static void tc_encode_astc_const_block_sse2(const uint8_t block[16][4],
                                            uint8_t out[16]) {
    uint32_t sum[4];
    uint64_t lanes[2];
    uint32_t c;
    const __m128i masks[4] = {
        _mm_set1_epi32(0x000000ff),
        _mm_set1_epi32(0x0000ff00),
        _mm_set1_epi32(0x00ff0000),
        _mm_set1_epi32((int)0xff000000u)};
    const __m128i z = _mm_setzero_si128();
    __m128i v0 = _mm_loadu_si128((const __m128i *)(const void *)(block + 0));
    __m128i v1 = _mm_loadu_si128((const __m128i *)(const void *)(block + 4));
    __m128i v2 = _mm_loadu_si128((const __m128i *)(const void *)(block + 8));
    __m128i v3 = _mm_loadu_si128((const __m128i *)(const void *)(block + 12));
    for (c = 0; c < 4u; ++c) {
        __m128i s = _mm_sad_epu8(_mm_and_si128(v0, masks[c]), z);
        s = _mm_add_epi64(s, _mm_sad_epu8(_mm_and_si128(v1, masks[c]), z));
        s = _mm_add_epi64(s, _mm_sad_epu8(_mm_and_si128(v2, masks[c]), z));
        s = _mm_add_epi64(s, _mm_sad_epu8(_mm_and_si128(v3, masks[c]), z));
        _mm_storeu_si128((__m128i *)(void *)lanes, s);
        sum[c] = (uint32_t)(lanes[0] + lanes[1]);
    }
    tc_astc_write_const_from_sum(sum, 16u, out);
}

TC_TARGET("sse4.1")
static void tc_encode_astc_const_block_sse41(const uint8_t block[16][4],
                                             uint8_t out[16]) {
    uint32_t sum[4];
    uint64_t lanes[2];
    uint32_t c;
    const __m128i mask[4] = {
        _mm_setr_epi8(0, 4, 8, 12, -128, -128, -128, -128, -128, -128, -128,
                      -128, -128, -128, -128, -128),
        _mm_setr_epi8(1, 5, 9, 13, -128, -128, -128, -128, -128, -128, -128,
                      -128, -128, -128, -128, -128),
        _mm_setr_epi8(2, 6, 10, 14, -128, -128, -128, -128, -128, -128, -128,
                      -128, -128, -128, -128, -128),
        _mm_setr_epi8(3, 7, 11, 15, -128, -128, -128, -128, -128, -128, -128,
                      -128, -128, -128, -128, -128)};
    const __m128i z = _mm_setzero_si128();
    __m128i v0 = _mm_loadu_si128((const __m128i *)(const void *)(block + 0));
    __m128i v1 = _mm_loadu_si128((const __m128i *)(const void *)(block + 4));
    __m128i v2 = _mm_loadu_si128((const __m128i *)(const void *)(block + 8));
    __m128i v3 = _mm_loadu_si128((const __m128i *)(const void *)(block + 12));
    for (c = 0; c < 4u; ++c) {
        __m128i s = _mm_sad_epu8(_mm_shuffle_epi8(v0, mask[c]), z);
        s = _mm_add_epi64(s, _mm_sad_epu8(_mm_shuffle_epi8(v1, mask[c]), z));
        s = _mm_add_epi64(s, _mm_sad_epu8(_mm_shuffle_epi8(v2, mask[c]), z));
        s = _mm_add_epi64(s, _mm_sad_epu8(_mm_shuffle_epi8(v3, mask[c]), z));
        _mm_storeu_si128((__m128i *)(void *)lanes, s);
        sum[c] = (uint32_t)(lanes[0] + lanes[1]);
    }
    tc_astc_write_const_from_sum(sum, 16u, out);
}

TC_TARGET("avx2")
static void tc_encode_astc_const_block_avx2(const uint8_t block[16][4],
                                            uint8_t out[16]) {
    uint32_t sum[4];
    uint64_t lanes[4];
    uint32_t c;
    const __m256i mask[4] = {
        _mm256_setr_epi8(0, 4, 8, 12, -128, -128, -128, -128, -128, -128, -128,
                         -128, -128, -128, -128, -128, 0, 4, 8, 12, -128,
                         -128, -128, -128, -128, -128, -128, -128, -128, -128,
                         -128, -128),
        _mm256_setr_epi8(1, 5, 9, 13, -128, -128, -128, -128, -128, -128, -128,
                         -128, -128, -128, -128, -128, 1, 5, 9, 13, -128,
                         -128, -128, -128, -128, -128, -128, -128, -128, -128,
                         -128, -128),
        _mm256_setr_epi8(2, 6, 10, 14, -128, -128, -128, -128, -128, -128, -128,
                         -128, -128, -128, -128, -128, 2, 6, 10, 14, -128,
                         -128, -128, -128, -128, -128, -128, -128, -128, -128,
                         -128, -128),
        _mm256_setr_epi8(3, 7, 11, 15, -128, -128, -128, -128, -128, -128, -128,
                         -128, -128, -128, -128, -128, 3, 7, 11, 15, -128,
                         -128, -128, -128, -128, -128, -128, -128, -128, -128,
                         -128, -128)};
    const __m256i z = _mm256_setzero_si256();
    __m256i v0 = _mm256_loadu_si256((const __m256i *)(const void *)(block + 0));
    __m256i v1 = _mm256_loadu_si256((const __m256i *)(const void *)(block + 8));
    for (c = 0; c < 4u; ++c) {
        __m256i s = _mm256_sad_epu8(_mm256_shuffle_epi8(v0, mask[c]), z);
        s = _mm256_add_epi64(s, _mm256_sad_epu8(_mm256_shuffle_epi8(v1, mask[c]), z));
        _mm256_storeu_si256((__m256i *)(void *)lanes, s);
        sum[c] = (uint32_t)(lanes[0] + lanes[1] + lanes[2] + lanes[3]);
    }
    tc_astc_write_const_from_sum(sum, 16u, out);
}
#endif

#if defined(__ARM_NEON) || defined(__ARM_NEON__)
static uint32_t tc_neon_sum_u8(uint8x16_t v) {
    uint16x8_t s16 = vpaddlq_u8(v);
    uint32x4_t s32 = vpaddlq_u16(s16);
    uint64x2_t s64 = vpaddlq_u32(s32);
    return (uint32_t)(vgetq_lane_u64(s64, 0) + vgetq_lane_u64(s64, 1));
}

static void tc_encode_astc_const_block_neon(const uint8_t block[16][4],
                                            uint8_t out[16]) {
    uint32_t sum[4];
    uint8x16x4_t px = vld4q_u8((const uint8_t *)block);
    sum[0] = tc_neon_sum_u8(px.val[0]);
    sum[1] = tc_neon_sum_u8(px.val[1]);
    sum[2] = tc_neon_sum_u8(px.val[2]);
    sum[3] = tc_neon_sum_u8(px.val[3]);
    tc_astc_write_const_from_sum(sum, 16u, out);
}
#endif

static void tc_encode_astc_const_block(const uint8_t block[144][4],
                                       uint32_t count, uint8_t out[16]) {
    uint32_t i, c;
    uint32_t sum[4] = {0, 0, 0, 0};
#if defined(TC_X86)
    if (count == 16u) {
        uint32_t caps = tc_cpu_caps();
        if (caps & TC_CPU_AVX2) {
            tc_encode_astc_const_block_avx2((const uint8_t(*)[4])block, out);
            return;
        }
        if (caps & TC_CPU_SSE41) {
            tc_encode_astc_const_block_sse41((const uint8_t(*)[4])block, out);
            return;
        }
        if (caps & TC_CPU_SSE2) {
            tc_encode_astc_const_block_sse2((const uint8_t(*)[4])block, out);
            return;
        }
    }
#endif
#if defined(__ARM_NEON) || defined(__ARM_NEON__)
    if (count == 16u && (tc_cpu_caps() & TC_CPU_NEON)) {
        tc_encode_astc_const_block_neon((const uint8_t(*)[4])block, out);
        return;
    }
#endif
    for (i = 0; i < count; ++i) {
        for (c = 0; c < 4u; ++c) sum[c] += block[i][c];
    }
    tc_astc_write_const_from_sum(sum, count, out);
}

/* Encodes block rows [brow_begin, brow_end) with a private context; output
 * ranges of distinct bands are disjoint, so bands run in parallel without
 * synchronization and produce output byte-identical to the serial order. */
static tc_result tc_astc_compress_band(tc_astc_encode_context *astc_ctx,
                                       const uint8_t *rgba, uint32_t width,
                                       uint32_t height, size_t stride,
                                       const tc_astc_options *opt,
                                       uint8_t *out_astc, uint32_t brow_begin,
                                       uint32_t brow_end) {
    uint8_t block[144][4];
    uint32_t bx, by, xx, yy, x, y, brow;
    uint32_t blocks_x = (width + opt->block_x - 1u) / opt->block_x;
    size_t off = (size_t)brow_begin * blocks_x * 16u;

    for (brow = brow_begin; brow < brow_end; ++brow) {
        by = brow * opt->block_y;
        for (bx = 0; bx < width; bx += opt->block_x) {
            uint32_t count = 0;
            for (yy = 0; yy < opt->block_y; ++yy) {
                y = by + yy;
                if (y >= height) y = height - 1u;
                for (xx = 0; xx < opt->block_x; ++xx) {
                    const uint8_t *src;
                    x = bx + xx;
                    if (x >= width) x = width - 1u;
                    src = rgba + (size_t)y * stride + (size_t)x * 4u;
                    memcpy(block[count], src, 4u);
                    ++count;
                }
            }
            if (opt->uastc) {
                tc_encode_uastc_ldr_block(block, count, astc_ctx,
                                          out_astc + off);
            } else if (!tc_astc_block_is_solid(block, count)) {
                tc_encode_astc_ldr_block(block, count, opt->quality, astc_ctx,
                                         out_astc + off);
            } else {
                tc_encode_astc_const_block(block, count, out_astc + off);
            }
            off += 16u;
        }
    }
    return TC_SUCCESS;
}

#if !defined(__STDC_NO_THREADS__) && !defined(TC_NO_THREADS) && \
    !defined(_WIN32)
#include <threads.h>
#define TC_ASTC_HAVE_THREADS 1

typedef struct tc_astc_thread_job {
    tc_astc_encode_context *ctx; /* shared; strictly read-only here */
    const uint8_t *rgba;
    uint32_t width, height;
    size_t stride;
    const tc_astc_options *opt;
    uint8_t *out_astc;
    uint32_t brow_begin, brow_end;
    tc_result result;
} tc_astc_thread_job;

static int tc_astc_thread_worker(void *arg) {
    tc_astc_thread_job *job = (tc_astc_thread_job *)arg;
    job->result = tc_astc_compress_band(job->ctx, job->rgba, job->width,
                                        job->height, job->stride, job->opt,
                                        job->out_astc, job->brow_begin,
                                        job->brow_end);
    return 0;
}
#endif

tc_result tc_astc_compress_rgba8(const uint8_t *rgba, uint32_t width,
                                 uint32_t height, size_t stride,
                                 const tc_astc_options *opt,
                                 uint8_t *out_astc, size_t out_size) {
    tc_astc_options defopt;
    uint32_t blocks_y, nthreads;
    size_t need;

    if (!opt) {
        tc_astc_options_init(&defopt);
        opt = &defopt;
    }
    if (!rgba || !out_astc || !width || !height) return TC_ERROR_INVALID_ARGUMENT;
    if (!tc_astc_valid_block(opt->block_x, opt->block_y))
        return TC_ERROR_INVALID_ARGUMENT;
    if (stride < (size_t)width * 4u) return TC_ERROR_INVALID_ARGUMENT;
    need = tc_astc_compressed_size(width, height, opt);
    if (!need || out_size < need) return TC_ERROR_INVALID_ARGUMENT;

    blocks_y = (height + opt->block_y - 1u) / opt->block_y;
    nthreads = opt->threads > 0 ? (uint32_t)opt->threads : 1u;
    if (nthreads > blocks_y) nthreads = blocks_y;
    if (nthreads > 64u) nthreads = 64u;

    /* The encode context memoizes decimation tables lazily during encoding
     * (tc_astc_get_decim_cache), so it is NOT safe to share across threads --
     * concurrent cache fills race. Give every worker its own context. The
     * caches are pure memoization, so per-thread contexts produce output
     * byte-identical to the single-threaded path. */
#if defined(TC_ASTC_HAVE_THREADS)
    if (nthreads > 1u) {
        tc_astc_thread_job jobs[64];
        thrd_t tids[64];
        tc_astc_encode_context *ctxs[64];
        uint32_t t, k, spawned = 0;
        uint32_t base = blocks_y / nthreads, rem = blocks_y % nthreads;
        uint32_t row = 0;
        tc_result tr = TC_SUCCESS;
        for (t = 0; t < nthreads; ++t) {
            uint32_t rows = base + (t < rem ? 1u : 0u);
            ctxs[t] = (tc_astc_encode_context *)malloc(sizeof(*ctxs[t]));
            if (!ctxs[t]) {
                for (k = 0; k < t; ++k) free(ctxs[k]);
                return TC_ERROR_OUT_OF_MEMORY;
            }
            tc_astc_encode_context_init(ctxs[t], opt->block_x, opt->block_y,
                                        opt->quality);
            jobs[t].ctx = ctxs[t];
            jobs[t].rgba = rgba;
            jobs[t].width = width;
            jobs[t].height = height;
            jobs[t].stride = stride;
            jobs[t].opt = opt;
            jobs[t].out_astc = out_astc;
            jobs[t].brow_begin = row;
            jobs[t].brow_end = row + rows;
            jobs[t].result = TC_SUCCESS;
            row += rows;
        }
        for (t = 1; t < nthreads; ++t) {
            if (thrd_create(&tids[t], tc_astc_thread_worker, &jobs[t]) !=
                thrd_success)
                break;
            ++spawned;
        }
        tc_astc_thread_worker(&jobs[0]);
        /* Bands whose thread never launched run serially on this thread. */
        for (t = spawned + 1u; t < nthreads; ++t)
            tc_astc_thread_worker(&jobs[t]);
        for (t = 1; t <= spawned; ++t) {
            int rr;
            thrd_join(tids[t], &rr);
        }
        for (t = 0; t < nthreads; ++t) {
            if (jobs[t].result != TC_SUCCESS) tr = jobs[t].result;
            free(ctxs[t]);
        }
        return tr;
    }
#endif
    {
        tc_astc_encode_context *astc_ctx =
            (tc_astc_encode_context *)malloc(sizeof(*astc_ctx));
        tc_result tr_all;
        if (!astc_ctx) return TC_ERROR_OUT_OF_MEMORY;
        tc_astc_encode_context_init(astc_ctx, opt->block_x, opt->block_y,
                                    opt->quality);
        tr_all = tc_astc_compress_band(astc_ctx, rgba, width, height, stride,
                                       opt, out_astc, 0u, blocks_y);
        free(astc_ctx);
        return tr_all;
    }
}

size_t tc_astc_file_size(uint32_t width, uint32_t height,
                         const tc_astc_options *opt) {
    size_t payload = tc_astc_compressed_size(width, height, opt);
    if (!payload || payload > SIZE_MAX - 16u) return 0;
    return 16u + payload;
}

tc_result tc_astc_write_file_memory(const uint8_t *astc_blocks, uint32_t width,
                                    uint32_t height,
                                    const tc_astc_options *opt,
                                    uint8_t *out_astc_file, size_t out_size) {
    tc_astc_options defopt;
    size_t payload, need;
    if (!opt) {
        tc_astc_options_init(&defopt);
        opt = &defopt;
    }
    if (!astc_blocks || !out_astc_file || !width || !height)
        return TC_ERROR_INVALID_ARGUMENT;
    if (!tc_astc_valid_block(opt->block_x, opt->block_y))
        return TC_ERROR_INVALID_ARGUMENT;
    payload = tc_astc_compressed_size(width, height, opt);
    need = tc_astc_file_size(width, height, opt);
    if (!need || out_size < need) return TC_ERROR_INVALID_ARGUMENT;

    memset(out_astc_file, 0, 16u);
    out_astc_file[0] = 0x13u;
    out_astc_file[1] = 0xabu;
    out_astc_file[2] = 0xa1u;
    out_astc_file[3] = 0x5cu;
    out_astc_file[4] = (uint8_t)opt->block_x;
    out_astc_file[5] = (uint8_t)opt->block_y;
    out_astc_file[6] = 1u;
    tc_wr_u24(out_astc_file + 7, width);
    tc_wr_u24(out_astc_file + 10, height);
    tc_wr_u24(out_astc_file + 13, 1u);
    memcpy(out_astc_file + 16, astc_blocks, payload);
    return TC_SUCCESS;
}
