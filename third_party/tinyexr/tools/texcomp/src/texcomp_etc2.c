/*
 * TinyEXR texcomp - ETC2 encoder
 *
 * Copyright (c) 2014-2026 Syoyo Fujita and TinyEXR authors
 * SPDX-License-Identifier: Apache-2.0
 */

#include "texcomp.h"
#include "texcomp_internal.h"

#include <limits.h>
#include <stdlib.h>
#include <string.h>

static const int32_t tc_etc1_mod[8][4] = {
    {2, 8, -2, -8},     {5, 17, -5, -17},    {9, 29, -9, -29},
    {13, 42, -13, -42}, {18, 60, -18, -60},  {24, 80, -24, -80},
    {33, 106, -33, -106}, {47, 183, -47, -183}};

static const uint32_t tc_etc2_planar_flags[64] = {
    0x80800402u, 0x80800402u, 0x80800402u, 0x80800402u,
    0x80800402u, 0x80800402u, 0x80800402u, 0x8080e002u,
    0x80800402u, 0x80800402u, 0x8080e002u, 0x8080e002u,
    0x80800402u, 0x8080e002u, 0x8080e002u, 0x8080e002u,
    0x80000402u, 0x80000402u, 0x80000402u, 0x80000402u,
    0x80000402u, 0x80000402u, 0x80000402u, 0x8000e002u,
    0x80000402u, 0x80000402u, 0x8000e002u, 0x8000e002u,
    0x80000402u, 0x8000e002u, 0x8000e002u, 0x8000e002u,
    0x00800402u, 0x00800402u, 0x00800402u, 0x00800402u,
    0x00800402u, 0x00800402u, 0x00800402u, 0x0080e002u,
    0x00800402u, 0x00800402u, 0x0080e002u, 0x0080e002u,
    0x00800402u, 0x0080e002u, 0x0080e002u, 0x0080e002u,
    0x00000402u, 0x00000402u, 0x00000402u, 0x00000402u,
    0x00000402u, 0x00000402u, 0x00000402u, 0x0000e002u,
    0x00000402u, 0x00000402u, 0x0000e002u, 0x0000e002u,
    0x00000402u, 0x0000e002u, 0x0000e002u, 0x0000e002u};

void tc_etc2_options_init(tc_etc2_options *opt) {
    if (!opt) return;
    memset(opt, 0, sizeof(*opt));
    opt->alpha = 1;
}

size_t tc_etc2_rgb_compressed_size(uint32_t width, uint32_t height) {
    size_t bc = tc_bc7_compressed_size(width, height);
    return bc ? bc / 2u : 0u;
}

size_t tc_etc2_rgba_compressed_size(uint32_t width, uint32_t height) {
    return tc_bc7_compressed_size(width, height);
}

static uint64_t tc_etc2_fix_byte_order(uint64_t d) {
    return (d & 0x00000000ffffffffULL) |
           ((d & 0xff00000000000000ULL) >> 24) |
           ((d & 0x000000ff00000000ULL) << 24) |
           ((d & 0x00ff000000000000ULL) >> 8) |
           ((d & 0x0000ff0000000000ULL) << 8);
}

static uint64_t tc_encode_etc2_rgb_flat(const uint8_t block[16][4]) {
    uint32_t i, r = 0, g = 0, b = 0;
    for (i = 0; i < 16u; ++i) {
        r += block[i][0];
        g += block[i][1];
        b += block[i][2];
    }
    r = (r + 8u) >> 4;
    g = (g + 8u) >> 4;
    b = (b + 8u) >> 4;
    return 0x02000000u | ((b & 0xf8u) << 16) | ((g & 0xf8u) << 8) |
           (r & 0xf8u);
}

static uint8_t tc_etc2_convert6(float f) {
    int32_t i = (tc_clamp_i32((int32_t)f, 0, 1023) - 15) >> 1;
    return (uint8_t)((i + 11 - ((i + 11) >> 7) - ((i + 4) >> 7)) >> 3);
}

static uint8_t tc_etc2_convert7(float f) {
    int32_t i = (tc_clamp_i32((int32_t)f, 0, 1023) - 15) >> 1;
    return (uint8_t)((i + 9 - ((i + 9) >> 8) - ((i + 6) >> 8)) >> 2);
}

static uint8_t tc_etc2_expand6(uint32_t v) {
    return (uint8_t)((v >> 4) | (v << 2));
}

static uint8_t tc_etc2_expand7(uint32_t v) {
    return (uint8_t)((v >> 6) | (v << 1));
}

static uint64_t tc_etc2_flat_error(const uint8_t block[16][4], uint64_t flat) {
    uint8_t r = (uint8_t)(flat & 0xffu);
    uint8_t g = (uint8_t)((flat >> 8) & 0xffu);
    uint8_t b = (uint8_t)((flat >> 16) & 0xffu);
    uint64_t err = 0;
    uint32_t i;
    r = (uint8_t)(r | (r >> 5));
    g = (uint8_t)(g | (g >> 5));
    b = (uint8_t)(b | (b >> 5));
    for (i = 0; i < 16u; ++i) {
        int32_t dr = (int32_t)block[i][0] - r;
        int32_t dg = (int32_t)block[i][1] - g;
        int32_t db = (int32_t)block[i][2] - b;
        err += (uint64_t)(dr * dr + dg * dg + db * db);
    }
    return err;
}

static uint8_t tc_etc1_quant4(uint32_t v) {
    return (uint8_t)((v * 15u + 127u) / 255u);
}

static uint8_t tc_etc1_expand4(uint32_t v) {
    return (uint8_t)((v << 4) | v);
}

static uint8_t tc_etc1_quant5(uint32_t v) {
    return (uint8_t)((v * 31u + 127u) / 255u);
}

static uint8_t tc_etc1_expand5(uint32_t v) {
    return (uint8_t)((v << 3) | (v >> 2));
}

static uint32_t tc_etc1_subset_for_split(uint32_t split, uint32_t i) {
    uint32_t x = i & 3u;
    uint32_t y = i >> 2;
    return split == 0u ? (y < 2u ? 1u : 0u) : (x < 2u ? 1u : 0u);
}

static uint64_t tc_encode_etc1_individual(const uint8_t block[16][4],
                                          uint64_t *out_err) {
    uint64_t best_bits = 0;
    uint64_t best_err = ~(uint64_t)0;
    uint32_t split;
    for (split = 0; split < 2u; ++split) {
        uint32_t sum[2][3] = {{0, 0, 0}, {0, 0, 0}};
        uint32_t cnt[2] = {0, 0};
        uint8_t base[2][3];
        uint32_t table[2] = {0, 0};
        uint8_t sel[16];
        uint64_t err_total = 0;
        uint32_t s, c, i, t;
        uint64_t d = (uint64_t)split << 24;

        for (i = 0; i < 16u; ++i) {
            s = tc_etc1_subset_for_split(split, i);
            for (c = 0; c < 3u; ++c) sum[s][c] += block[i][c];
            ++cnt[s];
        }
        for (s = 0; s < 2u; ++s) {
            for (c = 0; c < 3u; ++c) {
                uint32_t avg = (sum[s][c] + cnt[s] / 2u) / cnt[s];
                uint8_t q = tc_etc1_quant4(avg);
                base[s][c] = tc_etc1_expand4(q);
            }
        }

        for (s = 0; s < 2u; ++s) {
            uint64_t best_tab_err = ~(uint64_t)0;
            uint8_t best_tab_sel[16];
            uint32_t best_tab = 0;
            memset(best_tab_sel, 0, sizeof(best_tab_sel));
            for (t = 0; t < 8u; ++t) {
                uint64_t tab_err = 0;
                uint8_t tab_sel[16];
                for (i = 0; i < 16u; ++i) {
                    uint64_t pix_best = ~(uint64_t)0;
                    uint32_t pix_sel = 0;
                    if (tc_etc1_subset_for_split(split, i) != s) {
                        tab_sel[i] = 0;
                        continue;
                    }
                    for (c = 0; c < 4u; ++c) {
                        int32_t rr = tc_clamp_i32((int32_t)base[s][0] + tc_etc1_mod[t][c],
                                                   0, 255);
                        int32_t gg = tc_clamp_i32((int32_t)base[s][1] + tc_etc1_mod[t][c],
                                                   0, 255);
                        int32_t bb = tc_clamp_i32((int32_t)base[s][2] + tc_etc1_mod[t][c],
                                                   0, 255);
                        int32_t er = (int32_t)block[i][0] - rr;
                        int32_t eg = (int32_t)block[i][1] - gg;
                        int32_t eb = (int32_t)block[i][2] - bb;
                        uint64_t e = (uint64_t)(er * er + eg * eg + eb * eb);
                        if (e < pix_best) {
                            pix_best = e;
                            pix_sel = c;
                        }
                    }
                    tab_sel[i] = (uint8_t)pix_sel;
                    tab_err += pix_best;
                }
                if (tab_err < best_tab_err) {
                    best_tab_err = tab_err;
                    best_tab = t;
                    memcpy(best_tab_sel, tab_sel, sizeof(best_tab_sel));
                }
            }
            table[s] = best_tab;
            for (i = 0; i < 16u; ++i) {
                if (tc_etc1_subset_for_split(split, i) == s) sel[i] = best_tab_sel[i];
            }
            err_total += best_tab_err;
        }

        d |= (uint64_t)(base[0][0] >> 4) << 0;
        d |= (uint64_t)(base[1][0] >> 4) << 4;
        d |= (uint64_t)(base[0][1] >> 4) << 8;
        d |= (uint64_t)(base[1][1] >> 4) << 12;
        d |= (uint64_t)(base[0][2] >> 4) << 16;
        d |= (uint64_t)(base[1][2] >> 4) << 20;
        d |= (uint64_t)table[0] << 26;
        d |= (uint64_t)table[1] << 29;
        for (i = 0; i < 16u; ++i) {
            uint64_t sv = sel[i];
            d |= (sv & 1u) << (i + 32u);
            d |= (sv & 2u) << (i + 47u);
        }
        d = tc_etc2_fix_byte_order(d);
        if (err_total < best_err) {
            best_err = err_total;
            best_bits = d;
        }
    }
    if (out_err) *out_err = best_err;
    return best_bits;
}

static uint64_t tc_encode_etc1_differential(const uint8_t block[16][4],
                                            uint64_t *out_err) {
    uint64_t best_bits = 0;
    uint64_t best_err = ~(uint64_t)0;
    uint32_t split;
    for (split = 0; split < 2u; ++split) {
        uint32_t sum[2][3] = {{0, 0, 0}, {0, 0, 0}};
        uint32_t cnt[2] = {0, 0};
        uint8_t q[2][3];
        int32_t diff[3];
        uint8_t base[2][3];
        uint32_t table[2] = {0, 0};
        uint8_t sel[16];
        uint64_t err_total = 0;
        uint32_t s, c, i, t;
        uint64_t d = ((uint64_t)split << 24) | (1ULL << 25);

        for (i = 0; i < 16u; ++i) {
            s = tc_etc1_subset_for_split(split, i);
            for (c = 0; c < 3u; ++c) sum[s][c] += block[i][c];
            ++cnt[s];
        }
        for (s = 0; s < 2u; ++s) {
            for (c = 0; c < 3u; ++c) {
                uint32_t avg = (sum[s][c] + cnt[s] / 2u) / cnt[s];
                q[s][c] = tc_etc1_quant5(avg);
            }
        }
        for (c = 0; c < 3u; ++c) {
            diff[c] = (int32_t)q[0][c] - (int32_t)q[1][c];
            if (diff[c] < -4 || diff[c] > 3) {
                if (out_err) *out_err = ~(uint64_t)0;
                return 0;
            }
            base[0][c] = tc_etc1_expand5(q[0][c]);
            base[1][c] = tc_etc1_expand5(q[1][c]);
        }

        for (s = 0; s < 2u; ++s) {
            uint64_t best_tab_err = ~(uint64_t)0;
            uint8_t best_tab_sel[16];
            uint32_t best_tab = 0;
            memset(best_tab_sel, 0, sizeof(best_tab_sel));
            for (t = 0; t < 8u; ++t) {
                uint64_t tab_err = 0;
                uint8_t tab_sel[16];
                for (i = 0; i < 16u; ++i) {
                    uint64_t pix_best = ~(uint64_t)0;
                    uint32_t pix_sel = 0;
                    if (tc_etc1_subset_for_split(split, i) != s) {
                        tab_sel[i] = 0;
                        continue;
                    }
                    for (c = 0; c < 4u; ++c) {
                        int32_t rr = tc_clamp_i32((int32_t)base[s][0] + tc_etc1_mod[t][c],
                                                   0, 255);
                        int32_t gg = tc_clamp_i32((int32_t)base[s][1] + tc_etc1_mod[t][c],
                                                   0, 255);
                        int32_t bb = tc_clamp_i32((int32_t)base[s][2] + tc_etc1_mod[t][c],
                                                   0, 255);
                        int32_t er = (int32_t)block[i][0] - rr;
                        int32_t eg = (int32_t)block[i][1] - gg;
                        int32_t eb = (int32_t)block[i][2] - bb;
                        uint64_t e = (uint64_t)(er * er + eg * eg + eb * eb);
                        if (e < pix_best) {
                            pix_best = e;
                            pix_sel = c;
                        }
                    }
                    tab_sel[i] = (uint8_t)pix_sel;
                    tab_err += pix_best;
                }
                if (tab_err < best_tab_err) {
                    best_tab_err = tab_err;
                    best_tab = t;
                    memcpy(best_tab_sel, tab_sel, sizeof(best_tab_sel));
                }
            }
            table[s] = best_tab;
            for (i = 0; i < 16u; ++i) {
                if (tc_etc1_subset_for_split(split, i) == s) sel[i] = best_tab_sel[i];
            }
            err_total += best_tab_err;
        }

        d |= (uint64_t)q[1][0] << 0;
        d |= (uint64_t)(diff[0] & 7) << 3;
        d |= (uint64_t)q[1][1] << 8;
        d |= (uint64_t)(diff[1] & 7) << 11;
        d |= (uint64_t)q[1][2] << 16;
        d |= (uint64_t)(diff[2] & 7) << 19;
        d |= (uint64_t)table[0] << 26;
        d |= (uint64_t)table[1] << 29;
        for (i = 0; i < 16u; ++i) {
            uint64_t sv = sel[i];
            d |= (sv & 1u) << (i + 32u);
            d |= (sv & 2u) << (i + 47u);
        }
        d = tc_etc2_fix_byte_order(d);
        if (err_total < best_err) {
            best_err = err_total;
            best_bits = d;
        }
    }
    if (out_err) *out_err = best_err;
    return best_bits;
}

static uint64_t tc_encode_etc2_rgb_planar(const uint8_t block[16][4],
                                          uint64_t *out_err) {
    int32_t sum_r = 0, sum_g = 0, sum_b = 0;
    int32_t dif_rxz = 0, dif_gxz = 0, dif_bxz = 0;
    int32_t dif_ryz = 0, dif_gyz = 0, dif_byz = 0;
    const int32_t scaling[4] = {-255, -85, 85, 255};
    const float scale = -4.0f / ((255.0f * 255.0f * 8.0f + 85.0f * 85.0f * 8.0f) *
                                 16.0f);
    float ar, ag, ab, br, bg, bb, dr, dg, db;
    uint32_t co_r, co_g, co_b, ch_r, ch_g, ch_b, cv_r, cv_g, cv_b;
    uint32_t rgbv, rgbh, lo, hi, idx;
    uint64_t err = 0;
    uint32_t i;

    for (i = 0; i < 16u; ++i) {
        sum_r += block[i][0];
        sum_g += block[i][1];
        sum_b += block[i][2];
    }
    for (i = 0; i < 16u; ++i) {
        int32_t x = (int32_t)(i / 4u);
        int32_t y = (int32_t)(i & 3u);
        int32_t dif_r = ((int32_t)block[i][0] << 4) - sum_r;
        int32_t dif_g = ((int32_t)block[i][1] << 4) - sum_g;
        int32_t dif_b = ((int32_t)block[i][2] << 4) - sum_b;
        dif_rxz += dif_r * scaling[x];
        dif_gxz += dif_g * scaling[x];
        dif_bxz += dif_b * scaling[x];
        dif_ryz += dif_r * scaling[y];
        dif_gyz += dif_g * scaling[y];
        dif_byz += dif_b * scaling[y];
    }

    ar = (float)dif_rxz * scale;
    ag = (float)dif_gxz * scale;
    ab = (float)dif_bxz * scale;
    br = (float)dif_ryz * scale;
    bg = (float)dif_gyz * scale;
    bb = (float)dif_byz * scale;
    dr = (float)sum_r * 0.25f;
    dg = (float)sum_g * 0.25f;
    db = (float)sum_b * 0.25f;

    co_r = tc_etc2_convert6(ar * 255.0f + br * 255.0f + dr);
    co_g = tc_etc2_convert7(ag * 255.0f + bg * 255.0f + dg);
    co_b = tc_etc2_convert6(ab * 255.0f + bb * 255.0f + db);
    ch_r = tc_etc2_convert6(ar * -425.0f + br * 255.0f + dr);
    ch_g = tc_etc2_convert7(ag * -425.0f + bg * 255.0f + dg);
    ch_b = tc_etc2_convert6(ab * -425.0f + bb * 255.0f + db);
    cv_r = tc_etc2_convert6(ar * 255.0f + br * -425.0f + dr);
    cv_g = tc_etc2_convert7(ag * 255.0f + bg * -425.0f + dg);
    cv_b = tc_etc2_convert6(ab * 255.0f + bb * -425.0f + db);

    {
        int32_t ro = tc_etc2_expand6(co_r);
        int32_t go = tc_etc2_expand7(co_g);
        int32_t bo = tc_etc2_expand6(co_b);
        int32_t rh = (int32_t)tc_etc2_expand6(ch_r) - ro;
        int32_t gh = (int32_t)tc_etc2_expand7(ch_g) - go;
        int32_t bh = (int32_t)tc_etc2_expand6(ch_b) - bo;
        int32_t rv = (int32_t)tc_etc2_expand6(cv_r) - ro;
        int32_t gv = (int32_t)tc_etc2_expand7(cv_g) - go;
        int32_t bv = (int32_t)tc_etc2_expand6(cv_b) - bo;
        int32_t ro2 = (ro << 2) + 2;
        int32_t go2 = (go << 2) + 2;
        int32_t bo2 = (bo << 2) + 2;
        for (i = 0; i < 16u; ++i) {
            int32_t x = (int32_t)(i / 4u);
            int32_t y = (int32_t)(i & 3u);
            int32_t rr = tc_clamp_u8_i32((rh * x + rv * y + ro2) >> 2);
            int32_t gg = tc_clamp_u8_i32((gh * x + gv * y + go2) >> 2);
            int32_t bbv = tc_clamp_u8_i32((bh * x + bv * y + bo2) >> 2);
            int32_t er = (int32_t)block[i][0] - rr;
            int32_t eg = (int32_t)block[i][1] - gg;
            int32_t eb = (int32_t)block[i][2] - bbv;
            err += (uint64_t)(er * er + eg * eg + eb * eb);
        }
    }

    rgbv = cv_b | (cv_g << 6) | (cv_r << 13);
    rgbh = ch_b | (ch_g << 6) | (ch_r << 13);
    hi = rgbv | ((rgbh & 0x1fffu) << 19);
    lo = (ch_r & 0x1u) | 0x2u | ((ch_r << 1) & 0x7cu);
    lo |= ((co_b & 0x07u) << 7) | ((co_b & 0x18u) << 8) | ((co_b & 0x20u) << 11);
    lo |= ((co_g & 0x3fu) << 17) | ((co_g & 0x40u) << 18);
    lo |= co_r << 25;
    idx = (co_r & 0x20u) | ((co_g & 0x20u) >> 1) | ((co_b & 0x1eu) >> 1);
    lo |= tc_etc2_planar_flags[idx];
    if (out_err) *out_err = err;
    return (uint64_t)tc_bswap32(lo) | ((uint64_t)tc_bswap32(hi) << 32);
}

static uint64_t tc_encode_etc2_rgb(const uint8_t block[16][4]) {
    uint64_t flat = tc_encode_etc2_rgb_flat(block);
    uint64_t flat_err = tc_etc2_flat_error(block, flat);
    uint64_t individual_err = 0;
    uint64_t individual = tc_encode_etc1_individual(block, &individual_err);
    uint64_t differential_err = 0;
    uint64_t differential = tc_encode_etc1_differential(block, &differential_err);
    uint64_t planar_err = 0;
    uint64_t planar = tc_encode_etc2_rgb_planar(block, &planar_err);
    uint64_t best = flat;
    uint64_t best_err = flat_err;
    if (individual_err < best_err) {
        best = individual;
        best_err = individual_err;
    }
    if (differential_err < best_err) {
        best = differential;
        best_err = differential_err;
    }
    if (planar_err < best_err) best = planar;
    return best;
}

tc_result tc_etc2_compress_rgba8(const uint8_t *rgba, uint32_t width,
                                 uint32_t height, size_t stride,
                                 const tc_etc2_options *opt,
                                 uint8_t *out_etc2, size_t out_size) {
    tc_etc2_options defopt;
    uint32_t bx, by, x, y, xx, yy;
    uint8_t block[16][4];
    uint8_t alpha[16];
    size_t need, off = 0;

    if (!opt) {
        tc_etc2_options_init(&defopt);
        opt = &defopt;
    }
    if (!rgba || !out_etc2 || !width || !height) return TC_ERROR_INVALID_ARGUMENT;
    if (stride < (size_t)width * 4u) return TC_ERROR_INVALID_ARGUMENT;
    need = opt->alpha ? tc_etc2_rgba_compressed_size(width, height)
                      : tc_etc2_rgb_compressed_size(width, height);
    if (!need || out_size < need) return TC_ERROR_INVALID_ARGUMENT;

    for (by = 0; by < height; by += 4u) {
        for (bx = 0; bx < width; bx += 4u) {
            for (yy = 0; yy < 4u; ++yy) {
                y = by + yy;
                if (y >= height) y = height - 1u;
                for (xx = 0; xx < 4u; ++xx) {
                    const uint8_t *src;
                    x = bx + xx;
                    if (x >= width) x = width - 1u;
                    src = rgba + (size_t)y * stride + (size_t)x * 4u;
                    memcpy(block[yy * 4u + xx], src, 4u);
                    alpha[yy * 4u + xx] = src[3];
                }
            }
            if (opt->alpha) {
                tc_wr_u64(out_etc2 + off, tc_encode_eac_alpha(alpha));
                off += 8u;
            }
            tc_wr_u64(out_etc2 + off, tc_encode_etc2_rgb(block));
            off += 8u;
        }
    }

    return TC_SUCCESS;
}
