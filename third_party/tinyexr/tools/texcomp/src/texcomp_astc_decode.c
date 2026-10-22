/*
 * TinyEXR texcomp - ASTC block decoder (pure-C11, UASTC-capable).
 *
 * Independent decoder for the ASTC spec bit-level semantics: ISE decode,
 * bilinear weight infill (spec C.2.18), partition hash (C.2.21), and LDR
 * CEM 0/4/6/8/10/12 decode including the endpoint-swap + blue-contract rule
 * for CEM 8/12. Handles all ASTC 2D block modes (including void-extent),
 * not just the UASTC subset — the encoder emits only UASTC modes, but a
 * conformant decoder must accept all valid ASTC blocks.
 *
 * Final texel interpolation uses the 8-bit model
 * (e0*(64-w) + e1*w + 32) >> 6, matching the encoder's error model.
 *
 * Copyright (c) 2014-2026 Syoyo Fujita and TinyEXR authors
 * SPDX-License-Identifier: Apache-2.0
 *
 * Portions adapted from astc_ref_decode.h (test reference decoder, same
 * license), which in turn adapted portions from the bcdec project (public
 * domain, https://github.com/iOrange/bcdec).
 */
#include "texcomp_internal.h"
#include <string.h>

/* ---- ISE tables (ASTC spec data) --------------------------------------- */

static const uint8_t tacd_ise_bits[21] = {1, 0, 2, 0, 1, 3, 1, 2, 4, 2, 3,
                                          5, 3, 4, 6, 4, 5, 7, 5, 6, 8};
static const uint8_t tacd_ise_has_trit[21] = {
    0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0};
static const uint8_t tacd_ise_has_quint[21] = {
    0, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0};

static const uint8_t tacd_integer_of_quints[5][5][5] = {
    {{0, 1, 2, 3, 4}, {8, 9, 10, 11, 12}, {16, 17, 18, 19, 20},
     {24, 25, 26, 27, 28}, {5, 13, 21, 29, 6}},
    {{32, 33, 34, 35, 36}, {40, 41, 42, 43, 44}, {48, 49, 50, 51, 52},
     {56, 57, 58, 59, 60}, {37, 45, 53, 61, 14}},
    {{64, 65, 66, 67, 68}, {72, 73, 74, 75, 76}, {80, 81, 82, 83, 84},
     {88, 89, 90, 91, 92}, {69, 77, 85, 93, 22}},
    {{96, 97, 98, 99, 100}, {104, 105, 106, 107, 108}, {112, 113, 114, 115, 116},
     {120, 121, 122, 123, 124}, {101, 109, 117, 125, 30}},
    {{102, 103, 70, 71, 38}, {110, 111, 78, 79, 46}, {118, 119, 86, 87, 54},
     {126, 127, 94, 95, 62}, {39, 47, 55, 63, 31}}};

static const uint8_t tacd_integer_of_trits[3][3][3][3][3] = {
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

static uint8_t tacd_trits_of_integer[256][5];
static uint8_t tacd_quints_of_integer[128][3];
static int tacd_inverse_tables_ready = 0;

static void tacd_init_inverse_tables(void) {
    uint32_t i0, i1, i2, i3, i4;
    if (tacd_inverse_tables_ready) return;
    memset(tacd_trits_of_integer, 0xff, sizeof(tacd_trits_of_integer));
    memset(tacd_quints_of_integer, 0xff, sizeof(tacd_quints_of_integer));
    for (i4 = 0; i4 < 3u; ++i4)
        for (i3 = 0; i3 < 3u; ++i3)
            for (i2 = 0; i2 < 3u; ++i2)
                for (i1 = 0; i1 < 3u; ++i1)
                    for (i0 = 0; i0 < 3u; ++i0) {
                        uint8_t t = tacd_integer_of_trits[i4][i3][i2][i1][i0];
                        tacd_trits_of_integer[t][0] = (uint8_t)i0;
                        tacd_trits_of_integer[t][1] = (uint8_t)i1;
                        tacd_trits_of_integer[t][2] = (uint8_t)i2;
                        tacd_trits_of_integer[t][3] = (uint8_t)i3;
                        tacd_trits_of_integer[t][4] = (uint8_t)i4;
                    }
    for (i2 = 0; i2 < 5u; ++i2)
        for (i1 = 0; i1 < 5u; ++i1)
            for (i0 = 0; i0 < 5u; ++i0) {
                uint8_t q = tacd_integer_of_quints[i2][i1][i0];
                tacd_quints_of_integer[q][0] = (uint8_t)i0;
                tacd_quints_of_integer[q][1] = (uint8_t)i1;
                tacd_quints_of_integer[q][2] = (uint8_t)i2;
            }
    tacd_inverse_tables_ready = 1;
}

/* ---- unquantization tables (ASTC spec data, ISE-symbol indexed) --------- */

static const uint8_t tacd_weight_unquant_q2[2] = {0, 64};
static const uint8_t tacd_weight_unquant_q3[3] = {0, 32, 64};
static const uint8_t tacd_weight_unquant_q4[4] = {0, 21, 43, 64};
static const uint8_t tacd_weight_unquant_q5[5] = {0, 16, 32, 48, 64};
static const uint8_t tacd_weight_unquant_q6[6] = {0, 64, 12, 52, 25, 39};
static const uint8_t tacd_weight_unquant_q8[8] = {0, 9, 18, 27, 37, 46, 55, 64};
static const uint8_t tacd_weight_unquant_q10[10] = {0, 64, 7, 57, 14, 50, 21, 43, 28, 36};
static const uint8_t tacd_weight_unquant_q12[12] = {0, 64, 17, 47, 5, 59, 23, 41, 11, 53, 28, 36};
static const uint8_t tacd_weight_unquant_q16[16] = {0, 4, 8, 12, 17, 21, 25, 29, 35, 39, 43, 47, 52, 56, 60, 64};
static const uint8_t tacd_weight_unquant_q20[20] = {0, 64, 16, 48, 3, 61, 19, 45, 6, 58, 23, 41, 9, 55, 26, 38, 13, 51, 29, 35};
static const uint8_t tacd_weight_unquant_q24[24] = {0, 64, 8, 56, 16, 48, 24, 40, 2, 62, 11, 53, 19, 45, 27, 37, 5, 59, 13, 51, 22, 42, 30, 34};
static const uint8_t tacd_weight_unquant_q32[32] = {0, 2, 4, 6, 8, 10, 12, 14, 16, 18, 20, 22, 24, 26, 28, 30, 34, 36, 38, 40, 42, 44, 46, 48, 50, 52, 54, 56, 58, 60, 62, 64};

static const uint8_t *const tacd_weight_unquant[12] = {
    tacd_weight_unquant_q2,  tacd_weight_unquant_q3,  tacd_weight_unquant_q4,
    tacd_weight_unquant_q5,  tacd_weight_unquant_q6,  tacd_weight_unquant_q8,
    tacd_weight_unquant_q10, tacd_weight_unquant_q12, tacd_weight_unquant_q16,
    tacd_weight_unquant_q20, tacd_weight_unquant_q24, tacd_weight_unquant_q32};

static const uint8_t tacd_color_unquant_q6[6] = {0, 255, 51, 204, 102, 153};
static const uint8_t tacd_color_unquant_q8[8] = {0, 36, 73, 109, 146, 182, 219, 255};
static const uint8_t tacd_color_unquant_q10[10] = {0, 255, 28, 227, 56, 199, 84, 171, 113, 142};
static const uint8_t tacd_color_unquant_q12[12] = {0, 255, 69, 186, 23, 232, 92, 163, 46, 209, 116, 139};
static const uint8_t tacd_color_unquant_q16[16] = {0, 17, 34, 51, 68, 85, 102, 119, 136, 153, 170, 187, 204, 221, 238, 255};
static const uint8_t tacd_color_unquant_q20[20] = {0, 255, 67, 188, 13, 242, 80, 175, 27, 228, 94, 161, 40, 215, 107, 148, 54, 201, 121, 134};
static const uint8_t tacd_color_unquant_q24[24] = {0, 255, 33, 222, 66, 189, 99, 156, 11, 244, 44, 211, 77, 178, 110, 145, 22, 233, 55, 200, 88, 167, 121, 134};
static const uint8_t tacd_color_unquant_q32[32] = {0, 8, 16, 24, 33, 41, 49, 57, 66, 74, 82, 90, 99, 107, 115, 123, 132, 140, 148, 156, 165, 173, 181, 189, 198, 206, 214, 222, 231, 239, 247, 255};
static const uint8_t tacd_color_unquant_q40[40] = {0, 255, 32, 223, 65, 190, 97, 158, 6, 249, 39, 216, 71, 184, 104, 151, 13, 242, 45, 210, 78, 177, 110, 145, 19, 236, 52, 203, 84, 171, 117, 138, 26, 229, 58, 197, 91, 164, 123, 132};
static const uint8_t tacd_color_unquant_q48[48] = {0, 255, 16, 239, 32, 223, 48, 207, 65, 190, 81, 174, 97, 158, 113, 142, 5, 250, 21, 234, 38, 217, 54, 201, 70, 185, 86, 169, 103, 152, 119, 136, 11, 244, 27, 228, 43, 212, 59, 196, 76, 179, 92, 163, 108, 147, 124, 131};
static const uint8_t tacd_color_unquant_q64[64] = {0, 4, 8, 12, 16, 20, 24, 28, 32, 36, 40, 44, 48, 52, 56, 60, 65, 69, 73, 77, 81, 85, 89, 93, 97, 101, 105, 109, 113, 117, 121, 125, 130, 134, 138, 142, 146, 150, 154, 158, 162, 166, 170, 174, 178, 182, 186, 190, 195, 199, 203, 207, 211, 215, 219, 223, 227, 231, 235, 239, 243, 247, 251, 255};
static const uint8_t tacd_color_unquant_q80[80] = {0, 255, 16, 239, 32, 223, 48, 207, 64, 191, 80, 175, 96, 159, 112, 143, 3, 252, 19, 236, 35, 220, 51, 204, 67, 188, 83, 172, 100, 155, 116, 139, 6, 249, 22, 233, 38, 217, 54, 201, 71, 184, 87, 168, 103, 152, 119, 136, 9, 246, 25, 230, 42, 213, 58, 197, 74, 181, 90, 165, 106, 149, 122, 133, 13, 242, 29, 226, 45, 210, 61, 194, 77, 178, 93, 162, 109, 146, 125, 130};
static const uint8_t tacd_color_unquant_q96[96] = {0, 255, 8, 247, 16, 239, 24, 231, 32, 223, 40, 215, 48, 207, 56, 199, 64, 191, 72, 183, 80, 175, 88, 167, 96, 159, 104, 151, 112, 143, 120, 135, 2, 253, 10, 245, 18, 237, 26, 229, 35, 220, 43, 212, 51, 204, 59, 196, 67, 188, 75, 180, 83, 172, 91, 164, 99, 156, 107, 148, 115, 140, 123, 132, 5, 250, 13, 242, 21, 234, 29, 226, 37, 218, 45, 210, 53, 202, 61, 194, 70, 185, 78, 177, 86, 169, 94, 161, 102, 153, 110, 145, 118, 137, 126, 129};
static const uint8_t tacd_color_unquant_q128[128] = {0, 2, 4, 6, 8, 10, 12, 14, 16, 18, 20, 22, 24, 26, 28, 30, 32, 34, 36, 38, 40, 42, 44, 46, 48, 50, 52, 54, 56, 58, 60, 62, 64, 66, 68, 70, 72, 74, 76, 78, 80, 82, 84, 86, 88, 90, 92, 94, 96, 98, 100, 102, 104, 106, 108, 110, 112, 114, 116, 118, 120, 122, 124, 126, 129, 131, 133, 135, 137, 139, 141, 143, 145, 147, 149, 151, 153, 155, 157, 159, 161, 163, 165, 167, 169, 171, 173, 175, 177, 179, 181, 183, 185, 187, 189, 191, 193, 195, 197, 199, 201, 203, 205, 207, 209, 211, 213, 215, 217, 219, 221, 223, 225, 227, 229, 231, 233, 235, 237, 239, 241, 243, 245, 247, 249, 251, 253, 255};
static const uint8_t tacd_color_unquant_q160[160] = {0, 255, 8, 247, 16, 239, 24, 231, 32, 223, 40, 215, 48, 207, 56, 199, 64, 191, 72, 183, 80, 175, 88, 167, 96, 159, 104, 151, 112, 143, 120, 135, 1, 254, 9, 246, 17, 238, 25, 230, 33, 222, 41, 214, 49, 206, 57, 198, 65, 190, 73, 182, 81, 174, 89, 166, 97, 158, 105, 150, 113, 142, 121, 134, 3, 252, 11, 244, 19, 236, 27, 228, 35, 220, 43, 212, 51, 204, 59, 196, 67, 188, 75, 180, 83, 172, 91, 164, 99, 156, 107, 148, 115, 140, 123, 132, 4, 251, 12, 243, 20, 235, 28, 227, 36, 219, 44, 211, 52, 203, 60, 195, 68, 187, 76, 179, 84, 171, 92, 163, 100, 155, 108, 147, 116, 139, 124, 131, 6, 249, 14, 241, 22, 233, 30, 225, 38, 217, 46, 209, 54, 201, 62, 193, 70, 185, 78, 177, 86, 169, 94, 161, 102, 153, 110, 145, 118, 137, 126, 129};
static const uint8_t tacd_color_unquant_q192[192] = {0, 255, 4, 251, 8, 247, 12, 243, 16, 239, 20, 235, 24, 231, 28, 227, 32, 223, 36, 219, 40, 215, 44, 211, 48, 207, 52, 203, 56, 199, 60, 195, 64, 191, 68, 187, 72, 183, 76, 179, 80, 175, 84, 171, 88, 167, 92, 163, 96, 159, 100, 155, 104, 151, 108, 147, 112, 143, 116, 139, 120, 135, 124, 131, 1, 254, 5, 250, 9, 246, 13, 242, 17, 238, 21, 234, 25, 230, 29, 226, 33, 222, 37, 218, 41, 214, 45, 210, 49, 206, 53, 202, 57, 198, 61, 194, 65, 190, 69, 186, 73, 182, 77, 178, 81, 174, 85, 170, 89, 166, 93, 162, 97, 158, 101, 154, 105, 150, 109, 146, 113, 142, 117, 138, 121, 134, 125, 130, 2, 253, 6, 249, 10, 245, 14, 241, 18, 237, 22, 233, 26, 229, 30, 225, 34, 221, 38, 217, 42, 213, 46, 209, 50, 205, 54, 201, 58, 197, 62, 193, 66, 189, 70, 185, 74, 181, 78, 177, 82, 173, 86, 169, 90, 165, 94, 161, 98, 157, 102, 153, 106, 149, 110, 145, 114, 141, 118, 137, 122, 133, 126, 129};

static const uint8_t *const tacd_color_unquant[16] = {
    tacd_color_unquant_q6,   tacd_color_unquant_q8,  tacd_color_unquant_q10,
    tacd_color_unquant_q12,  tacd_color_unquant_q16, tacd_color_unquant_q20,
    tacd_color_unquant_q24,  tacd_color_unquant_q32, tacd_color_unquant_q40,
    tacd_color_unquant_q48,  tacd_color_unquant_q64, tacd_color_unquant_q80,
    tacd_color_unquant_q96,  tacd_color_unquant_q128, tacd_color_unquant_q160,
    tacd_color_unquant_q192};

/* ---- bit access ---------------------------------------------------------- */

static uint32_t tacd_rd_bits(const uint8_t *p, uint32_t bitpos, uint32_t nbits) {
    uint32_t v = 0, i;
    for (i = 0; i < nbits; ++i)
        v |= (uint32_t)((p[(bitpos + i) >> 3] >> ((bitpos + i) & 7u)) & 1u) << i;
    return v;
}

static uint8_t tacd_bitrev8(uint8_t v) {
    v = (uint8_t)(((v & 0x0fu) << 4) | ((v >> 4) & 0x0fu));
    v = (uint8_t)(((v & 0x33u) << 2) | ((v >> 2) & 0x33u));
    v = (uint8_t)(((v & 0x55u) << 1) | ((v >> 1) & 0x55u));
    return v;
}

static unsigned int tacd_ise_bitcount(unsigned int value_count, unsigned int quant_level) {
    static const uint8_t scale[21] = {1, 8, 2, 7, 13, 3, 10, 18, 4, 13, 23, 5, 16, 28, 6, 19, 33, 7, 22, 38, 8};
    static const uint8_t div_code[21] = {0, 2, 0, 1, 2, 0, 1, 2, 0, 1, 2, 0, 1, 2, 0, 1, 2, 0, 1, 2, 0};
    unsigned int divisor;
    if (quant_level >= 21u) return 1024u;
    divisor = (unsigned int)div_code[quant_level] * 2u + 1u;
    return ((unsigned int)scale[quant_level] * value_count + divisor - 1u) / divisor;
}

/* ---- ISE decode ---------------------------------------------------------- */

static int tacd_ise_decode(uint32_t quant_level, uint32_t value_count,
                           const uint8_t *p, uint32_t bit_offset, uint8_t *out_symbols) {
    uint32_t nbits, i;
    if (quant_level >= 21u || !value_count) return 0;
    tacd_init_inverse_tables();
    nbits = tacd_ise_bits[quant_level];
    if (tacd_ise_has_trit[quant_level]) {
        static const uint8_t tbits[5] = {2, 2, 1, 2, 1};
        static const uint8_t tshift[5] = {0, 2, 4, 5, 7};
        i = 0;
        while (i < value_count) {
            uint32_t group = value_count - i < 5u ? value_count - i : 5u;
            uint32_t low[5], t = 0, j;
            for (j = 0; j < group; ++j) {
                low[j] = tacd_rd_bits(p, bit_offset, nbits);
                bit_offset += nbits;
                t |= tacd_rd_bits(p, bit_offset, tbits[j]) << tshift[j];
                bit_offset += tbits[j];
            }
            if (tacd_trits_of_integer[t][0] == 0xffu && tacd_trits_of_integer[t][1] == 0xffu)
                return 0;
            for (j = 0; j < group; ++j)
                out_symbols[i + j] = (uint8_t)((tacd_trits_of_integer[t][j] << nbits) | low[j]);
            i += group;
        }
        return 1;
    }
    if (tacd_ise_has_quint[quant_level]) {
        static const uint8_t qbits[3] = {3, 2, 2};
        static const uint8_t qshift[3] = {0, 3, 5};
        i = 0;
        while (i < value_count) {
            uint32_t group = value_count - i < 3u ? value_count - i : 3u;
            uint32_t low[3], q = 0, j;
            for (j = 0; j < group; ++j) {
                low[j] = tacd_rd_bits(p, bit_offset, nbits);
                bit_offset += nbits;
                q |= tacd_rd_bits(p, bit_offset, qbits[j]) << qshift[j];
                bit_offset += qbits[j];
            }
            if (q >= 128u || (tacd_quints_of_integer[q][0] == 0xffu && tacd_quints_of_integer[q][1] == 0xffu))
                return 0;
            for (j = 0; j < group; ++j)
                out_symbols[i + j] = (uint8_t)((tacd_quints_of_integer[q][j] << nbits) | low[j]);
            i += group;
        }
        return 1;
    }
    for (i = 0; i < value_count; ++i)
        out_symbols[i] = (uint8_t)tacd_rd_bits(p, bit_offset, nbits), bit_offset += nbits;
    return 1;
}

/* ---- block mode decode (spec C.2.10, 2D) --------------------------------- */

static int tacd_decode_block_mode_2d(uint32_t mode, uint32_t *wx, uint32_t *wy,
                                     uint32_t *quant, uint32_t *dual) {
    uint32_t base_quant = (mode >> 4) & 1u;
    uint32_t h = (mode >> 9) & 1u;
    uint32_t d = (mode >> 10) & 1u;
    uint32_t a = (mode >> 5) & 3u;
    uint32_t x = 0, y = 0;
    if ((mode & 3u) != 0u) {
        uint32_t b;
        base_quant |= (mode & 3u) << 1;
        b = (mode >> 7) & 3u;
        switch ((mode >> 2) & 3u) {
            case 0: x = b + 4u; y = a + 2u; break;
            case 1: x = b + 8u; y = a + 2u; break;
            case 2: x = a + 2u; y = b + 8u; break;
            default:
                b &= 1u;
                if (mode & 0x100u) { x = b + 2u; y = a + 2u; }
                else { x = a + 2u; y = b + 6u; }
                break;
        }
    } else {
        base_quant |= ((mode >> 2) & 3u) << 1;
        if (((mode >> 2) & 3u) == 0u) return 0;
        uint32_t b = (mode >> 9) & 3u;
        switch ((mode >> 7) & 3u) {
            case 0: x = 12u; y = a + 2u; break;
            case 1: x = a + 2u; y = 12u; break;
            case 2: x = a + 6u; y = b + 6u; d = 0u; h = 0u; break;
            default:
                if (a == 0u) { x = 6u; y = 10u; }
                else if (a == 1u) { x = 10u; y = 6u; }
                else return 0;
                break;
        }
    }
    if (base_quant < 2u) return 0;
    *wx = x; *wy = y; *quant = (base_quant - 2u) + 6u * h; *dual = d;
    return *quant < 12u && x * y * (d + 1u) <= 64u;
}

/* ---- partition hash (spec C.2.21) ---------------------------------------- */

static uint32_t tacd_hash52(uint32_t inp) {
    inp ^= inp >> 15; inp *= 0xeede0891u;
    inp ^= inp >> 5; inp += inp << 16;
    inp ^= inp >> 7; inp ^= inp >> 3;
    inp ^= inp << 6; inp ^= inp >> 17;
    return inp;
}

static uint32_t tacd_select_partition(uint32_t seed, uint32_t x, uint32_t y,
                                      uint32_t partition_count, int small_block) {
    uint32_t rnum, s1, s2, s3, s4, s5, s6, s7, s8, s9, s10, s11, s12;
    uint32_t sh1, sh2, sh3, a, b, c, d;
    if (small_block) { x <<= 1; y <<= 1; }
    seed += (partition_count - 1u) * 1024u;
    rnum = tacd_hash52(seed);
    s1 = rnum & 15u; s2 = (rnum >> 4) & 15u; s3 = (rnum >> 8) & 15u;
    s4 = (rnum >> 12) & 15u; s5 = (rnum >> 16) & 15u; s6 = (rnum >> 20) & 15u;
    s7 = (rnum >> 24) & 15u; s8 = (rnum >> 28) & 15u;
    s9 = (rnum >> 18) & 15u; s10 = (rnum >> 22) & 15u; s11 = (rnum >> 26) & 15u;
    s12 = ((rnum >> 30) | (rnum << 2)) & 15u;
    s1 *= s1; s2 *= s2; s3 *= s3; s4 *= s4; s5 *= s5; s6 *= s6;
    s7 *= s7; s8 *= s8; s9 *= s9; s10 *= s10; s11 *= s11; s12 *= s12;
    if (seed & 1u) { sh1 = (seed & 2u) ? 4u : 5u; sh2 = partition_count == 3u ? 6u : 5u; }
    else { sh1 = partition_count == 3u ? 6u : 5u; sh2 = (seed & 2u) ? 4u : 5u; }
    sh3 = (seed & 0x10u) ? sh1 : sh2;
    s1 >>= sh1; s2 >>= sh2; s3 >>= sh1; s4 >>= sh2; s5 >>= sh1; s6 >>= sh2;
    s7 >>= sh1; s8 >>= sh2; s9 >>= sh3; s10 >>= sh3; s11 >>= sh3; s12 >>= sh3;
    a = s1 * x + s2 * y + (rnum >> 14); b = s3 * x + s4 * y + (rnum >> 10);
    c = s5 * x + s6 * y + (rnum >> 6); d = s7 * x + s8 * y + (rnum >> 2);
    a &= 63u; b &= 63u; c &= partition_count > 2u ? 63u : 0u; d &= partition_count > 3u ? 63u : 0u;
    if (a >= b && a >= c && a >= d) return 0;
    if (b >= c && b >= d) return 1;
    if (c >= d) return 2;
    return 3;
}

/* ---- weight infill (spec C.2.18) ------------------------------------------ */

static uint32_t tacd_infill_weight(const uint8_t *grid, uint32_t gw, uint32_t gh,
                                   uint32_t bx, uint32_t by, uint32_t x, uint32_t y) {
    uint32_t ds = (1024u + bx / 2u) / (bx - 1u);
    uint32_t dt = (1024u + by / 2u) / (by - 1u);
    uint32_t cs = ds * x, ct = dt * y;
    uint32_t gs = (cs * (gw - 1u) + 32u) >> 6;
    uint32_t gt = (ct * (gh - 1u) + 32u) >> 6;
    uint32_t js = gs >> 4, fs = gs & 15u;
    uint32_t jt = gt >> 4, ft = gt & 15u;
    uint32_t w11 = (fs * ft + 8u) >> 4;
    uint32_t w10 = ft - w11, w01 = fs - w11, w00 = 16u - fs - ft + w11;
    uint32_t v0 = js + jt * gw;
    uint32_t p00 = grid[v0], p01 = (js + 1u < gw) ? grid[v0 + 1u] : 0u;
    uint32_t p10 = (jt + 1u < gh) ? grid[v0 + gw] : 0u;
    uint32_t p11 = (js + 1u < gw && jt + 1u < gh) ? grid[v0 + gw + 1u] : 0u;
    return (p00 * w00 + p01 * w01 + p10 * w10 + p11 * w11 + 8u) >> 4;
}

/* ---- CEM decode (spec C.2.14, LDR subset) --------------------------------- */

static void tacd_blue_contract(uint32_t v[4]) {
    v[0] = (v[0] + v[2]) >> 1; v[1] = (v[1] + v[2]) >> 1;
}

static uint32_t tacd_decode_cem(uint32_t cem, const uint8_t *v, uint32_t e0[4], uint32_t e1[4]) {
    switch (cem) {
        case 0u:
            e0[0] = e0[1] = e0[2] = v[0]; e1[0] = e1[1] = e1[2] = v[1];
            e0[3] = e1[3] = 255u; return 2u;
        case 4u:
            e0[0] = e0[1] = e0[2] = v[0]; e1[0] = e1[1] = e1[2] = v[1];
            e0[3] = v[2]; e1[3] = v[3]; return 4u;
        case 6u:
            e0[0] = (v[0] * v[3]) >> 8; e0[1] = (v[1] * v[3]) >> 8; e0[2] = (v[2] * v[3]) >> 8;
            e1[0] = v[0]; e1[1] = v[1]; e1[2] = v[2];
            e0[3] = e1[3] = 255u; return 4u;
        case 8u:
        case 12u: {
            uint32_t s0 = (uint32_t)v[0] + v[2] + v[4];
            uint32_t s1 = (uint32_t)v[1] + v[3] + v[5];
            uint32_t rgb0[4], rgb1[4];
            if (s1 >= s0) {
                rgb0[0] = v[0]; rgb0[1] = v[2]; rgb0[2] = v[4];
                rgb1[0] = v[1]; rgb1[1] = v[3]; rgb1[2] = v[5];
            } else {
                rgb0[0] = v[1]; rgb0[1] = v[3]; rgb0[2] = v[5];
                rgb1[0] = v[0]; rgb1[1] = v[2]; rgb1[2] = v[4];
                tacd_blue_contract(rgb0); tacd_blue_contract(rgb1);
            }
            e0[0] = rgb0[0]; e0[1] = rgb0[1]; e0[2] = rgb0[2];
            e1[0] = rgb1[0]; e1[1] = rgb1[1]; e1[2] = rgb1[2];
            if (cem == 12u) {
                if (s1 >= s0) { e0[3] = v[6]; e1[3] = v[7]; }
                else { e0[3] = v[7]; e1[3] = v[6]; }
                return 8u;
            }
            e0[3] = e1[3] = 255u; return 6u;
        }
        case 10u:
            e0[0] = (v[0] * v[3]) >> 8; e0[1] = (v[1] * v[3]) >> 8; e0[2] = (v[2] * v[3]) >> 8;
            e1[0] = v[0]; e1[1] = v[1]; e1[2] = v[2];
            e0[3] = v[4]; e1[3] = v[5]; return 6u;
        default:
            return 0u;
    }
}

/* ---- public API: decode one block ---------------------------------------- */

int tc_astc_decode_block_rgba8(const uint8_t block[16], uint32_t bx, uint32_t by,
                               uint8_t out_rgba[16*4]) {
    uint32_t mode = tacd_rd_bits(block, 0, 11);
    uint32_t wx, wy, wquant, dual;
    uint32_t part_count, weight_count, weight_bits;
    uint32_t pindex = 0, extra_bits = 0, color_start, ccs = 3u, ccs_bits = 0;
    uint32_t cems[4] = {0, 0, 0, 0};
    uint32_t e0[4][4], e1[4][4];
    uint8_t rev[16];
    uint8_t wsyms[64];
    uint8_t wgrid[2][64];
    uint8_t csyms[18], vals[18];
    uint32_t vcount = 0, used, avail, i, x, y;
    int cq = -1;
    int small_block;

    /* Void-extent / constant block */
    if ((mode & 0x1ffu) == 0x1fcu) {
        uint32_t c;
        uint8_t px[4];
        if (mode & (1u << 9)) return 0;
        for (c = 0; c < 4u; ++c) {
            uint32_t v16 = (uint32_t)block[8u + c * 2u] | ((uint32_t)block[9u + c * 2u] << 8);
            px[c] = (uint8_t)(v16 >> 8);
        }
        for (i = 0; i < bx * by; ++i) memcpy(out_rgba + i * 4u, px, 4u);
        return 1;
    }

    if (!tacd_decode_block_mode_2d(mode, &wx, &wy, &wquant, &dual)) return 0;
    if (wx > bx || wy > by) return 0;
    part_count = tacd_rd_bits(block, 11, 2) + 1u;
    if (dual && part_count == 4u) return 0;
    weight_count = wx * wy * (dual ? 2u : 1u);
    if (weight_count > 64u) return 0;
    weight_bits = (uint32_t)tacd_ise_bitcount(weight_count, wquant);
    if (weight_bits < 24u || weight_bits > 96u) return 0;

    for (i = 0; i < 16u; ++i) rev[i] = tacd_bitrev8(block[15u - i]);
    if (!tacd_ise_decode(wquant, weight_count, rev, 0, wsyms)) return 0;
    for (i = 0; i < wx * wy; ++i) {
        if (dual) {
            wgrid[0][i] = tacd_weight_unquant[wquant][wsyms[i * 2u]];
            wgrid[1][i] = tacd_weight_unquant[wquant][wsyms[i * 2u + 1u]];
        } else {
            wgrid[0][i] = tacd_weight_unquant[wquant][wsyms[i]];
        }
    }

    if (part_count == 1u) {
        cems[0] = tacd_rd_bits(block, 13, 4);
        color_start = 17u;
    } else {
        uint32_t cf;
        pindex = tacd_rd_bits(block, 13, 10);
        cf = tacd_rd_bits(block, 23, 6);
        color_start = 29u;
        if ((cf & 3u) == 0u) {
            for (i = 0; i < part_count; ++i) cems[i] = cf >> 2;
        } else {
            uint32_t base_class = (cf & 3u) - 1u;
            uint32_t code, ext_pos;
            extra_bits = 3u * part_count - 4u;
            if (weight_bits + extra_bits > 128u - color_start) return 0;
            ext_pos = 128u - weight_bits - extra_bits;
            code = cf | (tacd_rd_bits(block, ext_pos, extra_bits) << 6);
            for (i = 0; i < part_count; ++i) {
                uint32_t ci = (code >> (2u + i)) & 1u;
                uint32_t mi = (code >> (2u + part_count + 2u * i)) & 3u;
                cems[i] = ((base_class + ci) << 2) | mi;
            }
        }
    }

    if (dual) {
        ccs_bits = 2u;
        if (weight_bits + extra_bits + 2u > 128u - color_start) return 0;
        ccs = tacd_rd_bits(block, 128u - weight_bits - extra_bits - 2u, 2);
    }

    for (i = 0; i < part_count; ++i)
        vcount += (((cems[i] >> 2) + 1u) * 2u);
    if (vcount > 18u) return 0;
    used = color_start + weight_bits + extra_bits + ccs_bits;
    if (used > 128u) return 0;
    avail = 128u - used;
    for (i = 20u; i >= 4u; --i) {
        if (tacd_ise_bitcount(vcount, i) <= avail) { cq = (int)i; break; }
        if (i == 4u) break;
    }
    if (cq < 0) return 0;
    if (!tacd_ise_decode((uint32_t)cq, vcount, block, color_start, csyms)) return 0;
    for (i = 0; i < vcount; ++i)
        vals[i] = cq == 20 ? csyms[i] : tacd_color_unquant[cq - 4][csyms[i]];

    {
        uint32_t off = 0;
        for (i = 0; i < part_count; ++i) {
            uint32_t n = tacd_decode_cem(cems[i], vals + off, e0[i], e1[i]);
            if (!n) return 0;
            off += n;
        }
        if (off != vcount) return 0;
    }

    small_block = (int)(bx * by < 32u);
    for (y = 0; y < by; ++y) {
        for (x = 0; x < bx; ++x) {
            uint32_t part = part_count == 1u ? 0u : tacd_select_partition(pindex, x, y, part_count, small_block);
            uint32_t w0 = tacd_infill_weight(wgrid[0], wx, wy, bx, by, x, y);
            uint32_t w1 = dual ? tacd_infill_weight(wgrid[1], wx, wy, bx, by, x, y) : w0;
            uint32_t c;
            if (part >= part_count) part = part_count - 1u;
            for (c = 0; c < 4u; ++c) {
                uint32_t w = (dual && c == ccs) ? w1 : w0;
                uint32_t recon = (e0[part][c] * (64u - w) + e1[part][c] * w + 32u) >> 6;
                out_rgba[(y * bx + x) * 4u + c] = (uint8_t)recon;
            }
        }
    }
    return 1;
}

int tc_astc_decode_image_rgba8(const uint8_t *blocks, uint32_t width,
                               uint32_t height, uint32_t bx, uint32_t by,
                               uint8_t *out_rgba) {
    uint32_t bw = (width + bx - 1u) / bx;
    uint32_t bh = (height + by - 1u) / by;
    uint8_t tmp[144 * 4];
    uint32_t i, j, x, y;
    for (j = 0; j < bh; ++j) {
        for (i = 0; i < bw; ++i) {
            const uint8_t *blk = blocks + ((size_t)j * bw + i) * 16u;
            if (!tc_astc_decode_block_rgba8(blk, bx, by, tmp)) return 0;
            for (y = 0; y < by; ++y) {
                uint32_t py = j * by + y;
                if (py >= height) break;
                for (x = 0; x < bx; ++x) {
                    uint32_t px = i * bx + x;
                    if (px >= width) break;
                    memcpy(out_rgba + ((size_t)py * width + px) * 4u, tmp + (y * bx + x) * 4u, 4u);
                }
            }
        }
    }
    return 1;
}
