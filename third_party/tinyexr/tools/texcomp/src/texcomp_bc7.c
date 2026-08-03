/*
 * TinyEXR texcomp - BC7 encoder
 *
 * Copyright (c) 2014-2026 Syoyo Fujita and TinyEXR authors
 * SPDX-License-Identifier: Apache-2.0
 */

#include "texcomp.h"
#include "texcomp_internal.h"

#include <limits.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>

const uint32_t tc_bc7_weights4[16] = {0,  4,  9,  13, 17, 21, 26, 30,
                                             34, 38, 43, 47, 51, 55, 60, 64};
static const uint32_t tc_bc7_weights3[8] = {0, 9, 18, 27, 37, 46, 55, 64};
static const uint32_t tc_bc7_weights2[4] = {0, 21, 43, 64};

static const uint8_t tc_bc7_num_subsets[8] = {3, 2, 3, 2, 1, 1, 1, 2};
static const uint8_t tc_bc7_partition_bits[8] = {4, 6, 6, 6, 0, 0, 0, 6};
static const uint8_t tc_bc7_color_index_bits[8] = {3, 3, 2, 2, 2, 2, 4, 2};
static const uint8_t tc_bc7_alpha_index_bits[8] = {0, 0, 0, 0, 3, 2, 4, 2};
static const uint8_t tc_bc7_color_precision[8] = {4, 6, 5, 7, 5, 7, 7, 5};
static const uint8_t tc_bc7_alpha_precision[8] = {0, 0, 0, 0, 6, 8, 7, 5};
static const uint8_t tc_bc7_has_pbits[8] = {1, 1, 0, 1, 0, 0, 1, 1};
static const uint8_t tc_bc7_shared_pbits[8] = {0, 1, 0, 0, 0, 0, 0, 0};
static const uint8_t tc_bc7_sep_alpha[8] = {0, 0, 0, 0, 1, 1, 0, 0};

static const uint8_t tc_bc7_partition1[16] = {0, 0, 0, 0, 0, 0, 0, 0,
                                              0, 0, 0, 0, 0, 0, 0, 0};
static const uint8_t tc_bc7_partition2[64 * 16] = {
    0,0,1,1,0,0,1,1,0,0,1,1,0,0,1,1,    0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,    0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,    0,0,0,1,0,0,1,1,0,0,1,1,0,1,1,1,    0,0,0,0,0,0,0,1,0,0,0,1,0,0,1,1,    0,0,1,1,0,1,1,1,0,1,1,1,1,1,1,1,    0,0,0,1,0,0,1,1,0,1,1,1,1,1,1,1,    0,0,0,0,0,0,0,1,0,0,1,1,0,1,1,1,
    0,0,0,0,0,0,0,0,0,0,0,1,0,0,1,1,    0,0,1,1,0,1,1,1,1,1,1,1,1,1,1,1,    0,0,0,0,0,0,0,1,0,1,1,1,1,1,1,1,    0,0,0,0,0,0,0,0,0,0,0,1,0,1,1,1,    0,0,0,1,0,1,1,1,1,1,1,1,1,1,1,1,    0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,    0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,    0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,
    0,0,0,0,1,0,0,0,1,1,1,0,1,1,1,1,    0,1,1,1,0,0,0,1,0,0,0,0,0,0,0,0,    0,0,0,0,0,0,0,0,1,0,0,0,1,1,1,0,    0,1,1,1,0,0,1,1,0,0,0,1,0,0,0,0,    0,0,1,1,0,0,0,1,0,0,0,0,0,0,0,0,    0,0,0,0,1,0,0,0,1,1,0,0,1,1,1,0,    0,0,0,0,0,0,0,0,1,0,0,0,1,1,0,0,    0,1,1,1,0,0,1,1,0,0,1,1,0,0,0,1,
    0,0,1,1,0,0,0,1,0,0,0,1,0,0,0,0,    0,0,0,0,1,0,0,0,1,0,0,0,1,1,0,0,    0,1,1,0,0,1,1,0,0,1,1,0,0,1,1,0,    0,0,1,1,0,1,1,0,0,1,1,0,1,1,0,0,    0,0,0,1,0,1,1,1,1,1,1,0,1,0,0,0,    0,0,0,0,1,1,1,1,1,1,1,1,0,0,0,0,    0,1,1,1,0,0,0,1,1,0,0,0,1,1,1,0,    0,0,1,1,1,0,0,1,1,0,0,1,1,1,0,0,
    0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,    0,0,0,0,1,1,1,1,0,0,0,0,1,1,1,1,    0,1,0,1,1,0,1,0,0,1,0,1,1,0,1,0,    0,0,1,1,0,0,1,1,1,1,0,0,1,1,0,0,    0,0,1,1,1,1,0,0,0,0,1,1,1,1,0,0,    0,1,0,1,0,1,0,1,1,0,1,0,1,0,1,0,    0,1,1,0,1,0,0,1,0,1,1,0,1,0,0,1,    0,1,0,1,1,0,1,0,1,0,1,0,0,1,0,1,
    0,1,1,1,0,0,1,1,1,1,0,0,1,1,1,0,    0,0,0,1,0,0,1,1,1,1,0,0,1,0,0,0,    0,0,1,1,0,0,1,0,0,1,0,0,1,1,0,0,    0,0,1,1,1,0,1,1,1,1,0,1,1,1,0,0,    0,1,1,0,1,0,0,1,1,0,0,1,0,1,1,0,    0,0,1,1,1,1,0,0,1,1,0,0,0,0,1,1,    0,1,1,0,0,1,1,0,1,0,0,1,1,0,0,1,    0,0,0,0,0,1,1,0,0,1,1,0,0,0,0,0,
    0,1,0,0,1,1,1,0,0,1,0,0,0,0,0,0,    0,0,1,0,0,1,1,1,0,0,1,0,0,0,0,0,    0,0,0,0,0,0,1,0,0,1,1,1,0,0,1,0,    0,0,0,0,0,1,0,0,1,1,1,0,0,1,0,0,    0,1,1,0,1,1,0,0,1,0,0,1,0,0,1,1,    0,0,1,1,0,1,1,0,1,1,0,0,1,0,0,1,    0,1,1,0,0,0,1,1,1,0,0,1,1,1,0,0,    0,0,1,1,1,0,0,1,1,1,0,0,0,1,1,0,
    0,1,1,0,1,1,0,0,1,1,0,0,1,0,0,1,    0,1,1,0,0,0,1,1,0,0,1,1,1,0,0,1,    0,1,1,1,1,1,1,0,1,0,0,0,0,0,0,1,    0,0,0,1,1,0,0,0,1,1,1,0,0,1,1,1,    0,0,0,0,1,1,1,1,0,0,1,1,0,0,1,1,    0,0,1,1,0,0,1,1,1,1,1,1,0,0,0,0,    0,0,1,0,0,0,1,0,1,1,1,0,1,1,1,0,    0,1,0,0,0,1,0,0,0,1,1,1,0,1,1,1
};
static const uint8_t tc_bc7_partition3_0[16] = {0, 0, 1, 1, 0, 0, 1, 1,
                                                0, 2, 2, 1, 2, 2, 2, 2};
static const uint8_t tc_bc7_anchor2[64] = {
    15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,
    15, 2, 8, 2, 2, 8, 8,15, 2, 8, 2, 2, 8, 8, 2, 2,
    15,15, 6, 8, 2, 8,15,15, 2, 8, 2, 2, 2,15,15, 6,
     6, 2, 6, 8,15,15, 2, 2,15,15,15,15,15, 2, 2,15
};
static const uint8_t tc_bc7_anchor3_0[2] = {3, 15};

static const uint8_t tc_bc7_part_lut[8][21] = {
    {5, 9, 12, 27, 28, 47, 49, 51, 53},
    {34, 38, 39, 44},
    {2, 26, 32, 37, 46, 63},
    {16, 21, 25, 31, 43, 55, 59},
    {14, 29, 33, 36, 45, 60},
    {17, 19, 23, 30, 40, 54, 57},
    {48, 52, 56, 58},
    {0, 1, 3, 4, 6, 7, 8, 10, 11, 13, 15, 18, 20, 22, 24, 35, 41, 42, 50, 61, 62}
};
static const uint8_t tc_bc7_alpha_part_lut[8][12] = {
    {4, 7, 8, 11, 52, 60, 63},
    {27, 28, 30, 31, 34, 35, 36, 37, 40, 41, 42, 43},
    {0, 1, 2, 3, 23, 32, 39, 45, 58, 59},
    {18, 21, 22, 25, 54, 62},
    {10, 13, 14, 15, 16, 33, 38, 46, 56, 57},
    {17, 19, 20, 24, 55, 61},
    {5, 6, 9, 12, 53},
    {26, 29, 44, 47, 48, 49, 50, 51}
};

void tc_bc7_options_init(tc_bc7_options *opt) {
    if (!opt) return;
    memset(opt, 0, sizeof(*opt));
    opt->quality = TC_BC7_QUALITY_MEDIUM;
    opt->perceptual = 1;
    opt->quick = 2;
    opt->threads = 1;
    opt->mode_mask = 0xffu;
}

size_t tc_bc7_compressed_size(uint32_t width, uint32_t height) {
    size_t bx, by, blocks;
    if (!width || !height) return 0;
    bx = ((size_t)width + 3u) >> 2;
    by = ((size_t)height + 3u) >> 2;
    if (tc_mul_ovf_size(bx, by, &blocks)) return 0;
    if (blocks > SIZE_MAX / 16u) return 0;
    return blocks * 16u;
}

static uint8_t tc_unquant(uint32_t q, uint32_t bits) {
    uint32_t maxv;
    if (bits >= 8u) return (uint8_t)q;
    maxv = (1u << bits) - 1u;
    return (uint8_t)((q * 255u + maxv / 2u) / maxv);
}

static uint32_t tc_quant(uint32_t v, uint32_t bits) {
    uint32_t maxv = (1u << bits) - 1u;
    return (v * maxv + 127u) / 255u;
}

static uint32_t tc_block_quick_mask(const uint8_t pix[16][4], uint32_t user_mask) {
    uint32_t i;
    uint32_t lmin = UINT_MAX, lmax = 0, amin = UINT_MAX, amax = 0;
    uint32_t has_alpha = 0, lstate, astate, mask = 0;
    for (i = 0; i < 16u; ++i) {
        uint32_t y = tc_luma_u8(pix[i]) >> 7;
        uint32_t a = pix[i][3];
        if (y < lmin) lmin = y;
        if (y > lmax) lmax = y;
        if (a < amin) amin = a;
        if (a > amax) amax = a;
        if (a < 255u) has_alpha = 1;
    }

    lstate = (lmax - lmin <= 19u) ? 0u : ((lmax - lmin <= 48u) ? 1u : 2u);
    if (!has_alpha) {
        if (lstate == 0u) mask = 1u << 6;
        else if (lstate == 1u) mask = (1u << 1) | (1u << 6);
        else mask = 1u << 1;
    } else {
        uint32_t adiff = amax - amin;
        astate = (adiff < 10u) ? 0u : ((adiff <= 21u) ? 1u : 2u);
        if (lstate <= 1u && astate <= 1u) mask |= 1u << 6;
        if (lstate <= 1u && astate >= 1u) mask |= 1u << 5;
        if (lstate >= 1u) mask |= 1u << 7;
        if (!mask) mask = 1u << 6;
    }

    mask &= user_mask;
    return mask ? mask : user_mask;
}

/* Per-channel error weights (R,G,B,A), applied to the squared-error metric.
 * Default {1,1,1,1} makes the weighted expression byte-identical to the plain
 * sum of squares, so uniform weights change nothing. Set once by
 * tc_bc7_compress_rgba8 before any worker threads run (read-only thereafter);
 * not safe against concurrent compress calls using different weights. */
static uint32_t tc_bc7_cw[4] = {1u, 1u, 1u, 1u};

static uint32_t tc_err4(const uint8_t *a, uint8_t r, uint8_t g, uint8_t b,
                        uint8_t al, int has_alpha) {
    int dr = (int)a[0] - (int)r;
    int dg = (int)a[1] - (int)g;
    int db = (int)a[2] - (int)b;
    int da = has_alpha ? ((int)a[3] - (int)al) : 0;
    return tc_bc7_cw[0] * (uint32_t)(dr * dr) + tc_bc7_cw[1] * (uint32_t)(dg * dg) +
           tc_bc7_cw[2] * (uint32_t)(db * db) + tc_bc7_cw[3] * (uint32_t)(da * da);
}

static uint32_t tc_err3(const uint8_t *a, uint8_t r, uint8_t g, uint8_t b) {
    int dr = (int)a[0] - (int)r;
    int dg = (int)a[1] - (int)g;
    int db = (int)a[2] - (int)b;
    return tc_bc7_cw[0] * (uint32_t)(dr * dr) + tc_bc7_cw[1] * (uint32_t)(dg * dg) +
           tc_bc7_cw[2] * (uint32_t)(db * db);
}

static uint32_t tc_err1(uint8_t a, uint8_t b) {
    int d = (int)a - (int)b;
    return tc_bc7_cw[3] * (uint32_t)(d * d);
}

typedef struct tc_bc7_candidate {
    uint8_t mode;
    uint8_t partition;
    uint8_t index_selector;
    uint8_t rotation;
    uint8_t lo[3][4];
    uint8_t hi[3][4];
    uint8_t pbits[3][2];
    uint8_t selectors[16];
    uint8_t alpha_selectors[16];
} tc_bc7_candidate;

static const uint8_t *tc_partition_for(uint32_t mode, uint32_t partition) {
    if (tc_bc7_num_subsets[mode] == 1u) return tc_bc7_partition1;
    if (tc_bc7_num_subsets[mode] == 2u)
        return &tc_bc7_partition2[(partition & 63u) * 16u];
    return tc_bc7_partition3_0;
}

static uint8_t tc_anchor_for_subset(uint32_t mode, uint32_t partition,
                                    uint32_t subset) {
    if (subset == 0u) return 0;
    if (tc_bc7_num_subsets[mode] == 3u) return tc_bc7_anchor3_0[subset - 1u];
    return tc_bc7_anchor2[partition & 63u];
}

static const uint32_t *tc_weights_for_bits(uint32_t bits) {
    if (bits == 2u) return tc_bc7_weights2;
    if (bits == 3u) return tc_bc7_weights3;
    return tc_bc7_weights4;
}

static uint8_t tc_decode_endpoint(uint8_t q, uint8_t p, uint32_t precision,
                                  uint32_t has_pbit) {
    if (has_pbit) return tc_unquant(((uint32_t)q << 1u) | p, precision + 1u);
    return tc_unquant(q, precision);
}

static void tc_quant_endpoint(uint8_t v, uint32_t precision, uint32_t has_pbit,
                              uint32_t shared_pbit, uint8_t shared_p,
                              uint8_t *q, uint8_t *p) {
    if (has_pbit) {
        uint32_t total_bits = precision + 1u;
        uint32_t u = tc_quant(v, total_bits);
        if (shared_pbit) {
            *p = shared_p;
            *q = (uint8_t)(u >> 1u);
        } else {
            *p = (uint8_t)(u & 1u);
            *q = (uint8_t)(u >> 1u);
        }
    } else {
        *p = 0;
        *q = (uint8_t)tc_quant(v, precision);
    }
}

static void tc_recon_color(const tc_bc7_candidate *cand, uint32_t mode,
                           uint32_t subset, uint32_t sel, uint32_t asel,
                           uint8_t out[4]) {
    uint32_t c;
    uint32_t cbits = tc_bc7_color_index_bits[mode] + cand->index_selector;
    uint32_t abits = tc_bc7_alpha_index_bits[mode] - cand->index_selector;
    const uint32_t *cw = tc_weights_for_bits(cbits);
    const uint32_t *aw = tc_weights_for_bits(abits ? abits : cbits);
    uint32_t w = cw[sel];
    uint32_t awv = aw[asel];

    for (c = 0; c < 3u; ++c) {
        uint32_t a = tc_decode_endpoint(cand->lo[subset][c], cand->pbits[subset][0],
                                        tc_bc7_color_precision[mode],
                                        tc_bc7_has_pbits[mode]);
        uint32_t b = tc_decode_endpoint(cand->hi[subset][c], cand->pbits[subset][1],
                                        tc_bc7_color_precision[mode],
                                        tc_bc7_has_pbits[mode]);
        out[c] = (uint8_t)(((64u - w) * a + w * b + 32u) >> 6);
    }
    if (mode < 4u) {
        out[3] = 255u;
    } else {
        uint32_t ap = tc_bc7_has_pbits[mode] && !tc_bc7_sep_alpha[mode];
        uint32_t a = tc_decode_endpoint(cand->lo[subset][3], cand->pbits[subset][0],
                                        tc_bc7_alpha_precision[mode], ap);
        uint32_t b = tc_decode_endpoint(cand->hi[subset][3], cand->pbits[subset][1],
                                        tc_bc7_alpha_precision[mode], ap);
        if (!tc_bc7_sep_alpha[mode]) awv = w;
        out[3] = (uint8_t)(((64u - awv) * a + awv * b + 32u) >> 6);
    }
}

static void tc_fill_palette(const tc_bc7_candidate *cand, uint32_t mode,
                            uint8_t pal[3][16][4]) {
    uint32_t subset, sel, max_sel = 1u << (tc_bc7_color_index_bits[mode] + cand->index_selector);
    uint32_t max_asel = tc_bc7_alpha_index_bits[mode] > cand->index_selector
                            ? (1u << (tc_bc7_alpha_index_bits[mode] - cand->index_selector))
                            : max_sel;
    uint32_t n = max_sel > max_asel ? max_sel : max_asel;
    for (subset = 0; subset < tc_bc7_num_subsets[mode]; ++subset) {
        for (sel = 0; sel < n; ++sel) {
            uint32_t cs = sel < max_sel ? sel : max_sel - 1u;
            uint32_t as = sel < max_asel ? sel : max_asel - 1u;
            tc_recon_color(cand, mode, subset, cs, as, pal[subset][sel]);
        }
    }
}

static void tc_pack_candidate(const tc_bc7_candidate *src, uint8_t out[16]) {
    tc_bc7_candidate cand = *src;
    const uint8_t *part = tc_partition_for(cand.mode, cand.partition);
    uint32_t subsets = tc_bc7_num_subsets[cand.mode];
    uint32_t bitpos = 0, subset, comp, i;
    uint8_t anchor[3] = {0, 0, 0};

    for (subset = 0; subset < subsets; ++subset) {
        uint32_t anchor_index = tc_anchor_for_subset(cand.mode, cand.partition, subset);
        uint32_t cidx_bits = tc_bc7_color_index_bits[cand.mode] + cand.index_selector;
        uint32_t ncolor = 1u << cidx_bits;
        anchor[subset] = (uint8_t)anchor_index;
        if (cand.selectors[anchor_index] & (ncolor >> 1u)) {
            uint8_t tq[4], tp;
            for (i = 0; i < 16u; ++i)
                if (part[i] == subset) cand.selectors[i] = (uint8_t)((ncolor - 1u) - cand.selectors[i]);
            for (comp = 0; comp < (tc_bc7_sep_alpha[cand.mode] ? 3u : 4u); ++comp) {
                tq[comp] = cand.lo[subset][comp];
                cand.lo[subset][comp] = cand.hi[subset][comp];
                cand.hi[subset][comp] = tq[comp];
            }
            if (!tc_bc7_shared_pbits[cand.mode]) {
                tp = cand.pbits[subset][0];
                cand.pbits[subset][0] = cand.pbits[subset][1];
                cand.pbits[subset][1] = tp;
            }
        }
        if (tc_bc7_sep_alpha[cand.mode]) {
            uint32_t aidx_bits = tc_bc7_alpha_index_bits[cand.mode] - cand.index_selector;
            uint32_t nalpha = 1u << aidx_bits;
            if (cand.alpha_selectors[anchor_index] & (nalpha >> 1u)) {
                uint8_t tq = cand.lo[subset][3];
                for (i = 0; i < 16u; ++i)
                    if (part[i] == subset)
                        cand.alpha_selectors[i] = (uint8_t)((nalpha - 1u) - cand.alpha_selectors[i]);
                cand.lo[subset][3] = cand.hi[subset][3];
                cand.hi[subset][3] = tq;
            }
        }
    }

    memset(out, 0, 16);
    tc_set_bits(out, &bitpos, 1u << cand.mode, cand.mode + 1u);
    if (cand.mode == 4u || cand.mode == 5u) tc_set_bits(out, &bitpos, cand.rotation, 2);
    if (cand.mode == 4u) tc_set_bits(out, &bitpos, cand.index_selector, 1);
    if (tc_bc7_partition_bits[cand.mode])
        tc_set_bits(out, &bitpos, cand.partition, tc_bc7_partition_bits[cand.mode]);

    for (comp = 0; comp < (cand.mode >= 4u ? 4u : 3u); ++comp) {
        uint32_t prec = comp == 3u ? tc_bc7_alpha_precision[cand.mode]
                                   : tc_bc7_color_precision[cand.mode];
        for (subset = 0; subset < subsets; ++subset) {
            tc_set_bits(out, &bitpos, cand.lo[subset][comp], prec);
            tc_set_bits(out, &bitpos, cand.hi[subset][comp], prec);
        }
    }
    if (tc_bc7_has_pbits[cand.mode]) {
        for (subset = 0; subset < subsets; ++subset) {
            tc_set_bits(out, &bitpos, cand.pbits[subset][0], 1);
            if (!tc_bc7_shared_pbits[cand.mode]) tc_set_bits(out, &bitpos, cand.pbits[subset][1], 1);
        }
    }
    for (i = 0; i < 16u; ++i) {
        uint32_t n = cand.index_selector ? (tc_bc7_alpha_index_bits[cand.mode] - cand.index_selector)
                                         : (tc_bc7_color_index_bits[cand.mode] + cand.index_selector);
        if (i == anchor[0] || i == anchor[1] || i == anchor[2]) n--;
        tc_set_bits(out, &bitpos,
                    cand.index_selector ? cand.alpha_selectors[i] : cand.selectors[i], n);
    }
    if (tc_bc7_sep_alpha[cand.mode]) {
        for (i = 0; i < 16u; ++i) {
            uint32_t n = cand.index_selector ? (tc_bc7_color_index_bits[cand.mode] + cand.index_selector)
                                             : (tc_bc7_alpha_index_bits[cand.mode] - cand.index_selector);
            if (i == anchor[0] || i == anchor[1] || i == anchor[2]) n--;
            tc_set_bits(out, &bitpos,
                        cand.index_selector ? cand.selectors[i] : cand.alpha_selectors[i], n);
        }
    }
}

/* Weighted-PCA endpoint pixels: pick the two subset pixels at the extremes of
 * the principal color axis (covariance power iteration) instead of the luma
 * min/max. The luma axis is wrong when chroma varies at ~constant luminance;
 * the principal axis follows the actual color spread. Per-channel error weights
 * (tc_bc7_cw) scale the space so the axis favors the channels the error metric
 * weights. include_alpha folds A into the axis for the modes whose alpha shares
 * the color index (6, 7); modes with separate alpha fit it independently, so
 * their axis stays RGB. Falls back gracefully (degenerate blocks -> min==max). */
static void tc_bc7_pca_extremes(const uint8_t pix[16][4], const uint8_t *part,
                                uint32_t subset, int include_alpha,
                                uint32_t *out_min, uint32_t *out_max) {
    int nch = include_alpha ? 4 : 3;
    float w[4], mean[4] = {0.f, 0.f, 0.f, 0.f};
    float cov[4][4], axis[4];
    float mnp = 1e30f, mxp = -1e30f;
    uint32_t i, cnt = 0u, mn_i = 0u, mx_i = 0u;
    int c, d, it;

    for (c = 0; c < 4; ++c) w[c] = sqrtf((float)tc_bc7_cw[c]);
    for (i = 0; i < 16u; ++i)
        if (part[i] == subset) {
            for (c = 0; c < nch; ++c) mean[c] += (float)pix[i][c];
            ++cnt;
        }
    if (cnt == 0u) { *out_min = 0u; *out_max = 0u; return; }
    for (c = 0; c < nch; ++c) mean[c] /= (float)cnt;

    for (c = 0; c < 4; ++c)
        for (d = 0; d < 4; ++d) cov[c][d] = 0.f;
    for (i = 0; i < 16u; ++i)
        if (part[i] == subset) {
            float dv[4];
            for (c = 0; c < nch; ++c) dv[c] = w[c] * ((float)pix[i][c] - mean[c]);
            for (c = 0; c < nch; ++c)
                for (d = 0; d < nch; ++d) cov[c][d] += dv[c] * dv[d];
        }

    for (c = 0; c < nch; ++c) axis[c] = 1.f;
    for (it = 0; it < 8; ++it) {
        float na[4] = {0.f, 0.f, 0.f, 0.f}, len = 0.f;
        for (c = 0; c < nch; ++c)
            for (d = 0; d < nch; ++d) na[c] += cov[c][d] * axis[d];
        for (c = 0; c < nch; ++c) len += na[c] * na[c];
        if (len < 1e-12f) break;
        len = 1.f / sqrtf(len);
        for (c = 0; c < nch; ++c) axis[c] = na[c] * len;
    }

    for (i = 0; i < 16u; ++i)
        if (part[i] == subset) {
            float p = 0.f;
            for (c = 0; c < nch; ++c)
                p += axis[c] * w[c] * ((float)pix[i][c] - mean[c]);
            if (p <= mnp) { mnp = p; mn_i = i; }
            if (p >= mxp) { mxp = p; mx_i = i; }
        }
    *out_min = mn_i;
    *out_max = mx_i;
}

/* Endpoint-pixel seeding for a subset: luma extremes (fast, robust) or
 * weighted-PCA extremes (better on chromatic blocks whose spread is off the
 * luma axis). Neither dominates -- PCA can be distracted by variance in a
 * low-error-weight channel -- so tc_build_candidate tries both and keeps the
 * lower-error block (never regresses). */
static void tc_bc7_luma_extremes(const uint8_t pix[16][4], const uint8_t *part,
                                 uint32_t subset, uint32_t *out_min,
                                 uint32_t *out_max) {
    uint32_t min_l = UINT_MAX, max_l = 0, min_i = 0, max_i = 0, i;
    for (i = 0; i < 16u; ++i)
        if (part[i] == subset) {
            uint32_t y = tc_luma_u8(pix[i]);
            if (y < min_l) { min_l = y; min_i = i; }
            if (y >= max_l) { max_l = y; max_i = i; }
        }
    *out_min = min_i;
    *out_max = max_i;
}

static uint64_t tc_build_candidate_seed(uint32_t mode, uint32_t partition,
                                        const uint8_t pix[16][4],
                                        tc_bc7_candidate *cand, int use_pca) {
    const uint8_t *part = tc_partition_for(mode, partition);
    uint32_t subsets = tc_bc7_num_subsets[mode];
    uint32_t subset, i, c;
    uint64_t total_err = 0;
    memset(cand, 0, sizeof(*cand));
    cand->mode = (uint8_t)mode;
    cand->partition = (uint8_t)partition;
    cand->index_selector = 0;
    cand->rotation = 0;

    for (subset = 0; subset < subsets; ++subset) {
        uint32_t min_i, max_i;
        int inc_a = (mode >= 4u) && !tc_bc7_sep_alpha[mode];
        if (use_pca)
            tc_bc7_pca_extremes(pix, part, subset, inc_a, &min_i, &max_i);
        else
            tc_bc7_luma_extremes(pix, part, subset, &min_i, &max_i);
        for (c = 0; c < 4u; ++c) {
            uint8_t qp0, qp1;
            uint32_t prec = c == 3u && mode >= 4u ? tc_bc7_alpha_precision[mode]
                                                   : tc_bc7_color_precision[mode];
            uint32_t has_p = tc_bc7_has_pbits[mode] && (c < 3u || !tc_bc7_sep_alpha[mode]);
            uint8_t shared = (uint8_t)(((pix[min_i][c] + pix[max_i][c]) >> 1u) & 1u);
            if (mode < 4u && c == 3u) {
                cand->lo[subset][c] = 0;
                cand->hi[subset][c] = 0;
                continue;
            }
            tc_quant_endpoint(pix[min_i][c], prec, has_p, tc_bc7_shared_pbits[mode],
                              shared, &cand->lo[subset][c], &qp0);
            tc_quant_endpoint(pix[max_i][c], prec, has_p, tc_bc7_shared_pbits[mode],
                              shared, &cand->hi[subset][c], &qp1);
            if (tc_bc7_shared_pbits[mode]) {
                cand->pbits[subset][0] = shared;
                cand->pbits[subset][1] = shared;
            } else if (has_p) {
                cand->pbits[subset][0] = qp0;
                cand->pbits[subset][1] = qp1;
            }
        }
    }

    {
        uint8_t pal[3][16][4];
        uint32_t cbits = tc_bc7_color_index_bits[mode] + cand->index_selector;
        uint32_t abits = tc_bc7_alpha_index_bits[mode] - cand->index_selector;
        uint32_t nc = 1u << cbits;
        uint32_t na = abits ? (1u << abits) : nc;
        tc_fill_palette(cand, mode, pal);
        for (i = 0; i < 16u; ++i) {
            uint32_t subset = part[i], s, best_s = 0, best_a = 0;
            uint32_t best = UINT_MAX;
            uint32_t best_color_err = 0;
            uint32_t best_alpha_err = 0;
            for (s = 0; s < nc; ++s) {
                const uint8_t *r = pal[subset][s];
                uint32_t e;
                if (tc_bc7_sep_alpha[mode] || mode < 4u)
                    e = tc_err3(pix[i], r[0], r[1], r[2]);
                else
                    e = tc_err4(pix[i], r[0], r[1], r[2], r[3], 1);
                if (e < best) {
                    best = e;
                    best_s = s;
                }
            }
            best_color_err = best;
            if (tc_bc7_sep_alpha[mode]) {
                best = UINT_MAX;
                for (s = 0; s < na; ++s) {
                    const uint8_t *r = pal[subset][s];
                    uint32_t e = tc_err1(pix[i][3], r[3]);
                    if (e < best) {
                        best = e;
                        best_a = s;
                    }
                }
                best_alpha_err = best;
            } else {
                best_a = best_s;
                best_alpha_err = 0;
            }
            cand->selectors[i] = (uint8_t)best_s;
            cand->alpha_selectors[i] = (uint8_t)best_a;
            total_err += (uint64_t)best_color_err + (uint64_t)best_alpha_err;
        }
    }
    return total_err;
}

/* Build a candidate for (mode, partition). The luma seed is always evaluated.
 * When try_pca is set (opt->pca_endpoints), also try the weighted-PCA seed and
 * keep the lower-error block -- monotonic, never worse than luma alone. PCA
 * roughly doubles the per-candidate cost, so it is opt-in (off by default);
 * measured ~+0.09 dB on photos, more on chromatic (off-luma-axis) blocks. */
static uint64_t tc_build_candidate(uint32_t mode, uint32_t partition,
                                   const uint8_t pix[16][4],
                                   tc_bc7_candidate *cand, int try_pca) {
    uint64_t e0 = tc_build_candidate_seed(mode, partition, pix, cand, 0);
    tc_bc7_candidate alt;
    uint64_t e1;
    if (!try_pca) return e0;
    e1 = tc_build_candidate_seed(mode, partition, pix, &alt, 1);
    if (e1 < e0) {
        *cand = alt;
        return e1;
    }
    return e0;
}

static uint8_t tc_lookup_index_from_mask(uint32_t mask) {
    switch (mask) {
        case 11u: return 0;
        case 12u: return 1;
        case 18u: return 2;
        case 21u: return 3;
        case 33u: return 4;
        case 38u: return 5;
        case 56u: return 6;
        case 63u: return 7;
        default: return 0;
    }
}

static uint8_t tc_dominant_rgb_channel(const uint8_t *p) {
    if (p[0] > p[1]) return p[0] > p[2] ? 0u : 2u;
    return p[1] > p[2] ? 1u : 2u;
}

static uint8_t tc_rgb_partition_lut_index(const uint8_t pix[16][4]) {
    const uint8_t ids[4] = {0, 1, 4, 5};
    uint8_t ch[4];
    uint32_t i, j, bit = 0, mask = 0;
    for (i = 0; i < 4u; ++i) ch[i] = tc_dominant_rgb_channel(pix[ids[i]]);
    for (i = 0; i < 4u; ++i) {
        for (j = i + 1u; j < 4u; ++j) {
            if (ch[i] == ch[j]) mask |= 1u << bit;
            ++bit;
        }
    }
    return tc_lookup_index_from_mask(mask);
}

static uint8_t tc_alpha_partition_lut_index(const uint8_t pix[16][4]) {
    const uint8_t ids[4] = {0, 3, 12, 15};
    uint32_t i, j, bit = 0, mask = 0;
    uint8_t cls[4], amin = 255, amax = 0, median;
    for (i = 0; i < 16u; ++i) {
        if (pix[i][3] < amin) amin = pix[i][3];
        if (pix[i][3] > amax) amax = pix[i][3];
    }
    median = (uint8_t)(((uint32_t)amin + (uint32_t)amax) >> 1u);
    for (i = 0; i < 4u; ++i) cls[i] = pix[ids[i]][3] > median ? 1u : 0u;
    for (i = 0; i < 4u; ++i) {
        for (j = i + 1u; j < 4u; ++j) {
            if (cls[i] == cls[j]) mask |= 1u << bit;
            ++bit;
        }
    }
    return tc_lookup_index_from_mask(mask);
}

static uint64_t tc_partition_cluster_score(const uint8_t pix[16][4],
                                           uint32_t partition,
                                           uint32_t include_alpha) {
    const uint8_t *part = &tc_bc7_partition2[(partition & 63u) * 16u];
    uint32_t sums[2][4] = {{0, 0, 0, 0}, {0, 0, 0, 0}};
    uint32_t total[4] = {0, 0, 0, 0};
    uint32_t counts[2] = {0, 0};
    uint32_t i, c, comps = include_alpha ? 4u : 3u;
    uint64_t score = 0;
    for (i = 0; i < 16u; ++i) {
        uint32_t s = part[i];
        ++counts[s];
        for (c = 0; c < comps; ++c) {
            sums[s][c] += pix[i][c];
            total[c] += pix[i][c];
        }
    }
    for (i = 0; i < 2u; ++i) {
        if (!counts[i]) continue;
        for (c = 0; c < comps; ++c) {
            int32_t d = (int32_t)(16u * sums[i][c]) -
                        (int32_t)(counts[i] * total[c]);
            score += ((uint64_t)(uint32_t)(d < 0 ? -d : d) *
                      (uint64_t)(uint32_t)(d < 0 ? -d : d)) /
                     counts[i];
        }
    }
    return score;
}

static uint32_t tc_select_partition2(const uint8_t pix[16][4], uint32_t mode,
                                     uint32_t quick) {
    uint32_t best_partition = 0, i, count;
    uint64_t best_score = 0;
    if (!quick) {
        count = 64u;
        for (i = 0; i < count; ++i) {
            uint64_t score = tc_partition_cluster_score(pix, i, mode == 7u);
            if (score > best_score || i == 0u) {
                best_score = score;
                best_partition = i;
            }
        }
    } else if (mode == 7u) {
        uint8_t lut = tc_alpha_partition_lut_index(pix);
        (void)best_score;
        best_partition = tc_bc7_alpha_part_lut[lut][0];
    } else {
        uint8_t lut = tc_rgb_partition_lut_index(pix);
        (void)best_score;
        best_partition = tc_bc7_part_lut[lut][0];
    }
    return best_partition;
}

static void tc_encode_bc7_all_modes_block(const uint8_t pix[16][4],
                                          const tc_bc7_options *opt,
                                          uint8_t out[16]) {
    uint32_t mode;
    uint64_t best_err = UINT64_MAX;
    uint8_t best_block[16];
    uint32_t mask = opt && opt->mode_mask ? opt->mode_mask : 0xffu;
    uint32_t is_quick = opt ? (uint32_t)opt->quick : 0u;
    if (is_quick == 1u) {
        mask = tc_block_quick_mask(pix, mask);
    } else {
        /* For non-quick modes, still exclude modes that can't encode alpha
         * when the block has any translucent texel. Modes 0-3 are opaque-only;
         * mode 4+ handle alpha. Without this filter the encoder would pick a
         * mode that discards alpha, producing corrupt output. */
        uint32_t i, has_alpha = 0;
        for (i = 0; i < 16u; ++i)
            if (pix[i][3] < 255u) { has_alpha = 1; break; }
        if (has_alpha) mask &= ~0x0fu; /* drop modes 0-3 */
    }
    memset(best_block, 0, sizeof(best_block));
    for (mode = 0; mode < 8u; ++mode) {
        tc_bc7_candidate cand;
        uint64_t err;
        if ((mask & (1u << mode)) == 0u) continue;
        err = tc_build_candidate(mode,
                                 (mode == 1u || mode == 7u)
                                     ? tc_select_partition2(pix, mode,
                                                            is_quick != 0u)
                                     : 0u,
                                 pix, &cand, opt && opt->pca_endpoints);
        if (err < best_err) {
            best_err = err;
            tc_pack_candidate(&cand, best_block);
        }
    }
    memcpy(out, best_block, 16);
}

/* --- BC7 decode (used by decoder-driven RDO; also the public decompressor) ---
 * BPTC decode: all 8 modes, partitions, rotations, dual-plane, p-bits. The
 * 2/3-subset partition+anchor tables are the corrected BPTC tables (the
 * Khronos-published ones have known errors); an independent copy lives in
 * tools/texcomp/test/bc7_ref_decode.h and the two are cross-checked in the
 * xbc7 gate. Partition values carry the subset in the low 2 bits and mark each
 * subset's anchor texel with bit 0x80. */
static uint32_t tc_bc7_dec_rb(uint64_t *lo, uint64_t *hi, int n) {
    uint32_t mask = (n >= 32) ? 0xffffffffu : ((1u << n) - 1u);
    uint32_t bits = (uint32_t)(*lo & mask);
    *lo >>= n;
    *lo |= (*hi & mask) << (64 - n);
    *hi >>= n;
    return bits;
}
static int tc_bc7_dec_interp(int a, int b, const int *w, int idx) {
    return (a * (64 - w[idx]) + b * w[idx] + 32) >> 6;
}
static void tc_bc7_decode_block(const uint8_t blk[16], uint8_t out[16][4]) {
    static const unsigned char tc_bc7_dec_parts[2][64][4][4] = {
        {   /* Partition table for 2-subset BPTC */
            { {128, 0,   1, 1}, {0, 0,   1, 1}, {  0, 0, 1, 1}, {0, 0, 1, 129} }, /*  0 */
            { {128, 0,   0, 1}, {0, 0,   0, 1}, {  0, 0, 0, 1}, {0, 0, 0, 129} }, /*  1 */
            { {128, 1,   1, 1}, {0, 1,   1, 1}, {  0, 1, 1, 1}, {0, 1, 1, 129} }, /*  2 */
            { {128, 0,   0, 1}, {0, 0,   1, 1}, {  0, 0, 1, 1}, {0, 1, 1, 129} }, /*  3 */
            { {128, 0,   0, 0}, {0, 0,   0, 1}, {  0, 0, 0, 1}, {0, 0, 1, 129} }, /*  4 */
            { {128, 0,   1, 1}, {0, 1,   1, 1}, {  0, 1, 1, 1}, {1, 1, 1, 129} }, /*  5 */
            { {128, 0,   0, 1}, {0, 0,   1, 1}, {  0, 1, 1, 1}, {1, 1, 1, 129} }, /*  6 */
            { {128, 0,   0, 0}, {0, 0,   0, 1}, {  0, 0, 1, 1}, {0, 1, 1, 129} }, /*  7 */
            { {128, 0,   0, 0}, {0, 0,   0, 0}, {  0, 0, 0, 1}, {0, 0, 1, 129} }, /*  8 */
            { {128, 0,   1, 1}, {0, 1,   1, 1}, {  1, 1, 1, 1}, {1, 1, 1, 129} }, /*  9 */
            { {128, 0,   0, 0}, {0, 0,   0, 1}, {  0, 1, 1, 1}, {1, 1, 1, 129} }, /* 10 */
            { {128, 0,   0, 0}, {0, 0,   0, 0}, {  0, 0, 0, 1}, {0, 1, 1, 129} }, /* 11 */
            { {128, 0,   0, 1}, {0, 1,   1, 1}, {  1, 1, 1, 1}, {1, 1, 1, 129} }, /* 12 */
            { {128, 0,   0, 0}, {0, 0,   0, 0}, {  1, 1, 1, 1}, {1, 1, 1, 129} }, /* 13 */
            { {128, 0,   0, 0}, {1, 1,   1, 1}, {  1, 1, 1, 1}, {1, 1, 1, 129} }, /* 14 */
            { {128, 0,   0, 0}, {0, 0,   0, 0}, {  0, 0, 0, 0}, {1, 1, 1, 129} }, /* 15 */
            { {128, 0,   0, 0}, {1, 0,   0, 0}, {  1, 1, 1, 0}, {1, 1, 1, 129} }, /* 16 */
            { {128, 1, 129, 1}, {0, 0,   0, 1}, {  0, 0, 0, 0}, {0, 0, 0,   0} }, /* 17 */
            { {128, 0,   0, 0}, {0, 0,   0, 0}, {129, 0, 0, 0}, {1, 1, 1,   0} }, /* 18 */
            { {128, 1, 129, 1}, {0, 0,   1, 1}, {  0, 0, 0, 1}, {0, 0, 0,   0} }, /* 19 */
            { {128, 0, 129, 1}, {0, 0,   0, 1}, {  0, 0, 0, 0}, {0, 0, 0,   0} }, /* 20 */
            { {128, 0,   0, 0}, {1, 0,   0, 0}, {129, 1, 0, 0}, {1, 1, 1,   0} }, /* 21 */
            { {128, 0,   0, 0}, {0, 0,   0, 0}, {129, 0, 0, 0}, {1, 1, 0,   0} }, /* 22 */
            { {128, 1,   1, 1}, {0, 0,   1, 1}, {  0, 0, 1, 1}, {0, 0, 0, 129} }, /* 23 */
            { {128, 0, 129, 1}, {0, 0,   0, 1}, {  0, 0, 0, 1}, {0, 0, 0,   0} }, /* 24 */
            { {128, 0,   0, 0}, {1, 0,   0, 0}, {129, 0, 0, 0}, {1, 1, 0,   0} }, /* 25 */
            { {128, 1, 129, 0}, {0, 1,   1, 0}, {  0, 1, 1, 0}, {0, 1, 1,   0} }, /* 26 */
            { {128, 0, 129, 1}, {0, 1,   1, 0}, {  0, 1, 1, 0}, {1, 1, 0,   0} }, /* 27 */
            { {128, 0,   0, 1}, {0, 1,   1, 1}, {129, 1, 1, 0}, {1, 0, 0,   0} }, /* 28 */
            { {128, 0,   0, 0}, {1, 1,   1, 1}, {129, 1, 1, 1}, {0, 0, 0,   0} }, /* 29 */
            { {128, 1, 129, 1}, {0, 0,   0, 1}, {  1, 0, 0, 0}, {1, 1, 1,   0} }, /* 30 */
            { {128, 0, 129, 1}, {1, 0,   0, 1}, {  1, 0, 0, 1}, {1, 1, 0,   0} }, /* 31 */
            { {128, 1,   0, 1}, {0, 1,   0, 1}, {  0, 1, 0, 1}, {0, 1, 0, 129} }, /* 32 */
            { {128, 0,   0, 0}, {1, 1,   1, 1}, {  0, 0, 0, 0}, {1, 1, 1, 129} }, /* 33 */
            { {128, 1,   0, 1}, {1, 0, 129, 0}, {  0, 1, 0, 1}, {1, 0, 1,   0} }, /* 34 */
            { {128, 0,   1, 1}, {0, 0,   1, 1}, {129, 1, 0, 0}, {1, 1, 0,   0} }, /* 35 */
            { {128, 0, 129, 1}, {1, 1,   0, 0}, {  0, 0, 1, 1}, {1, 1, 0,   0} }, /* 36 */
            { {128, 1,   0, 1}, {0, 1,   0, 1}, {129, 0, 1, 0}, {1, 0, 1,   0} }, /* 37 */
            { {128, 1,   1, 0}, {1, 0,   0, 1}, {  0, 1, 1, 0}, {1, 0, 0, 129} }, /* 38 */
            { {128, 1,   0, 1}, {1, 0,   1, 0}, {  1, 0, 1, 0}, {0, 1, 0, 129} }, /* 39 */
            { {128, 1, 129, 1}, {0, 0,   1, 1}, {  1, 1, 0, 0}, {1, 1, 1,   0} }, /* 40 */
            { {128, 0,   0, 1}, {0, 0,   1, 1}, {129, 1, 0, 0}, {1, 0, 0,   0} }, /* 41 */
            { {128, 0, 129, 1}, {0, 0,   1, 0}, {  0, 1, 0, 0}, {1, 1, 0,   0} }, /* 42 */
            { {128, 0, 129, 1}, {1, 0,   1, 1}, {  1, 1, 0, 1}, {1, 1, 0,   0} }, /* 43 */
            { {128, 1, 129, 0}, {1, 0,   0, 1}, {  1, 0, 0, 1}, {0, 1, 1,   0} }, /* 44 */
            { {128, 0,   1, 1}, {1, 1,   0, 0}, {  1, 1, 0, 0}, {0, 0, 1, 129} }, /* 45 */
            { {128, 1,   1, 0}, {0, 1,   1, 0}, {  1, 0, 0, 1}, {1, 0, 0, 129} }, /* 46 */
            { {128, 0,   0, 0}, {0, 1, 129, 0}, {  0, 1, 1, 0}, {0, 0, 0,   0} }, /* 47 */
            { {128, 1,   0, 0}, {1, 1, 129, 0}, {  0, 1, 0, 0}, {0, 0, 0,   0} }, /* 48 */
            { {128, 0, 129, 0}, {0, 1,   1, 1}, {  0, 0, 1, 0}, {0, 0, 0,   0} }, /* 49 */
            { {128, 0,   0, 0}, {0, 0, 129, 0}, {  0, 1, 1, 1}, {0, 0, 1,   0} }, /* 50 */
            { {128, 0,   0, 0}, {0, 1,   0, 0}, {129, 1, 1, 0}, {0, 1, 0,   0} }, /* 51 */
            { {128, 1,   1, 0}, {1, 1,   0, 0}, {  1, 0, 0, 1}, {0, 0, 1, 129} }, /* 52 */
            { {128, 0,   1, 1}, {0, 1,   1, 0}, {  1, 1, 0, 0}, {1, 0, 0, 129} }, /* 53 */
            { {128, 1, 129, 0}, {0, 0,   1, 1}, {  1, 0, 0, 1}, {1, 1, 0,   0} }, /* 54 */
            { {128, 0, 129, 1}, {1, 0,   0, 1}, {  1, 1, 0, 0}, {0, 1, 1,   0} }, /* 55 */
            { {128, 1,   1, 0}, {1, 1,   0, 0}, {  1, 1, 0, 0}, {1, 0, 0, 129} }, /* 56 */
            { {128, 1,   1, 0}, {0, 0,   1, 1}, {  0, 0, 1, 1}, {1, 0, 0, 129} }, /* 57 */
            { {128, 1,   1, 1}, {1, 1,   1, 0}, {  1, 0, 0, 0}, {0, 0, 0, 129} }, /* 58 */
            { {128, 0,   0, 1}, {1, 0,   0, 0}, {  1, 1, 1, 0}, {0, 1, 1, 129} }, /* 59 */
            { {128, 0,   0, 0}, {1, 1,   1, 1}, {  0, 0, 1, 1}, {0, 0, 1, 129} }, /* 60 */
            { {128, 0, 129, 1}, {0, 0,   1, 1}, {  1, 1, 1, 1}, {0, 0, 0,   0} }, /* 61 */
            { {128, 0, 129, 0}, {0, 0,   1, 0}, {  1, 1, 1, 0}, {1, 1, 1,   0} }, /* 62 */
            { {128, 1,   0, 0}, {0, 1,   0, 0}, {  0, 1, 1, 1}, {0, 1, 1, 129} }  /* 63 */
        },
        {   /* Partition table for 3-subset BPTC */
            { {128, 0, 1, 129}, {0,   0,   1, 1}, {  0,   2,   2, 1}, {  2,   2, 2, 130} }, /*  0 */
            { {128, 0, 0, 129}, {0,   0,   1, 1}, {130,   2,   1, 1}, {  2,   2, 2,   1} }, /*  1 */
            { {128, 0, 0,   0}, {2,   0,   0, 1}, {130,   2,   1, 1}, {  2,   2, 1, 129} }, /*  2 */
            { {128, 2, 2, 130}, {0,   0,   2, 2}, {  0,   0,   1, 1}, {  0,   1, 1, 129} }, /*  3 */
            { {128, 0, 0,   0}, {0,   0,   0, 0}, {129,   1,   2, 2}, {  1,   1, 2, 130} }, /*  4 */
            { {128, 0, 1, 129}, {0,   0,   1, 1}, {  0,   0,   2, 2}, {  0,   0, 2, 130} }, /*  5 */
            { {128, 0, 2, 130}, {0,   0,   2, 2}, {  1,   1,   1, 1}, {  1,   1, 1, 129} }, /*  6 */
            { {128, 0, 1,   1}, {0,   0,   1, 1}, {130,   2,   1, 1}, {  2,   2, 1, 129} }, /*  7 */
            { {128, 0, 0,   0}, {0,   0,   0, 0}, {129,   1,   1, 1}, {  2,   2, 2, 130} }, /*  8 */
            { {128, 0, 0,   0}, {1,   1,   1, 1}, {129,   1,   1, 1}, {  2,   2, 2, 130} }, /*  9 */
            { {128, 0, 0,   0}, {1,   1, 129, 1}, {  2,   2,   2, 2}, {  2,   2, 2, 130} }, /* 10 */
            { {128, 0, 1,   2}, {0,   0, 129, 2}, {  0,   0,   1, 2}, {  0,   0, 1, 130} }, /* 11 */
            { {128, 1, 1,   2}, {0,   1, 129, 2}, {  0,   1,   1, 2}, {  0,   1, 1, 130} }, /* 12 */
            { {128, 1, 2,   2}, {0, 129,   2, 2}, {  0,   1,   2, 2}, {  0,   1, 2, 130} }, /* 13 */
            { {128, 0, 1, 129}, {0,   1,   1, 2}, {  1,   1,   2, 2}, {  1,   2, 2, 130} }, /* 14 */
            { {128, 0, 1, 129}, {2,   0,   0, 1}, {130,   2,   0, 0}, {  2,   2, 2,   0} }, /* 15 */
            { {128, 0, 0, 129}, {0,   0,   1, 1}, {  0,   1,   1, 2}, {  1,   1, 2, 130} }, /* 16 */
            { {128, 1, 1, 129}, {0,   0,   1, 1}, {130,   0,   0, 1}, {  2,   2, 0,   0} }, /* 17 */
            { {128, 0, 0,   0}, {1,   1,   2, 2}, {129,   1,   2, 2}, {  1,   1, 2, 130} }, /* 18 */
            { {128, 0, 2, 130}, {0,   0,   2, 2}, {  0,   0,   2, 2}, {  1,   1, 1, 129} }, /* 19 */
            { {128, 1, 1, 129}, {0,   1,   1, 1}, {  0,   2,   2, 2}, {  0,   2, 2, 130} }, /* 20 */
            { {128, 0, 0, 129}, {0,   0,   0, 1}, {130,   2,   2, 1}, {  2,   2, 2,   1} }, /* 21 */
            { {128, 0, 0,   0}, {0,   0, 129, 1}, {  0,   1,   2, 2}, {  0,   1, 2, 130} }, /* 22 */
            { {128, 0, 0,   0}, {1,   1,   0, 0}, {130,   2, 129, 0}, {  2,   2, 1,   0} }, /* 23 */
            { {128, 1, 2, 130}, {0, 129,   2, 2}, {  0,   0,   1, 1}, {  0,   0, 0,   0} }, /* 24 */
            { {128, 0, 1,   2}, {0,   0,   1, 2}, {129,   1,   2, 2}, {  2,   2, 2, 130} }, /* 25 */
            { {128, 1, 1,   0}, {1,   2, 130, 1}, {129,   2,   2, 1}, {  0,   1, 1,   0} }, /* 26 */
            { {128, 0, 0,   0}, {0,   1, 129, 0}, {  1,   2, 130, 1}, {  1,   2, 2,   1} }, /* 27 */
            { {128, 0, 2,   2}, {1,   1,   0, 2}, {129,   1,   0, 2}, {  0,   0, 2, 130} }, /* 28 */
            { {128, 1, 1,   0}, {0, 129,   1, 0}, {  2,   0,   0, 2}, {  2,   2, 2, 130} }, /* 29 */
            { {128, 0, 1,   1}, {0,   1,   2, 2}, {  0,   1, 130, 2}, {  0,   0, 1, 129} }, /* 30 */
            { {128, 0, 0,   0}, {2,   0,   0, 0}, {130,   2,   1, 1}, {  2,   2, 2, 129} }, /* 31 */
            { {128, 0, 0,   0}, {0,   0,   0, 2}, {129,   1,   2, 2}, {  1,   2, 2, 130} }, /* 32 */
            { {128, 2, 2, 130}, {0,   0,   2, 2}, {  0,   0,   1, 2}, {  0,   0, 1, 129} }, /* 33 */
            { {128, 0, 1, 129}, {0,   0,   1, 2}, {  0,   0,   2, 2}, {  0,   2, 2, 130} }, /* 34 */
            { {128, 1, 2,   0}, {0, 129,   2, 0}, {  0,   1, 130, 0}, {  0,   1, 2,   0} }, /* 35 */
            { {128, 0, 0,   0}, {1,   1, 129, 1}, {  2,   2, 130, 2}, {  0,   0, 0,   0} }, /* 36 */
            { {128, 1, 2,   0}, {1,   2,   0, 1}, {130,   0, 129, 2}, {  0,   1, 2,   0} }, /* 37 */
            { {128, 1, 2,   0}, {2,   0,   1, 2}, {129, 130,   0, 1}, {  0,   1, 2,   0} }, /* 38 */
            { {128, 0, 1,   1}, {2,   2,   0, 0}, {  1,   1, 130, 2}, {  0,   0, 1, 129} }, /* 39 */
            { {128, 0, 1,   1}, {1,   1, 130, 2}, {  2,   2,   0, 0}, {  0,   0, 1, 129} }, /* 40 */
            { {128, 1, 0, 129}, {0,   1,   0, 1}, {  2,   2,   2, 2}, {  2,   2, 2, 130} }, /* 41 */
            { {128, 0, 0,   0}, {0,   0,   0, 0}, {130,   1,   2, 1}, {  2,   1, 2, 129} }, /* 42 */
            { {128, 0, 2,   2}, {1, 129,   2, 2}, {  0,   0,   2, 2}, {  1,   1, 2, 130} }, /* 43 */
            { {128, 0, 2, 130}, {0,   0,   1, 1}, {  0,   0,   2, 2}, {  0,   0, 1, 129} }, /* 44 */
            { {128, 2, 2,   0}, {1,   2, 130, 1}, {  0,   2,   2, 0}, {  1,   2, 2, 129} }, /* 45 */
            { {128, 1, 0,   1}, {2,   2, 130, 2}, {  2,   2,   2, 2}, {  0,   1, 0, 129} }, /* 46 */
            { {128, 0, 0,   0}, {2,   1,   2, 1}, {130,   1,   2, 1}, {  2,   1, 2, 129} }, /* 47 */
            { {128, 1, 0, 129}, {0,   1,   0, 1}, {  0,   1,   0, 1}, {  2,   2, 2, 130} }, /* 48 */
            { {128, 2, 2, 130}, {0,   1,   1, 1}, {  0,   2,   2, 2}, {  0,   1, 1, 129} }, /* 49 */
            { {128, 0, 0,   2}, {1, 129,   1, 2}, {  0,   0,   0, 2}, {  1,   1, 1, 130} }, /* 50 */
            { {128, 0, 0,   0}, {2, 129,   1, 2}, {  2,   1,   1, 2}, {  2,   1, 1, 130} }, /* 51 */
            { {128, 2, 2,   2}, {0, 129,   1, 1}, {  0,   1,   1, 1}, {  0,   2, 2, 130} }, /* 52 */
            { {128, 0, 0,   2}, {1,   1,   1, 2}, {129,   1,   1, 2}, {  0,   0, 0, 130} }, /* 53 */
            { {128, 1, 1,   0}, {0, 129,   1, 0}, {  0,   1,   1, 0}, {  2,   2, 2, 130} }, /* 54 */
            { {128, 0, 0,   0}, {0,   0,   0, 0}, {  2,   1, 129, 2}, {  2,   1, 1, 130} }, /* 55 */
            { {128, 1, 1,   0}, {0, 129,   1, 0}, {  2,   2,   2, 2}, {  2,   2, 2, 130} }, /* 56 */
            { {128, 0, 2,   2}, {0,   0,   1, 1}, {  0,   0, 129, 1}, {  0,   0, 2, 130} }, /* 57 */
            { {128, 0, 2,   2}, {1,   1,   2, 2}, {129,   1,   2, 2}, {  0,   0, 2, 130} }, /* 58 */
            { {128, 0, 0,   0}, {0,   0,   0, 0}, {  0,   0,   0, 0}, {  2, 129, 1, 130} }, /* 59 */
            { {128, 0, 0, 130}, {0,   0,   0, 1}, {  0,   0,   0, 2}, {  0,   0, 0, 129} }, /* 60 */
            { {128, 2, 2,   2}, {1,   2,   2, 2}, {  0,   2,   2, 2}, {129,   2, 2, 130} }, /* 61 */
            { {128, 1, 0, 129}, {2,   2,   2, 2}, {  2,   2,   2, 2}, {  2,   2, 2, 130} }, /* 62 */
            { {128, 1, 1, 129}, {2,   0,   1, 1}, {130,   2,   0, 1}, {  2,   2, 2,   0} }  /* 63 */
        }
    };
    static const int aWeight2[4] = {0, 21, 43, 64};
    static const int aWeight3[8] = {0, 9, 18, 27, 37, 46, 55, 64};
    static const int aWeight4[16] = {0,  4,  9,  13, 17, 21, 26, 30,
                                     34, 38, 43, 47, 51, 55, 60, 64};
    static const signed char actual_bits[2][8] = {
        {4, 6, 5, 7, 5, 7, 7, 5}, {0, 0, 0, 0, 6, 8, 7, 5}};
    static const unsigned char mode_has_pbits = 0xCB;
    uint64_t lo, hi;
    int mode, partition = 0, numPart = 1, numEp, rotation = 0, idxSelBit = 0;
    int ib, ib2, i, j, k;
    int ep[6][4];
    signed char cidx[4][4];
    const int *w, *w2;
    memcpy(&lo, blk, 8);
    memcpy(&hi, blk + 8, 8);
    for (mode = 0; mode < 8 && tc_bc7_dec_rb(&lo, &hi, 1) == 0; ++mode)
        ;
    if (mode >= 8) {
        memset(out, 0, 16u * 4u);
        return;
    }
    if (mode == 0 || mode == 1 || mode == 2 || mode == 3 || mode == 7) {
        numPart = (mode == 0 || mode == 2) ? 3 : 2;
        partition = (int)tc_bc7_dec_rb(&lo, &hi, (mode == 0) ? 4 : 6);
    }
    numEp = numPart * 2;
    if (mode == 4 || mode == 5) {
        rotation = (int)tc_bc7_dec_rb(&lo, &hi, 2);
        if (mode == 4) idxSelBit = (int)tc_bc7_dec_rb(&lo, &hi, 1);
    }
    for (i = 0; i < 3; ++i)
        for (j = 0; j < numEp; ++j)
            ep[j][i] = (int)tc_bc7_dec_rb(&lo, &hi, actual_bits[0][mode]);
    if (actual_bits[1][mode] > 0)
        for (j = 0; j < numEp; ++j)
            ep[j][3] = (int)tc_bc7_dec_rb(&lo, &hi, actual_bits[1][mode]);
    if (mode == 0 || mode == 1 || mode == 3 || mode == 6 || mode == 7) {
        for (i = 0; i < numEp; ++i)
            for (j = 0; j < 4; ++j) ep[i][j] <<= 1;
        if (mode == 1) {
            int p0 = (int)tc_bc7_dec_rb(&lo, &hi, 1);
            int p1 = (int)tc_bc7_dec_rb(&lo, &hi, 1);
            for (k = 0; k < 3; ++k) {
                ep[0][k] |= p0; ep[1][k] |= p0; ep[2][k] |= p1; ep[3][k] |= p1;
            }
        } else {
            for (i = 0; i < numEp; ++i) {
                int p = (int)tc_bc7_dec_rb(&lo, &hi, 1);
                for (k = 0; k < 4; ++k) ep[i][k] |= p;
            }
        }
    }
    for (i = 0; i < numEp; ++i) {
        int cb = actual_bits[0][mode] + ((mode_has_pbits >> mode) & 1);
        int ab = actual_bits[1][mode] + ((mode_has_pbits >> mode) & 1);
        for (k = 0; k < 3; ++k) {
            ep[i][k] = ep[i][k] << (8 - cb);
            ep[i][k] |= ep[i][k] >> cb;
        }
        ep[i][3] = ep[i][3] << (8 - ab);
        ep[i][3] |= ep[i][3] >> ab;
    }
    if (!actual_bits[1][mode])
        for (j = 0; j < numEp; ++j) ep[j][3] = 0xFF;
    ib = (mode == 0 || mode == 1) ? 3 : ((mode == 6) ? 4 : 2);
    ib2 = (mode == 4) ? 3 : ((mode == 5) ? 2 : 0);
    w = (ib == 2) ? aWeight2 : ((ib == 3) ? aWeight3 : aWeight4);
    w2 = (ib2 == 2) ? aWeight2 : aWeight3;
    for (i = 0; i < 4; ++i)
        for (j = 0; j < 4; ++j) {
            int ps = (numPart == 1) ? ((i | j) ? 0 : 128)
                                    : tc_bc7_dec_parts[numPart - 2][partition][i][j];
            int bits = ib - ((ps & 0x80) ? 1 : 0);
            cidx[i][j] = (signed char)tc_bc7_dec_rb(&lo, &hi, bits);
        }
    for (i = 0; i < 4; ++i)
        for (j = 0; j < 4; ++j) {
            int ps = ((numPart == 1) ? ((i | j) ? 0 : 128)
                                     : tc_bc7_dec_parts[numPart - 2][partition][i][j]) & 0x03;
            int idx = cidx[i][j], idx2 = 0;
            int r, g, b, a, s0 = ps * 2, s1 = ps * 2 + 1;
            if (ib2) idx2 = (int)tc_bc7_dec_rb(&lo, &hi, (i | j) ? ib2 : (ib2 - 1));
            if (!ib2 || !idxSelBit) {
                r = tc_bc7_dec_interp(ep[s0][0], ep[s1][0], w, idx);
                g = tc_bc7_dec_interp(ep[s0][1], ep[s1][1], w, idx);
                b = tc_bc7_dec_interp(ep[s0][2], ep[s1][2], w, idx);
                a = ib2 ? tc_bc7_dec_interp(ep[s0][3], ep[s1][3], w2, idx2)
                        : tc_bc7_dec_interp(ep[s0][3], ep[s1][3], w, idx);
            } else {
                r = tc_bc7_dec_interp(ep[s0][0], ep[s1][0], w2, idx2);
                g = tc_bc7_dec_interp(ep[s0][1], ep[s1][1], w2, idx2);
                b = tc_bc7_dec_interp(ep[s0][2], ep[s1][2], w2, idx2);
                a = tc_bc7_dec_interp(ep[s0][3], ep[s1][3], w, idx);
            }
            if (rotation == 1) { int t = a; a = r; r = t; }
            else if (rotation == 2) { int t = a; a = g; g = t; }
            else if (rotation == 3) { int t = a; a = b; b = t; }
            out[i * 4 + j][0] = (uint8_t)r;
            out[i * 4 + j][1] = (uint8_t)g;
            out[i * 4 + j][2] = (uint8_t)b;
            out[i * 4 + j][3] = (uint8_t)a;
        }
}

tc_result tc_bc7_decompress_rgba8(const uint8_t *bc7, uint32_t width,
                                  uint32_t height, size_t stride,
                                  uint8_t *out_rgba, size_t out_size) {
    uint32_t bxc, bx, by, xx, yy;
    if (!bc7 || !out_rgba || !width || !height) return TC_ERROR_INVALID_ARGUMENT;
    if (stride < (size_t)width * 4u) return TC_ERROR_INVALID_ARGUMENT;
    if (out_size < (size_t)(height - 1u) * stride + (size_t)width * 4u)
        return TC_ERROR_INVALID_ARGUMENT;
    bxc = (width + 3u) / 4u;
    for (by = 0; by < height; by += 4u)
        for (bx = 0; bx < width; bx += 4u) {
            uint8_t dec[16][4];
            size_t bi = ((size_t)(by / 4u) * bxc + bx / 4u) * 16u;
            tc_bc7_decode_block(bc7 + bi, dec);
            for (yy = 0; yy < 4u; ++yy) {
                uint32_t y = by + yy;
                if (y >= height) continue;
                for (xx = 0; xx < 4u; ++xx) {
                    uint32_t x = bx + xx;
                    if (x >= width) continue;
                    memcpy(out_rgba + (size_t)y * stride + (size_t)x * 4u,
                           dec[yy * 4u + xx], 4u);
                }
            }
        }
    return TC_SUCCESS;
}

tc_result tc_bc7_decompress_rgbaf(const uint8_t *bc7, uint32_t width,
                                  uint32_t height, size_t stride_bytes,
                                  float *out_rgba, size_t out_size) {
    uint32_t x, y;
    uint8_t *u8;
    size_t need, row_bytes;
    tc_result ret;

    if (!bc7 || !out_rgba || !width || !height) return TC_ERROR_INVALID_ARGUMENT;
    if (out_size < (size_t)height * (size_t)width * 4u * sizeof(float))
        return TC_ERROR_INVALID_ARGUMENT;
    need = (size_t)height * (size_t)width * 4u;
    u8 = (uint8_t *)malloc(need);
    if (!u8) return TC_ERROR_OUT_OF_MEMORY;
    row_bytes = (size_t)width * 4u;
    ret = tc_bc7_decompress_rgba8(bc7, width, height, row_bytes, u8, need);
    if (ret != TC_SUCCESS) { free(u8); return ret; }
    for (y = 0; y < height; ++y) {
        const uint8_t *src = u8 + (size_t)y * row_bytes;
        float *dst = (float *)((uint8_t *)out_rgba + (size_t)y * stride_bytes);
        for (x = 0; x < width; ++x) {
            uint32_t c;
            for (c = 0; c < 4u; ++c)
                dst[x * 4u + c] = (float)src[x * 4u + c] / 255.0f;
        }
    }
    free(u8);
    return TC_SUCCESS;
}

/* Gather the 4x4 source block at (bx,by), clamping to the image edge. */
static void tc_bc7_gather_block(const uint8_t *rgba, uint32_t width,
                                uint32_t height, size_t stride, uint32_t bx,
                                uint32_t by, uint8_t out[16][4]) {
    uint32_t xx, yy, x, y;
    for (yy = 0; yy < 4u; ++yy) {
        y = by + yy;
        if (y >= height) y = height - 1u;
        for (xx = 0; xx < 4u; ++xx) {
            x = bx + xx;
            if (x >= width) x = width - 1u;
            memcpy(out[yy * 4u + xx], rgba + (size_t)y * stride + (size_t)x * 4u,
                   4u);
        }
    }
}

/* Decoder-driven windowed rate-distortion pass. For each block, decode its own
 * encoding and every recent block's encoding (cached in the ring), and reuse
 * the recent block whose *decoded* pixels best match this block's source --
 * accepting only when the reconstruction error it introduces stays within the
 * budget (rdo = max per-channel RMS increase). Reuse turns near-duplicate
 * blocks into exact byte duplicates that zstd matches across the whole image.
 * Because the actual decoded bytes are scored, there is no reuse-chaining drift
 * and the distortion is measured rather than merely bounded. Output stays valid
 * BC7 (every block is a real encoding). */
#define TC_BC7_RDO_WINDOW 64u
static int64_t tc_bc7_block_sse(const uint8_t a[16][4], const uint8_t b[16][4]) {
    int64_t sse = 0;
    uint32_t t;
    for (t = 0; t < 64u; ++t) {
        int d = (int)((const uint8_t *)a)[t] - (int)((const uint8_t *)b)[t];
        sse += (int64_t)d * d;
    }
    return sse;
}
static void tc_bc7_rdo_pass(const uint8_t *rgba, uint32_t width,
                            uint32_t height, size_t stride, int rdo,
                            uint8_t *blocks) {
    uint32_t bxc = (width + 3u) / 4u, byc = (height + 3u) / 4u;
    uint32_t nblocks = bxc * byc, i, ring_count = 0;
    int64_t thresh = (int64_t)rdo * rdo * 16 * 4; /* budget: RMS increase <= rdo */
    uint8_t ring[TC_BC7_RDO_WINDOW][16][4]; /* decoded pixels of recent blocks */
    uint32_t ring_idx[TC_BC7_RDO_WINDOW];   /* block index whose bytes they are */
    for (i = 0; i < nblocks; ++i) {
        uint8_t cur[16][4], own[16][4], final_dec[16][4];
        int64_t own_err, best = 0;
        int best_slot = -1;
        uint32_t k, slot, lim = ring_count < TC_BC7_RDO_WINDOW
                                    ? ring_count
                                    : TC_BC7_RDO_WINDOW;
        tc_bc7_gather_block(rgba, width, height, stride, (i % bxc) * 4u,
                            (i / bxc) * 4u, cur);
        tc_bc7_decode_block(blocks + (size_t)i * 16u, own);
        own_err = tc_bc7_block_sse(cur, own);
        for (k = 0; k < lim; ++k) {
            int64_t sse = tc_bc7_block_sse(cur, ring[k]);
            if (best_slot < 0 || sse < best) {
                best = sse;
                best_slot = (int)k;
            }
        }
        if (best_slot >= 0 && best - own_err <= thresh) {
            memcpy(blocks + (size_t)i * 16u,
                   blocks + (size_t)ring_idx[best_slot] * 16u, 16u);
            memcpy(final_dec, ring[best_slot], sizeof(final_dec));
        } else {
            memcpy(final_dec, own, sizeof(final_dec));
        }
        slot = ring_count % TC_BC7_RDO_WINDOW;
        memcpy(ring[slot], final_dec, sizeof(final_dec));
        ring_idx[slot] = i;
        ++ring_count;
    }
}

tc_result tc_bc7_compress_rgba8(const uint8_t *rgba, uint32_t width,
                                uint32_t height, size_t stride,
                                const tc_bc7_options *opt, uint8_t *out_bc7,
                                size_t out_size) {
    uint32_t bx, by, x, y, xx, yy;
    uint8_t block[16][4];
    size_t need, off = 0;
    tc_bc7_options defopt;

    if (!rgba || !out_bc7 || !width || !height) return TC_ERROR_INVALID_ARGUMENT;
    if (stride < (size_t)width * 4u) return TC_ERROR_INVALID_ARGUMENT;
    need = tc_bc7_compressed_size(width, height);
    if (!need || out_size < need) return TC_ERROR_INVALID_ARGUMENT;
    if (!opt) {
        tc_bc7_options_init(&defopt);
        opt = &defopt;
    }

    /* Resolve per-channel error weights: all-zero -> uniform (byte-identical to
     * the unweighted path). */
    {
        int c, any = 0;
        for (c = 0; c < 4; ++c)
            if (opt->channel_weights[c]) any = 1;
        for (c = 0; c < 4; ++c)
            tc_bc7_cw[c] = any ? (uint32_t)opt->channel_weights[c] : 1u;
    }

    for (by = 0; by < height; by += 4) {
        for (bx = 0; bx < width; bx += 4) {
            for (yy = 0; yy < 4; ++yy) {
                y = by + yy;
                if (y >= height) y = height - 1u;
                for (xx = 0; xx < 4; ++xx) {
                    const uint8_t *src;
                    x = bx + xx;
                    if (x >= width) x = width - 1u;
                    src = rgba + (size_t)y * stride + (size_t)x * 4u;
                    memcpy(block[yy * 4u + xx], src, 4);
                }
            }
            tc_encode_bc7_all_modes_block(block, opt, out_bc7 + off);
            off += 16u;
        }
    }

    if (opt->rdo > 0)
        tc_bc7_rdo_pass(rgba, width, height, stride, opt->rdo, out_bc7);

    return TC_SUCCESS;
}

tc_result tc_bc7_compress_rgbaf(const float *rgba, uint32_t width,
                                uint32_t height, size_t stride_bytes,
                                const tc_bc7_options *opt, uint8_t *out_bc7,
                                size_t out_size) {
    uint32_t x, y;
    uint8_t *u8;
    size_t need, row_bytes;
    tc_result ret;

    if (!rgba || !out_bc7 || !width || !height) return TC_ERROR_INVALID_ARGUMENT;
    if (stride_bytes < (size_t)width * 4u * sizeof(float))
        return TC_ERROR_INVALID_ARGUMENT;

    need = (size_t)height * (size_t)width * 4u;
    u8 = (uint8_t *)malloc(need);
    if (!u8) return TC_ERROR_OUT_OF_MEMORY;

    row_bytes = (size_t)width * 4u;
    for (y = 0; y < height; ++y) {
        const float *src = (const float *)((const uint8_t *)rgba +
                                           (size_t)y * stride_bytes);
        uint8_t *dst = u8 + (size_t)y * row_bytes;
        for (x = 0; x < width; ++x) {
            uint32_t c;
            for (c = 0; c < 4u; ++c) {
                float v = src[x * 4u + c];
                if (v < 0.0f) v = 0.0f;
                if (v > 1.0f) v = 1.0f;
                dst[x * 4u + c] = (uint8_t)(v * 255.0f + 0.5f);
            }
        }
    }

    ret = tc_bc7_compress_rgba8(u8, width, height, row_bytes, opt, out_bc7,
                                out_size);
    free(u8);
    return ret;
}
