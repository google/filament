/*
 * TinyEXR - HTJ2K/JPH codec front end.
 *
 * This file implements the OpenEXR HTJ2K chunk wrapper, JPEG 2000 codestream
 * profile validation, and the HT block entropy decoder (cleanup pass). The
 * entropy decoder includes VLC/UVLC table generation, reverse/MEL/MagSgn
 * bitstream readers, quad-based VLC decoding, and coefficient reconstruction.
 * The scalar decoder handles the cleanup, significance propagation, and
 * magnitude refinement passes used by OpenEXR HTJ2K chunks.
 *
 * Table derivation and decoder structure are derived from OpenJPH
 * (BSD-2-Clause, Aous Naman / Kakadu / UNSW). The VLC/UVLC tables match
 * ITU-T T.814 / JPEG 2000 Part 15.
 *
 * Copyright (c) 2014-2026 Syoyo Fujita and TinyEXR authors
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "exr_internal.h"

#include <limits.h>

#if defined(__GNUC__) || defined(__clang__)
#define JPH_MAYBE_UNUSED __attribute__((unused))
#else
#define JPH_MAYBE_UNUSED
#endif

enum {
    JPH_MARK_SOC = 0xff4f,
    JPH_MARK_CAP = 0xff50,
    JPH_MARK_SIZ = 0xff51,
    JPH_MARK_COD = 0xff52,
    JPH_MARK_COC = 0xff53,
    JPH_MARK_TLM = 0xff55,
    JPH_MARK_PRF = 0xff56,
    JPH_MARK_PLM = 0xff57,
    JPH_MARK_PLT = 0xff58,
    JPH_MARK_CPF = 0xff59,
    JPH_MARK_QCD = 0xff5c,
    JPH_MARK_QCC = 0xff5d,
    JPH_MARK_RGN = 0xff5e,
    JPH_MARK_POC = 0xff5f,
    JPH_MARK_PPM = 0xff60,
    JPH_MARK_PPT = 0xff61,
    JPH_MARK_CRG = 0xff63,
    JPH_MARK_COM = 0xff64,
    JPH_MARK_DFS = 0xff72,
    JPH_MARK_ADS = 0xff73,
    JPH_MARK_NLT = 0xff76,
    JPH_MARK_ATK = 0xff79,
    JPH_MARK_SOT = 0xff90,
    JPH_MARK_SOD = 0xff93,
    JPH_MARK_EOC = 0xffd9
};

typedef struct {
    const uint8_t *p;
    const uint8_t *end;
} JphReader;

typedef struct {
    uint32_t xsiz, ysiz, xosiz, yosiz;
    uint32_t xtsiz, ytsiz, xtosiz, ytosiz;
    uint32_t psot;
    const uint8_t *sot_start;
    const uint8_t *tile_data;
    size_t tile_data_size;
    uint16_t rsiz, csiz;
    uint8_t *ssiz;
    uint8_t *xrsiz;
    uint8_t *yrsiz;
    uint8_t *nlt_type;
    uint8_t qcd_exp[16];
    uint8_t qcd_count;
    uint8_t qcd_guard_bits;
    /* Optional per-component quantization (QCC). Each array is [csiz]; a
     * qcc_count[c] of 0 means component c uses the default QCD. Mixed-precision
     * files (e.g. 16-bit half channels plus a 32-bit float Z) carry a QCC for
     * the wider component. */
    uint8_t (*qcc_exp)[16];
    uint8_t *qcc_count;
    uint8_t *qcc_guard_bits;
    uint8_t num_decomps;
    uint8_t mc_trans;
    int saw_siz, saw_cap, saw_cod, saw_qcd, saw_sot;
    int saw_sod, saw_qcc;
} JphProfile;

typedef struct {
    uint32_t w, h;
} JphSize;

typedef struct {
    uint32_t w, h;
    uint32_t cb_w, cb_h;
    uint32_t kmax;
    uint8_t exists;
} JphBandGeom;

typedef struct {
    uint32_t num_w, num_h;
    uint32_t cur_x, cur_y;
} JphPrecinctState;

typedef struct {
    uint32_t comp;
    uint32_t res;
    uint32_t band;
    uint32_t x0;
    uint32_t y0;
    uint32_t missing_msbs;
    uint32_t active_passes;
    uint32_t length0;
    uint32_t length1;
    uint32_t width;
    uint32_t height;
    const uint8_t *data;
    size_t data_size;
} JphCodeblockSeg;

typedef struct {
    JphCodeblockSeg *items;
    size_t count;
    size_t cap;
} JphCodeblockList;

typedef exr_result (*JphCodeblockCallback)(void *user,
                                           const JphCodeblockSeg *seg);

typedef struct {
    const uint8_t *magsgn;
    size_t magsgn_size;
    const uint8_t *mel;
    size_t mel_size;
    const uint8_t *vlc;
    size_t vlc_size;
    const uint8_t *refine;
    size_t refine_size;
    uint32_t scup;
} JphCodeblockStreams;

typedef struct {
    int32_t *data;
    uint32_t w;
    uint32_t h;
} JphComponentPlane;

/* Decode-side coefficient plane. Reversible 5/3 detail coefficients of a B-bit
 * component need up to ~B+2 bits, so 32-bit (float/uint) components overflow an
 * int32 plane; the decode path therefore carries int64 coefficients and only
 * narrows to the final 16/32-bit sample at store time. */
typedef struct {
    int64_t *data;
    /* For all-HALF parts the reversible 5/3 coefficients fit int32, so the
     * encode pipeline carries them in a narrower int32 plane and the codeblock
     * encoder reads it natively (no int32->int64 widening). When data32 is
     * non-NULL it is the source of truth and `data` is unused. */
    int32_t *data32;
    uint32_t w;
    uint32_t h;
} JphPlane64;

/* Dual-width decode plane: when every component is HALF (<=16-bit) the reversible
 * coefficients fit int32, so we use the narrower (and pre-float, proven) int32
 * pipeline + the exported i32 transforms; otherwise int64. Exactly one of
 * d32/d64 is non-NULL. The int32 path halves coefficient-plane bandwidth, which
 * is the dominant cost of the pixel-store and wavelet stages. */
typedef struct {
    int32_t *d32;
    int64_t *d64;
    uint32_t w;
    uint32_t h;
} JphPlaneD;

static uint16_t jph_be16(const uint8_t *p) {
    return (uint16_t)(((uint16_t)p[0] << 8) | (uint16_t)p[1]);
}

static uint32_t jph_be32(const uint8_t *p) {
    return ((uint32_t)p[0] << 24) | ((uint32_t)p[1] << 16) |
           ((uint32_t)p[2] << 8) | (uint32_t)p[3];
}

static int jph_read_u8(JphReader *r, uint8_t *v) {
    if (r->p >= r->end) return 0;
    *v = *r->p++;
    return 1;
}

static int jph_read_be16(JphReader *r, uint16_t *v) {
    if ((size_t)(r->end - r->p) < 2) return 0;
    *v = jph_be16(r->p);
    r->p += 2;
    return 1;
}

static int jph_read_be32(JphReader *r, uint32_t *v) {
    if ((size_t)(r->end - r->p) < 4) return 0;
    *v = jph_be32(r->p);
    r->p += 4;
    return 1;
}

static exr_result jph_skip_segment(JphReader *r) {
    uint16_t len;
    if (!jph_read_be16(r, &len)) return EXR_ERROR_CORRUPT;
    if (len < 2) return EXR_ERROR_CORRUPT;
    if ((size_t)(r->end - r->p) < (size_t)(len - 2)) return EXR_ERROR_CORRUPT;
    r->p += (size_t)(len - 2);
    return EXR_SUCCESS;
}

static exr_result jph_next_marker(JphReader *r, uint16_t *marker) {
    uint8_t b;
    while (jph_read_u8(r, &b)) {
        if (b == 0xff) {
            do {
                if (!jph_read_u8(r, &b)) return EXR_ERROR_CORRUPT;
            } while (b == 0xff);
            if (b == 0x00) continue;
            *marker = (uint16_t)(0xff00u | b);
            return EXR_SUCCESS;
        }
    }
    return EXR_ERROR_CORRUPT;
}

static exr_result jph_find_eoc(JphReader *r, const uint8_t **payload_end) {
    uint8_t b;
    while (jph_read_u8(r, &b)) {
        if (b == 0xff) {
            do {
                if (!jph_read_u8(r, &b)) return EXR_ERROR_CORRUPT;
            } while (b == 0xff);
            if (b == 0x00) continue;
            if ((uint16_t)(0xff00u | b) == JPH_MARK_EOC) {
                *payload_end = r->p - 2;
                return EXR_SUCCESS;
            }
            return EXR_ERROR_UNSUPPORTED;
        }
    }
    return EXR_ERROR_CORRUPT;
}

void exr_jph_bitreader_init(exr_jph_bitreader *br, const uint8_t *data,
                            size_t size) {
    if (!br) return;
    br->p = data;
    br->end = data ? data + size : data;
    br->bits = 0;
    br->bit_count = 0;
    br->prev_ff = 0;
}

static exr_result jph_bitreader_fill(exr_jph_bitreader *br, unsigned need) {
    while (br->bit_count < need) {
        uint8_t byte;
        unsigned nbits = 8;
        if (!br || !br->p || br->p >= br->end) return EXR_ERROR_CORRUPT;
        byte = *br->p++;
        if (br->prev_ff) {
            if (byte & 0x80u) return EXR_ERROR_CORRUPT;
            nbits = 7;
            byte = (uint8_t)(byte & 0x7fu);
        }
        if (br->bit_count + nbits > 64u) return EXR_ERROR_CORRUPT;
        br->bits = (br->bits << nbits) | (uint64_t)byte;
        br->bit_count += nbits;
        br->prev_ff = (byte == 0xffu && nbits == 8u);
    }
    return EXR_SUCCESS;
}

exr_result exr_jph_bitreader_read(exr_jph_bitreader *br, unsigned nbits,
                                  uint32_t *out) {
    exr_result rc;
    uint64_t mask;
    if (!br || !out || nbits > 32u) return EXR_ERROR_INVALID_ARGUMENT;
    if (nbits == 0) {
        *out = 0;
        return EXR_SUCCESS;
    }
    rc = jph_bitreader_fill(br, nbits);
    if (rc != EXR_SUCCESS) return rc;
    mask = (nbits == 32u) ? UINT64_C(0xffffffff) : ((UINT64_C(1) << nbits) - 1u);
    *out = (uint32_t)((br->bits >> (br->bit_count - nbits)) & mask);
    br->bit_count -= nbits;
    if (br->bit_count == 0) br->bits = 0;
    else br->bits &= ((UINT64_C(1) << br->bit_count) - 1u);
    return EXR_SUCCESS;
}

void exr_jph_bitreader_align(exr_jph_bitreader *br) {
    unsigned drop;
    if (!br) return;
    drop = br->bit_count & 7u;
    if (drop) {
        br->bit_count -= drop;
        if (br->bit_count == 0) br->bits = 0;
        else br->bits &= ((UINT64_C(1) << br->bit_count) - 1u);
    }
}

void exr_jph_ht_forward_init(exr_jph_ht_forward_reader *r,
                             const uint8_t *data, size_t size,
                             uint8_t fill_byte) {
    if (!r) return;
    r->p = data;
    r->end = data ? data + size : data;
    r->bits = 0;
    r->bit_count = 0;
    r->fill_byte = fill_byte;
    r->prev_ff = 0;
}

static exr_result jph_ht_forward_fill(exr_jph_ht_forward_reader *r,
                                      unsigned need) {
    while (r->bit_count < need) {
        uint8_t byte;
        unsigned nbits = 8;
        int have_data = 0;
        if (!r) return EXR_ERROR_INVALID_ARGUMENT;
        if (r->p && r->p < r->end) {
            byte = *r->p++;
            have_data = 1;
        } else {
            byte = r->fill_byte;
        }
        if (r->prev_ff) {
            if (have_data && (byte & 0x80u)) return EXR_ERROR_CORRUPT;
            byte &= 0x7fu;
            nbits = 7;
        }
        if (r->bit_count + nbits > 64u) return EXR_ERROR_CORRUPT;
        r->bits |= (uint64_t)byte << r->bit_count;
        r->bit_count += nbits;
        r->prev_ff = (nbits == 8u && byte == 0xffu);
    }
    return EXR_SUCCESS;
}

exr_result exr_jph_ht_forward_read(exr_jph_ht_forward_reader *r,
                                   unsigned nbits, uint32_t *out) {
    exr_result rc;
    uint64_t mask;
    if (!r || !out || nbits > 32u) return EXR_ERROR_INVALID_ARGUMENT;
    if (nbits == 0) {
        *out = 0;
        return EXR_SUCCESS;
    }
    rc = jph_ht_forward_fill(r, nbits);
    if (rc != EXR_SUCCESS) return rc;
    mask = (nbits == 32u) ? UINT64_C(0xffffffff) : ((UINT64_C(1) << nbits) - 1u);
    *out = (uint32_t)(r->bits & mask);
    r->bits >>= nbits;
    r->bit_count -= nbits;
    return EXR_SUCCESS;
}

void exr_jph_ht_reverse_init(exr_jph_ht_reverse_reader *r,
                             const uint8_t *data, size_t size,
                             int initial_unstuff, int zero_fill) {
    if (!r) return;
    r->start = data;
    r->p = data ? data + size : data;
    r->bits = 0;
    r->bit_count = 0;
    r->unstuff = initial_unstuff != 0;
    r->zero_fill = zero_fill != 0;
}

static exr_result jph_ht_reverse_fill(exr_jph_ht_reverse_reader *r,
                                      unsigned need) {
    while (r->bit_count < need) {
        uint8_t byte = 0;
        unsigned nbits = 8;
        if (!r) return EXR_ERROR_INVALID_ARGUMENT;
        if (r->p && r->p > r->start) byte = *--r->p;
        else if (!r->zero_fill) byte = 0;
        if (r->unstuff && ((byte & 0x7fu) == 0x7fu)) {
            if (byte & 0x80u) return EXR_ERROR_CORRUPT;
            nbits = 7;
        }
        if (r->bit_count + nbits > 64u) return EXR_ERROR_CORRUPT;
        r->bits |= (uint64_t)(byte & ((1u << nbits) - 1u)) << r->bit_count;
        r->bit_count += nbits;
        r->unstuff = byte > 0x8fu;
    }
    return EXR_SUCCESS;
}

exr_result exr_jph_ht_reverse_read(exr_jph_ht_reverse_reader *r,
                                   unsigned nbits, uint32_t *out) {
    exr_result rc;
    uint64_t mask;
    if (!r || !out || nbits > 32u) return EXR_ERROR_INVALID_ARGUMENT;
    if (nbits == 0) {
        *out = 0;
        return EXR_SUCCESS;
    }
    rc = jph_ht_reverse_fill(r, nbits);
    if (rc != EXR_SUCCESS) return rc;
    mask = (nbits == 32u) ? UINT64_C(0xffffffff) : ((UINT64_C(1) << nbits) - 1u);
    *out = (uint32_t)(r->bits & mask);
    r->bits >>= nbits;
    r->bit_count -= nbits;
    return EXR_SUCCESS;
}

void exr_jph_mel_init(exr_jph_mel_reader *r, const uint8_t *data,
                      size_t size) {
    if (!r) return;
    r->p = data;
    r->end = data ? data + size : data;
    r->bits = 0;
    r->bit_count = 0;
    r->prev_ff = 0;
    r->k = 0;
}

static exr_result jph_mel_read_bits(exr_jph_mel_reader *r, unsigned nbits,
                                    uint32_t *out) {
    uint64_t mask;
    if (!r || !out || nbits > 32u) return EXR_ERROR_INVALID_ARGUMENT;
    while (r->bit_count < nbits) {
        uint8_t byte;
        unsigned add_bits = 8;
        if (r->p && r->p < r->end) {
            byte = *r->p++;
            if (r->p == r->end) byte |= 0x0fu;
        } else {
            byte = 0xffu;
        }
        if (r->prev_ff) {
            if (byte > 0x8fu) return EXR_ERROR_CORRUPT;
            byte &= 0x7fu;
            add_bits = 7;
        }
        if (r->bit_count + add_bits > 64u) return EXR_ERROR_CORRUPT;
        r->bits = (r->bits << add_bits) | (uint64_t)byte;
        r->bit_count += add_bits;
        r->prev_ff = (add_bits == 8u && byte == 0xffu);
    }
    if (nbits == 0) {
        *out = 0;
        return EXR_SUCCESS;
    }
    mask = (nbits == 32u) ? UINT64_C(0xffffffff) : ((UINT64_C(1) << nbits) - 1u);
    *out = (uint32_t)((r->bits >> (r->bit_count - nbits)) & mask);
    r->bit_count -= nbits;
    if (r->bit_count == 0) r->bits = 0;
    else r->bits &= ((UINT64_C(1) << r->bit_count) - 1u);
    return EXR_SUCCESS;
}

exr_result exr_jph_mel_get_run(exr_jph_mel_reader *r, uint32_t *zero_run,
                               int *has_one) {
    static const uint8_t mel_exp[13] = {
        0, 0, 0, 1, 1, 1, 2, 2, 2, 3, 3, 4, 5
    };
    uint32_t bit = 0, v = 0;
    uint8_t eval;
    exr_result rc;
    if (!r || !zero_run || !has_one || r->k < 0 || r->k > 12)
        return EXR_ERROR_INVALID_ARGUMENT;
    eval = mel_exp[r->k];
    rc = jph_mel_read_bits(r, 1, &bit);
    if (rc != EXR_SUCCESS) return rc;
    if (bit) {
        *zero_run = (1u << eval) - 1u;
        *has_one = 0;
        if (r->k < 12) r->k++;
        return EXR_SUCCESS;
    }
    if (eval) {
        rc = jph_mel_read_bits(r, eval, &v);
        if (rc != EXR_SUCCESS) return rc;
    }
    *zero_run = v;
    *has_one = 1;
    if (r->k > 0) r->k--;
    return EXR_SUCCESS;
}

static uint32_t jph_bit_length_u32(uint32_t v) {
    uint32_t n = 0;
    while (v) {
        n++;
        v >>= 1u;
    }
    return n;
}

exr_result exr_jph_packet_read_pass_count(exr_jph_bitreader *br,
                                          uint32_t *raw_passes,
                                          uint32_t *active_passes,
                                          uint32_t *placeholder_groups) {
    uint32_t bit = 0, passes = 1, groups;
    exr_result rc;
    if (!br || !raw_passes || !active_passes || !placeholder_groups)
        return EXR_ERROR_INVALID_ARGUMENT;

    rc = exr_jph_bitreader_read(br, 1, &bit);
    if (rc != EXR_SUCCESS) return rc;
    if (bit) {
        passes = 2;
        rc = exr_jph_bitreader_read(br, 1, &bit);
        if (rc != EXR_SUCCESS) return rc;
        if (bit) {
            rc = exr_jph_bitreader_read(br, 2, &bit);
            if (rc != EXR_SUCCESS) return rc;
            passes = 3u + bit;
            if (bit == 3u) {
                rc = exr_jph_bitreader_read(br, 5, &bit);
                if (rc != EXR_SUCCESS) return rc;
                passes = 6u + bit;
                if (bit == 31u) {
                    rc = exr_jph_bitreader_read(br, 7, &bit);
                    if (rc != EXR_SUCCESS) return rc;
                    passes = 37u + bit;
                }
            }
        }
    }

    groups = (passes - 1u) / 3u;
    *raw_passes = passes;
    *placeholder_groups = groups;
    *active_passes = passes - groups * 3u;
    if (*active_passes == 0u || *active_passes > 3u) return EXR_ERROR_CORRUPT;
    return EXR_SUCCESS;
}

exr_result exr_jph_packet_read_pass_lengths(exr_jph_bitreader *br,
                                            uint32_t active_passes,
                                            uint32_t placeholder_groups,
                                            uint32_t lengths[2]) {
    uint32_t bit = 1, lblock = 3, nbits, v;
    exr_result rc;
    if (!br || !lengths) return EXR_ERROR_INVALID_ARGUMENT;
    if (active_passes == 0u || active_passes > 3u)
        return EXR_ERROR_INVALID_ARGUMENT;
    lengths[0] = 0;
    lengths[1] = 0;

    while (bit) {
        rc = exr_jph_bitreader_read(br, 1, &bit);
        if (rc != EXR_SUCCESS) return rc;
        if (bit) {
            lblock++;
            if (lblock > 32u) return EXR_ERROR_CORRUPT;
        }
    }

    nbits = lblock + jph_bit_length_u32(placeholder_groups + 1u) - 1u;
    if (nbits > 32u) return EXR_ERROR_CORRUPT;
    rc = exr_jph_bitreader_read(br, nbits, &v);
    if (rc != EXR_SUCCESS) return rc;
    if (v < 2u || v >= 65535u) return EXR_ERROR_CORRUPT;
    lengths[0] = v;

    if (active_passes > 1u) {
        nbits = lblock + (active_passes > 2u ? 1u : 0u);
        if (nbits > 32u) return EXR_ERROR_CORRUPT;
        rc = exr_jph_bitreader_read(br, nbits, &v);
        if (rc != EXR_SUCCESS) return rc;
        if (v >= 2047u) return EXR_ERROR_CORRUPT;
        lengths[1] = v;
    }
    return EXR_SUCCESS;
}

exr_result exr_jph_tag_tree_init(const exr_allocator *a,
                                 exr_jph_tag_tree *tree, uint32_t width,
                                 uint32_t height) {
    uint32_t w = width, h = height, level = 0;
    size_t total = 0;
    if (!tree) return EXR_ERROR_INVALID_ARGUMENT;
    if (!a) a = exr_default_allocator();
    memset(tree, 0, sizeof(*tree));
    if (width == 0 || height == 0) return EXR_ERROR_INVALID_ARGUMENT;
    while (1) {
        size_t nodes;
        if (level >= EXR_JPH_TAGTREE_MAX_LEVELS) return EXR_ERROR_CORRUPT;
        tree->width[level] = w;
        tree->height[level] = h;
        tree->offset[level] = total;
        if (exr_mul_ovf((size_t)w, (size_t)h, &nodes) ||
            exr_add_ovf(total, nodes, &total))
            return EXR_ERROR_CORRUPT;
        level++;
        if (w == 1u && h == 1u) break;
        w = (w + 1u) / 2u;
        h = (h + 1u) / 2u;
    }
    tree->num_levels = level;
    tree->node_count = total;
    tree->value = (uint32_t *)exr_calloc(a, total, sizeof(uint32_t));
    tree->known = (uint8_t *)exr_calloc(a, total, 1);
    if (!tree->value || !tree->known) {
        exr_jph_tag_tree_free(a, tree);
        return EXR_ERROR_OUT_OF_MEMORY;
    }
    return EXR_SUCCESS;
}

void exr_jph_tag_tree_free(const exr_allocator *a, exr_jph_tag_tree *tree) {
    if (!tree) return;
    if (!a) a = exr_default_allocator();
    exr_free(a, tree->value);
    exr_free(a, tree->known);
    memset(tree, 0, sizeof(*tree));
}

exr_result exr_jph_tag_tree_decode(exr_jph_tag_tree *tree,
                                   exr_jph_bitreader *br, uint32_t leaf_x,
                                   uint32_t leaf_y, uint32_t threshold,
                                   uint32_t *out_value) {
    uint32_t xs[EXR_JPH_TAGTREE_MAX_LEVELS];
    uint32_t ys[EXR_JPH_TAGTREE_MAX_LEVELS];
    uint32_t level, low = 0;
    if (!tree || !br || !out_value || !tree->value || !tree->known)
        return EXR_ERROR_INVALID_ARGUMENT;
    if (tree->num_levels == 0 || leaf_x >= tree->width[0] ||
        leaf_y >= tree->height[0])
        return EXR_ERROR_INVALID_ARGUMENT;

    xs[0] = leaf_x;
    ys[0] = leaf_y;
    for (level = 1; level < tree->num_levels; ++level) {
        xs[level] = xs[level - 1u] >> 1u;
        ys[level] = ys[level - 1u] >> 1u;
    }

    for (level = tree->num_levels; level > 0; --level) {
        uint32_t li = level - 1u;
        size_t idx = tree->offset[li] +
                     (size_t)ys[li] * tree->width[li] + xs[li];
        if (idx >= tree->node_count) return EXR_ERROR_CORRUPT;
        if (tree->value[idx] < low) tree->value[idx] = low;
        while (tree->value[idx] < threshold && !tree->known[idx]) {
            uint32_t bit = 0;
            exr_result rc = exr_jph_bitreader_read(br, 1, &bit);
            if (rc != EXR_SUCCESS) return rc;
            if (bit) tree->known[idx] = 1;
            else tree->value[idx]++;
        }
        low = tree->value[idx];
    }
    *out_value = low;
    return EXR_SUCCESS;
}

static uint32_t jph_divceil_u32(uint32_t a, uint32_t b) {
    return b ? (a / b + ((a % b) != 0)) : 0;
}

static uint32_t jph_ceil_div_pow2_u32(uint32_t v, uint32_t shift) {
    while (shift--) v = (v + 1u) >> 1u;
    return v;
}

static int64_t jph_floor_div_pow2(int64_t v, unsigned shift) {
    int64_t d = (int64_t)1 << shift;
    if (v >= 0) return v / d;
    return -(((-v) + d - 1) / d);
}

static exr_result jph_i64_to_i32(int64_t v, int32_t *out) {
    if (v < (int64_t)INT32_MIN || v > (int64_t)INT32_MAX)
        return EXR_ERROR_CORRUPT;
    *out = (int32_t)v;
    return EXR_SUCCESS;
}

exr_result exr_jph_inverse_rct_i32(int32_t *c0, int32_t *c1, int32_t *c2,
                                   size_t count) {
    size_t i = 0;
    if ((!c0 || !c1 || !c2) && count) return EXR_ERROR_INVALID_ARGUMENT;
#if defined(EXR_X86)
    if (exr_cpu_caps() & EXR_SIMD_AVX2)
        i = jph_inverse_rct_i32_avx2(c0, c1, c2, count);
    else if (exr_cpu_caps() & EXR_SIMD_SSE2)
        i = jph_inverse_rct_i32_sse2(c0, c1, c2, count);
#endif
    for (; i < count; ++i) {
        int64_t y = c0[i];
        int64_t db = c1[i];
        int64_t dr = c2[i];
        int64_t g = y - jph_floor_div_pow2(db + dr, 2);
        int64_t r = dr + g;
        int64_t b = db + g;
        exr_result rc = jph_i64_to_i32(r, &c0[i]);
        if (rc != EXR_SUCCESS) return rc;
        rc = jph_i64_to_i32(g, &c1[i]);
        if (rc != EXR_SUCCESS) return rc;
        rc = jph_i64_to_i32(b, &c2[i]);
        if (rc != EXR_SUCCESS) return rc;
    }
    return EXR_SUCCESS;
}

/* Scalar reference: if v<0, v = ~v - biasm1 (== -v-bias for bit_depth<=31). */
void jph_nlt_type3_i32_scalar(int32_t *data, size_t count, int32_t biasm1) {
    size_t i;
    for (i = 0; i < count; ++i)
        if (data[i] < 0) data[i] = ~data[i] - biasm1;
}

exr_result exr_jph_apply_nlt_type3_i32(int32_t *data, size_t count,
                                       uint32_t bit_depth) {
    size_t i;
    int64_t bias;
    if (!data && count) return EXR_ERROR_INVALID_ARGUMENT;
    if (bit_depth == 0 || bit_depth > 32u) return EXR_ERROR_INVALID_ARGUMENT;
    bias = ((int64_t)1 << (bit_depth - 1u)) + 1;
    /* For bit_depth <= 31, -v-bias always lands in int32 (~v in [0,2^31-1],
     * minus a non-negative bias-1), so it equals ~v-(bias-1) computed purely in
     * int32 - no range check needed; dispatch to SIMD. */
    if (bit_depth <= 31u) {
        int32_t biasm1 = (int32_t)(bias - 1);
#if defined(EXR_X86)
        uint32_t caps = exr_cpu_caps();
        if (caps & EXR_SIMD_AVX2) {
            jph_nlt_type3_i32_avx2(data, count, biasm1);
            return EXR_SUCCESS;
        }
        if (caps & EXR_SIMD_SSE2) {
            jph_nlt_type3_i32_sse2(data, count, biasm1);
            return EXR_SUCCESS;
        }
#elif defined(EXR_NEON)
        jph_nlt_type3_i32_neon(data, count, biasm1);
        return EXR_SUCCESS;
#endif
        jph_nlt_type3_i32_scalar(data, count, biasm1);
        return EXR_SUCCESS;
    }
    /* bit_depth == 32 (not reached on the all-HALF int32 fast path). */
    for (i = 0; i < count; ++i) {
        int64_t v = data[i];
        if (v < 0) {
            exr_result rc = jph_i64_to_i32(-v - bias, &data[i]);
            if (rc != EXR_SUCCESS) return rc;
        }
    }
    return EXR_SUCCESS;
}

exr_result exr_jph_inverse_53_i32(const int32_t *low, size_t low_count,
                                  const int32_t *high, size_t high_count,
                                  int32_t *out, size_t out_count) {
    size_t i;
    size_t expected_low = (out_count + 1u) / 2u;
    size_t expected_high = out_count / 2u;

    if (!out && out_count) return EXR_ERROR_INVALID_ARGUMENT;
    if (low_count != expected_low || high_count != expected_high)
        return EXR_ERROR_INVALID_ARGUMENT;
    if ((!low && low_count) || (!high && high_count))
        return EXR_ERROR_INVALID_ARGUMENT;
    if (out_count == 0) return EXR_SUCCESS;
    if (high_count == 0) {
        out[0] = low[0];
        return EXR_SUCCESS;
    }

    for (i = 0; i < low_count; ++i) {
        int64_t dl = high[i > 0 ? i - 1u : 0u];
        int64_t dr = high[i < high_count ? i : high_count - 1u];
        int64_t even = (int64_t)low[i] - jph_floor_div_pow2(dl + dr + 2, 2);
        exr_result rc = jph_i64_to_i32(even, &out[2u * i]);
        if (rc != EXR_SUCCESS) return rc;
    }
    for (i = 0; i < high_count; ++i) {
        int64_t e0 = out[2u * i];
        int64_t e1 = (i + 1u < low_count) ? out[2u * (i + 1u)] : e0;
        int64_t odd = (int64_t)high[i] + jph_floor_div_pow2(e0 + e1, 1);
        exr_result rc = jph_i64_to_i32(odd, &out[2u * i + 1u]);
        if (rc != EXR_SUCCESS) return rc;
    }
    return EXR_SUCCESS;
}

/* Row-wise (column-parallel) vertical inverse reversible 5/3, int32. This is the
 * source of truth for the SIMD jph_inverse_53_vert_i32_avx2: it must stay
 * bit-identical. `temp` holds `lh` contiguous low-rows [0..lh) then `hh`
 * high-rows [lh..lh+hh), each `rw` wide (stride rw). The rh = lh+hh
 * reconstructed rows are written interleaved into `data` (stride `width`): even
 * output row 2i is the predict of low row i, odd output row 2i+1 the update of
 * high row i. Per column this is exactly exr_jph_inverse_53_i32; running it
 * row-wise over all columns avoids the strided gather/scatter (and the natural
 * SIMD axis is the column, so it vectorizes cleanly). Returns EXR_ERROR_CORRUPT
 * iff any reconstructed sample leaves int32 (matching the 1D kernel). */
exr_result exr_jph_inverse_53_vert_i32(const int32_t *temp, size_t rw,
                                       size_t lh, size_t hh,
                                       int32_t *data, size_t width) {
    size_t i, c;
    if ((!temp || !data) && rw) return EXR_ERROR_INVALID_ARGUMENT;
    if (lh == 0u) return EXR_SUCCESS; /* rh == 0; lh >= 1 whenever rh >= 1 */
    if (hh == 0u) {                   /* rh == 1: copy low row 0 verbatim */
        for (c = 0u; c < rw; ++c) data[c] = temp[c];
        return EXR_SUCCESS;
    }
    /* Phase 1: even output rows -> data[2i]. */
    for (i = 0u; i < lh; ++i) {
        const int32_t *lo = temp + i * rw;
        const int32_t *hL = temp + (lh + (i > 0u ? i - 1u : 0u)) * rw;
        const int32_t *hR = temp + (lh + (i < hh ? i : hh - 1u)) * rw;
        int32_t *dst = data + (2u * i) * width;
        for (c = 0u; c < rw; ++c) {
            int64_t e = (int64_t)lo[c] -
                        jph_floor_div_pow2((int64_t)hL[c] + (int64_t)hR[c] + 2, 2);
            exr_result rc = jph_i64_to_i32(e, &dst[c]);
            if (rc != EXR_SUCCESS) return rc;
        }
    }
    /* Phase 2: odd output rows -> data[2i+1], reading even rows back from data.
     * Every even of the whole plane has passed the int32 range check above, so
     * each read-back equals its exact int64 value -- bit-identical to the 1D
     * kernel, which likewise reads narrowed evens back for the update. */
    for (i = 0u; i < hh; ++i) {
        const int32_t *hi = temp + (lh + i) * rw;
        const int32_t *e0p = data + (2u * i) * width;
        const int32_t *e1p = data + (2u * (i + 1u < lh ? i + 1u : i)) * width;
        int32_t *dst = data + (2u * i + 1u) * width;
        for (c = 0u; c < rw; ++c) {
            int64_t o = (int64_t)hi[c] +
                        jph_floor_div_pow2((int64_t)e0p[c] + (int64_t)e1p[c], 1);
            exr_result rc = jph_i64_to_i32(o, &dst[c]);
            if (rc != EXR_SUCCESS) return rc;
        }
    }
    return EXR_SUCCESS;
}

static size_t jph_ceil_div_pow2_size(size_t v, unsigned shift) {
    while (shift--) v = (v + 1u) / 2u;
    return v;
}

typedef struct {
    uint16_t *scratch;
    size_t scratch_cap;
    uint32_t *v_n32;
    size_t v_n32_cap;
    uint32_t *buf32;
    size_t buf32_cap;
    int32_t *inv53_temp32;
    size_t inv53_temp32_cap;
    int64_t *inv53_ev64;
    size_t inv53_ev64_cap;
    int64_t *inv53_od64;
    size_t inv53_od64_cap;
    uint64_t *magsgn_bits;
    size_t magsgn_bits_cap;
    uint64_t *sigprop_bits;
    size_t sigprop_bits_cap;
} JphDecodeWorkspace;

static exr_result jph_ws_reserve_raw(const exr_allocator *a, void **ptr,
                                     size_t *cap, size_t count,
                                     size_t elem_size);

static exr_result jph_inverse_53_2d_i32_ws(const exr_allocator *a, int32_t *data,
                                           size_t width, size_t height,
                                           unsigned levels,
                                           JphDecodeWorkspace *ws,
                                           int bounded) {
    unsigned level;
    int use_simd = 0;
#if !defined(EXR_X86)
    /* `bounded` selects the int32 vs int64-intermediate AVX2 variant; the NEON
     * and scalar paths use a single (always-safe) implementation. */
    (void)bounded;
#endif
    if (!a) a = exr_default_allocator();
    if (!data && width && height) return EXR_ERROR_INVALID_ARGUMENT;
    if (levels > 32) return EXR_ERROR_INVALID_ARGUMENT;
    if (width == 0 || height == 0 || levels == 0) return EXR_SUCCESS;
#if defined(EXR_X86)
    use_simd = (exr_cpu_caps() & EXR_SIMD_AVX2) != 0;
#elif defined(EXR_NEON)
    use_simd = 1;
#endif
    if (ws) {
        size_t max_temp_count, max_scratch_len;
        exr_result rc;
        if (exr_mul_ovf(width, height, &max_temp_count))
            return EXR_ERROR_CORRUPT;
        max_scratch_len = width > height ? width : height;
        rc = jph_ws_reserve_raw(a, (void **)&ws->inv53_temp32,
                                &ws->inv53_temp32_cap, max_temp_count,
                                sizeof(int32_t));
        if (rc != EXR_SUCCESS) return rc;
        if (use_simd) {
            rc = jph_ws_reserve_raw(a, (void **)&ws->inv53_ev64,
                                    &ws->inv53_ev64_cap, max_scratch_len,
                                    sizeof(int64_t));
            if (rc != EXR_SUCCESS) return rc;
            rc = jph_ws_reserve_raw(a, (void **)&ws->inv53_od64,
                                    &ws->inv53_od64_cap, max_scratch_len,
                                    sizeof(int64_t));
            if (rc != EXR_SUCCESS) return rc;
        }
    }

    for (level = levels; level > 0; --level) {
        size_t rw = jph_ceil_div_pow2_size(width, level - 1u);
        size_t rh = jph_ceil_div_pow2_size(height, level - 1u);
        size_t lw = (rw + 1u) / 2u, hw = rw / 2u;
        size_t lh = (rh + 1u) / 2u, hh = rh / 2u;
        size_t temp_count, temp_bytes, scratch_len, sb64;
        int32_t *temp = NULL;
        int64_t *ev = NULL, *od = NULL; /* SIMD 1D row-pass scratch */
        size_t y;
        exr_result rc = EXR_SUCCESS;

        if (rw == 0 || rh == 0) return EXR_ERROR_CORRUPT;
        if (exr_mul_ovf(rw, rh, &temp_count) ||
            exr_mul_ovf(temp_count, sizeof(int32_t), &temp_bytes))
            return EXR_ERROR_CORRUPT;
        scratch_len = rw > rh ? rw : rh;
        if (exr_mul_ovf(scratch_len, sizeof(int64_t), &sb64))
            return EXR_ERROR_CORRUPT;

        if (ws) {
            temp = ws->inv53_temp32;
            if (use_simd) {
                ev = ws->inv53_ev64;
                od = ws->inv53_od64;
            }
        } else {
            temp = (int32_t *)exr_malloc(a, temp_bytes);
            if (use_simd) {
                ev = (int64_t *)exr_malloc(a, sb64);
                od = (int64_t *)exr_malloc(a, sb64);
            }
        }
        if (!temp || (use_simd && (!ev || !od))) {
            if (!ws) {
                exr_free(a, temp);
                exr_free(a, ev);
                exr_free(a, od);
            }
            return EXR_ERROR_OUT_OF_MEMORY;
        }

        /* Horizontal (row) pass: each row is contiguous low|high. */
        for (y = 0; y < rh; ++y) {
            const int32_t *row = data + y * width;
#if defined(EXR_X86)
            if (use_simd) {
                if (bounded)
                    rc = jph_inverse_53_i32_bounded_avx2(row, lw, row + lw, hw,
                                                         temp + y * rw, rw, ev,
                                                         od);
                else
                    rc = jph_inverse_53_i32_avx2(row, lw, row + lw, hw,
                                                 temp + y * rw, rw, ev, od);
            }
            else
#elif defined(EXR_NEON)
            if (use_simd)
                rc = jph_inverse_53_i32_neon(row, lw, row + lw, hw,
                                             temp + y * rw, rw, ev, od);
            else
#endif
                rc = exr_jph_inverse_53_i32(row, lw, row + lw, hw,
                                            temp + y * rw, rw);
            if (rc != EXR_SUCCESS) goto done_level;
        }
        /* Vertical (column) pass, row-wise across all columns -- no gather/
         * scatter. temp's lh low-rows / hh high-rows -> interleaved data rows. */
#if defined(EXR_X86)
        if (use_simd) {
            if (bounded)
                rc = jph_inverse_53_vert_i32_bounded_avx2(temp, rw, lh, hh,
                                                          data, width);
            else
                rc = jph_inverse_53_vert_i32_avx2(temp, rw, lh, hh, data,
                                                  width);
        }
        else
#elif defined(EXR_NEON)
        if (use_simd)
            rc = jph_inverse_53_vert_i32_neon(temp, rw, lh, hh, data, width);
        else
#endif
            rc = exr_jph_inverse_53_vert_i32(temp, rw, lh, hh, data, width);

done_level:
        if (!ws) {
            exr_free(a, temp);
            exr_free(a, ev);
            exr_free(a, od);
        }
        if (rc != EXR_SUCCESS) return rc;
    }
    return EXR_SUCCESS;
}

exr_result exr_jph_inverse_53_2d_i32(const exr_allocator *a, int32_t *data,
                                     size_t width, size_t height,
                                     unsigned levels) {
    return jph_inverse_53_2d_i32_ws(a, data, width, height, levels, NULL, 0);
}

/* ---- int64 decode variants -----------------------------------------------
 * These mirror the exported i32 transforms above but keep full int64 width so
 * that >=32-bit-precision components (whose reversible-wavelet coefficients can
 * exceed int32) decode losslessly. The final samples fit in 16/32 bits and are
 * narrowed back to int32 at store time. */
EXR_NO_SANITIZE_SIO
exr_result jph_inverse_53_i64(const int64_t *low, size_t low_count,
                              const int64_t *high, size_t high_count,
                              int64_t *out, size_t out_count) {
    size_t i;
    size_t expected_low = (out_count + 1u) / 2u;
    size_t expected_high = out_count / 2u;
    if (!out && out_count) return EXR_ERROR_INVALID_ARGUMENT;
    if (low_count != expected_low || high_count != expected_high)
        return EXR_ERROR_INVALID_ARGUMENT;
    if ((!low && low_count) || (!high && high_count))
        return EXR_ERROR_INVALID_ARGUMENT;
    if (out_count == 0) return EXR_SUCCESS;
    if (high_count == 0) {
        out[0] = low[0];
        return EXR_SUCCESS;
    }
    for (i = 0; i < low_count; ++i) {
        int64_t dl = high[i > 0 ? i - 1u : 0u];
        int64_t dr = high[i < high_count ? i : high_count - 1u];
        out[2u * i] = low[i] - jph_floor_div_pow2(dl + dr + 2, 2);
    }
    for (i = 0; i < high_count; ++i) {
        int64_t e0 = out[2u * i];
        int64_t e1 = (i + 1u < low_count) ? out[2u * (i + 1u)] : e0;
        out[2u * i + 1u] = high[i] + jph_floor_div_pow2(e0 + e1, 1);
    }
    return EXR_SUCCESS;
}

/* Row-wise (column-parallel) vertical inverse 5/3, int64. Source of truth for
 * jph_inverse_53_vert_i64_avx2. Mirrors exr_jph_inverse_53_vert_i32 but stays
 * full int64 (no narrowing / range check): `temp` holds lh low-rows then hh
 * high-rows (stride rw); writes the rh interleaved rows into `data` (stride
 * width). Per column this is exactly jph_inverse_53_i64. */
EXR_NO_SANITIZE_SIO
exr_result jph_inverse_53_vert_i64(const int64_t *temp, size_t rw,
                                   size_t lh, size_t hh,
                                   int64_t *data, size_t width) {
    size_t i, c;
    if (lh == 0u) return EXR_SUCCESS;
    if (hh == 0u) { /* rh == 1: copy low row 0 */
        for (c = 0u; c < rw; ++c) data[c] = temp[c];
        return EXR_SUCCESS;
    }
    /* Phase 1: even rows -> data[2i]. */
    for (i = 0u; i < lh; ++i) {
        const int64_t *lo = temp + i * rw;
        const int64_t *hL = temp + (lh + (i > 0u ? i - 1u : 0u)) * rw;
        const int64_t *hR = temp + (lh + (i < hh ? i : hh - 1u)) * rw;
        int64_t *dst = data + (2u * i) * width;
        for (c = 0u; c < rw; ++c)
            dst[c] = lo[c] - jph_floor_div_pow2(hL[c] + hR[c] + 2, 2);
    }
    /* Phase 2: odd rows -> data[2i+1], reading even rows back from data. */
    for (i = 0u; i < hh; ++i) {
        const int64_t *hi = temp + (lh + i) * rw;
        const int64_t *e0p = data + (2u * i) * width;
        const int64_t *e1p = data + (2u * (i + 1u < lh ? i + 1u : i)) * width;
        int64_t *dst = data + (2u * i + 1u) * width;
        for (c = 0u; c < rw; ++c)
            dst[c] = hi[c] + jph_floor_div_pow2(e0p[c] + e1p[c], 1);
    }
    return EXR_SUCCESS;
}

static exr_result jph_inverse_53_2d_i64(const exr_allocator *a, int64_t *data,
                                        size_t width, size_t height,
                                        unsigned levels) {
    unsigned level;
    int use_simd = 0;
    if (!a) a = exr_default_allocator();
    if (!data && width && height) return EXR_ERROR_INVALID_ARGUMENT;
    if (levels > 32) return EXR_ERROR_INVALID_ARGUMENT;
    if (width == 0 || height == 0 || levels == 0) return EXR_SUCCESS;
#if defined(EXR_X86)
    use_simd = (exr_cpu_caps() & EXR_SIMD_AVX2) != 0;
#elif defined(EXR_NEON)
    use_simd = 1;
#endif

    for (level = levels; level > 0; --level) {
        size_t rw = jph_ceil_div_pow2_size(width, level - 1u);
        size_t rh = jph_ceil_div_pow2_size(height, level - 1u);
        size_t lw = (rw + 1u) / 2u, hw = rw / 2u;
        size_t lh = (rh + 1u) / 2u, hh = rh / 2u;
        size_t temp_count, temp_bytes, scratch_len, scratch_bytes;
        int64_t *temp = NULL, *ev = NULL, *od = NULL;
        size_t y;
        exr_result rc = EXR_SUCCESS;

        if (rw == 0 || rh == 0) return EXR_ERROR_CORRUPT;
        if (exr_mul_ovf(rw, rh, &temp_count) ||
            exr_mul_ovf(temp_count, sizeof(int64_t), &temp_bytes))
            return EXR_ERROR_CORRUPT;
        scratch_len = rw > rh ? rw : rh;
        if (exr_mul_ovf(scratch_len, sizeof(int64_t), &scratch_bytes))
            return EXR_ERROR_CORRUPT;

        temp = (int64_t *)exr_malloc(a, temp_bytes);
        if (use_simd) {
            ev = (int64_t *)exr_malloc(a, scratch_bytes);
            od = (int64_t *)exr_malloc(a, scratch_bytes);
        }
        if (!temp || (use_simd && (!ev || !od))) {
            exr_free(a, temp); exr_free(a, ev); exr_free(a, od);
            return EXR_ERROR_OUT_OF_MEMORY;
        }

        /* Horizontal (row) pass: each row is contiguous low|high. */
        for (y = 0; y < rh; ++y) {
            const int64_t *row = data + y * width;
#if defined(EXR_X86)
            if (use_simd)
                rc = jph_inverse_53_i64_avx2(row, lw, row + lw, hw,
                                             temp + y * rw, rw, ev, od);
            else
#elif defined(EXR_NEON)
            if (use_simd)
                rc = jph_inverse_53_i64_neon(row, lw, row + lw, hw,
                                             temp + y * rw, rw, ev, od);
            else
#endif
                rc = jph_inverse_53_i64(row, lw, row + lw, hw, temp + y * rw, rw);
            if (rc != EXR_SUCCESS) goto done_level64;
        }
        /* Vertical (column) pass, row-wise across all columns -- no gather. */
#if defined(EXR_X86)
        if (use_simd)
            rc = jph_inverse_53_vert_i64_avx2(temp, rw, lh, hh, data, width);
        else
#elif defined(EXR_NEON)
        if (use_simd)
            rc = jph_inverse_53_vert_i64_neon(temp, rw, lh, hh, data, width);
        else
#endif
            rc = jph_inverse_53_vert_i64(temp, rw, lh, hh, data, width);

done_level64:
        exr_free(a, temp);
        exr_free(a, ev);
        exr_free(a, od);
        if (rc != EXR_SUCCESS) return rc;
    }
    return EXR_SUCCESS;
}

static exr_result jph_inverse_rct_i64(int64_t *c0, int64_t *c1, int64_t *c2,
                                      size_t count) {
    size_t i;
    if ((!c0 || !c1 || !c2) && count) return EXR_ERROR_INVALID_ARGUMENT;
    for (i = 0; i < count; ++i) {
        int64_t y = c0[i];
        int64_t db = c1[i];
        int64_t dr = c2[i];
        int64_t g = y - jph_floor_div_pow2(db + dr, 2);
        c0[i] = dr + g; /* r */
        c1[i] = g;      /* g */
        c2[i] = db + g; /* b */
    }
    return EXR_SUCCESS;
}

/* Scalar reference (source of truth) for the NLT type-3 involution. */
void jph_nlt_type3_i64_scalar(int64_t *data, size_t count, int64_t bias) {
    size_t i;
    for (i = 0; i < count; ++i) {
        if (data[i] < 0) data[i] = -data[i] - bias;
    }
}

static exr_result jph_apply_nlt_type3_i64(int64_t *data, size_t count,
                                          uint32_t bit_depth) {
    int64_t bias;
    if (!data && count) return EXR_ERROR_INVALID_ARGUMENT;
    if (bit_depth == 0 || bit_depth > 32u) return EXR_ERROR_INVALID_ARGUMENT;
    bias = ((int64_t)1 << (bit_depth - 1u)) + 1;
#if defined(EXR_X86)
    {
        uint32_t caps = exr_cpu_caps();
        if (caps & EXR_SIMD_AVX2) {
            jph_nlt_type3_i64_avx2(data, count, bias);
            return EXR_SUCCESS;
        }
        if (caps & EXR_SIMD_SSE2) {
            jph_nlt_type3_i64_sse2(data, count, bias);
            return EXR_SUCCESS;
        }
    }
#elif defined(EXR_NEON)
    jph_nlt_type3_i64_neon(data, count, bias);
    return EXR_SUCCESS;
#endif
    jph_nlt_type3_i64_scalar(data, count, bias);
    return EXR_SUCCESS;
}

static exr_result jph_validate_siz_component(const exr_codec_ctx *ctx,
                                             const JphProfile *jp,
                                             const uint16_t *map,
                                             uint16_t c) {
    const exr_channel *ch;
    uint16_t file_c = map[c];
    uint32_t bit_depth = (uint32_t)(jp->ssiz[c] & 0x7fu) + 1u;
    int is_signed = (jp->ssiz[c] & 0x80u) != 0;
    uint32_t want_bits, want_signed, recon_w, recon_h;

    if (file_c >= (uint16_t)ctx->num_channels) return EXR_ERROR_CORRUPT;
    ch = &ctx->channels[file_c];
    want_bits = (ch->pixel_type == EXR_PIXEL_HALF) ? 16u : 32u;
    want_signed = (ch->pixel_type == EXR_PIXEL_UINT) ? 0u : 1u;
    if (bit_depth != want_bits || (uint32_t)is_signed != want_signed)
        return EXR_ERROR_UNSUPPORTED;
    if (jp->xrsiz[c] == 0 || jp->yrsiz[c] == 0) return EXR_ERROR_CORRUPT;
    if ((int32_t)jp->xrsiz[c] != ch->x_sampling ||
        (int32_t)jp->yrsiz[c] != ch->y_sampling)
        return EXR_ERROR_UNSUPPORTED;

    recon_w = jph_divceil_u32(jp->xsiz, jp->xrsiz[c]) -
              jph_divceil_u32(jp->xosiz, jp->xrsiz[c]);
    recon_h = jph_divceil_u32(jp->ysiz, jp->yrsiz[c]) -
              jph_divceil_u32(jp->yosiz, jp->yrsiz[c]);
    {
        int32_t chunk_w = exr_num_samples(ctx->x,
                                           ctx->x + ctx->width - 1,
                                           ch->x_sampling);
        int32_t chunk_h = exr_num_samples(ctx->y,
                                           ctx->y + ctx->num_lines - 1,
                                           ch->y_sampling);
        if (chunk_w < 0 || chunk_h < 0) return EXR_ERROR_CORRUPT;
        if ((int32_t)recon_w != chunk_w || (int32_t)recon_h != chunk_h)
            return EXR_ERROR_CORRUPT;
    }
    return EXR_SUCCESS;
}

static exr_result jph_parse_siz(const exr_allocator *a, JphReader *r,
                                JphProfile *jp) {
    uint16_t len, csiz;
    size_t i, remain;

    if (!jph_read_be16(r, &len)) return EXR_ERROR_CORRUPT;
    if (len < 38) return EXR_ERROR_CORRUPT;
    if ((size_t)(r->end - r->p) < (size_t)(len - 2)) return EXR_ERROR_CORRUPT;
    remain = (size_t)(len - 2);

    if (remain < 36) return EXR_ERROR_CORRUPT;
    jp->rsiz = jph_be16(r->p); r->p += 2;
    jp->xsiz = jph_be32(r->p); r->p += 4;
    jp->ysiz = jph_be32(r->p); r->p += 4;
    jp->xosiz = jph_be32(r->p); r->p += 4;
    jp->yosiz = jph_be32(r->p); r->p += 4;
    jp->xtsiz = jph_be32(r->p); r->p += 4;
    jp->ytsiz = jph_be32(r->p); r->p += 4;
    jp->xtosiz = jph_be32(r->p); r->p += 4;
    jp->ytosiz = jph_be32(r->p); r->p += 4;
    csiz = jph_be16(r->p); r->p += 2;
    remain -= 36;

    if (csiz == 0) return EXR_ERROR_CORRUPT;
    if (remain != (size_t)csiz * 3u) return EXR_ERROR_CORRUPT;
    jp->ssiz = (uint8_t *)exr_malloc(a, csiz ? csiz : 1);
    jp->xrsiz = (uint8_t *)exr_malloc(a, csiz ? csiz : 1);
    jp->yrsiz = (uint8_t *)exr_malloc(a, csiz ? csiz : 1);
    jp->nlt_type = (uint8_t *)exr_calloc(a, csiz ? csiz : 1, 1);
    jp->qcc_exp = (uint8_t (*)[16])exr_calloc(a, csiz ? csiz : 1, 16);
    jp->qcc_count = (uint8_t *)exr_calloc(a, csiz ? csiz : 1, 1);
    jp->qcc_guard_bits = (uint8_t *)exr_calloc(a, csiz ? csiz : 1, 1);
    if (!jp->ssiz || !jp->xrsiz || !jp->yrsiz || !jp->nlt_type ||
        !jp->qcc_exp || !jp->qcc_count || !jp->qcc_guard_bits)
        return EXR_ERROR_OUT_OF_MEMORY;
    for (i = 0; i < csiz; ++i) {
        jp->ssiz[i] = r->p[0];
        jp->xrsiz[i] = r->p[1];
        jp->yrsiz[i] = r->p[2];
        r->p += 3;
    }
    jp->csiz = csiz;
    jp->saw_siz = 1;
    return EXR_SUCCESS;
}

static exr_result jph_parse_cap(JphReader *r, JphProfile *jp) {
    uint16_t len;
    uint32_t pcap;
    if (!jph_read_be16(r, &len)) return EXR_ERROR_CORRUPT;
    if (len < 6 || ((len - 6u) & 1u)) return EXR_ERROR_CORRUPT;
    if ((size_t)(r->end - r->p) < (size_t)(len - 2)) return EXR_ERROR_CORRUPT;
    pcap = jph_be32(r->p);
    if ((pcap & 0x00020000u) == 0) return EXR_ERROR_UNSUPPORTED;
    r->p += (size_t)(len - 2);
    jp->saw_cap = 1;
    return EXR_SUCCESS;
}

static exr_result jph_parse_cod(JphReader *r, JphProfile *jp) {
    uint16_t len, layers;
    uint8_t scod, prog, mc, ndecomp, bw, bh, style, wavelet;

    if (!jph_read_be16(r, &len)) return EXR_ERROR_CORRUPT;
    if ((size_t)(r->end - r->p) < (size_t)(len - 2)) return EXR_ERROR_CORRUPT;
    if (len < 12) return EXR_ERROR_CORRUPT;
    scod = r->p[0];
    prog = r->p[1];
    layers = jph_be16(r->p + 2);
    mc = r->p[4];
    ndecomp = r->p[5];
    bw = r->p[6];
    bh = r->p[7];
    style = r->p[8];
    wavelet = r->p[9];
    r->p += 10;
    if (len != (uint16_t)(12u + ((scod & 1u) ? (1u + ndecomp) : 0u)))
        return EXR_ERROR_CORRUPT;
    if (scod != 0u) return EXR_ERROR_UNSUPPORTED;
    if (prog != 2u || layers != 1u) return EXR_ERROR_UNSUPPORTED;
    if (!(mc == 0u || mc == 1u)) return EXR_ERROR_UNSUPPORTED;
    if (ndecomp != 5u || bw != 5u || bh != 3u) return EXR_ERROR_UNSUPPORTED;
    if (style != 0x40u || wavelet != 1u) return EXR_ERROR_UNSUPPORTED;
    jp->num_decomps = ndecomp;
    jp->mc_trans = mc;
    jp->saw_cod = 1;
    return EXR_SUCCESS;
}

static exr_result jph_parse_quant(JphReader *r, JphProfile *jp, int is_qcc) {
    uint16_t len, comp_idx = 0;
    size_t payload, comp_bytes = 0, pos;
    uint8_t sq;

    if (!jph_read_be16(r, &len)) return EXR_ERROR_CORRUPT;
    if (len < (is_qcc ? 4u : 3u)) return EXR_ERROR_CORRUPT;
    if ((size_t)(r->end - r->p) < (size_t)(len - 2)) return EXR_ERROR_CORRUPT;
    payload = (size_t)(len - 2);
    if (is_qcc) {
        if (!jp->saw_siz) return EXR_ERROR_CORRUPT;
        comp_bytes = (jp->csiz < 257u) ? 1u : 2u;
        if (payload < comp_bytes + 1u) return EXR_ERROR_CORRUPT;
        comp_idx = comp_bytes == 1u ? (uint16_t)r->p[0] : jph_be16(r->p);
        if (comp_idx >= jp->csiz) return EXR_ERROR_CORRUPT;
        r->p += comp_bytes;
        payload -= comp_bytes;
        jp->saw_qcc = 1;
    }
    sq = r->p[0];
    r->p++;
    payload--;
    if ((sq & 0x1fu) != 0u) return EXR_ERROR_UNSUPPORTED;
    if ((sq >> 5) > 7u) return EXR_ERROR_CORRUPT;
    pos = 0;
    while (pos < payload) {
        uint8_t sp = r->p[pos];
        uint8_t expn = (uint8_t)(sp >> 3);
        if (expn == 0) return EXR_ERROR_CORRUPT;
        pos++;
    }
    if (payload == 0 || payload > sizeof(jp->qcd_exp))
        return EXR_ERROR_CORRUPT;
    if (is_qcc) {
        if (!jp->qcc_exp || !jp->qcc_count || !jp->qcc_guard_bits)
            return EXR_ERROR_CORRUPT;
        memcpy(jp->qcc_exp[comp_idx], r->p, payload);
        jp->qcc_count[comp_idx] = (uint8_t)payload;
        jp->qcc_guard_bits[comp_idx] = (uint8_t)(sq >> 5);
    } else {
        memcpy(jp->qcd_exp, r->p, payload);
        jp->qcd_count = (uint8_t)payload;
        jp->qcd_guard_bits = (uint8_t)(sq >> 5);
    }
    r->p += payload;
    return EXR_SUCCESS;
}

static exr_result jph_parse_nlt(JphReader *r, const JphProfile *jp) {
    uint16_t len, comp;
    uint8_t bd, type;
    uint32_t bit_depth;
    int is_signed;
    uint16_t i;
    if (!jph_read_be16(r, &len)) return EXR_ERROR_CORRUPT;
    if (len != 6u) return EXR_ERROR_UNSUPPORTED;
    if ((size_t)(r->end - r->p) < 4) return EXR_ERROR_CORRUPT;
    comp = jph_be16(r->p);
    bd = r->p[2];
    type = r->p[3];
    r->p += 4;
    if (!(type == 0u || type == 3u)) return EXR_ERROR_UNSUPPORTED;
    if (comp != 0xffffu && (!jp->saw_siz || comp >= jp->csiz))
        return EXR_ERROR_CORRUPT;
    if (!jp->saw_siz || !jp->nlt_type) return EXR_ERROR_CORRUPT;
    bit_depth = (uint32_t)(bd & 0x7fu) + 1u;
    is_signed = (bd & 0x80u) != 0;
    if (comp == 0xffffu) {
        for (i = 0; i < jp->csiz; ++i) {
            uint32_t siz_bits = (uint32_t)(jp->ssiz[i] & 0x7fu) + 1u;
            int siz_signed = (jp->ssiz[i] & 0x80u) != 0;
            if (bit_depth != siz_bits || is_signed != siz_signed)
                return EXR_ERROR_UNSUPPORTED;
            if (type == 3u && !is_signed) return EXR_ERROR_UNSUPPORTED;
            jp->nlt_type[i] = type;
        }
    } else {
        uint32_t siz_bits = (uint32_t)(jp->ssiz[comp] & 0x7fu) + 1u;
        int siz_signed = (jp->ssiz[comp] & 0x80u) != 0;
        if (bit_depth != siz_bits || is_signed != siz_signed)
            return EXR_ERROR_UNSUPPORTED;
        if (type == 3u && !is_signed) return EXR_ERROR_UNSUPPORTED;
        jp->nlt_type[comp] = type;
    }
    return EXR_SUCCESS;
}

static exr_result jph_parse_sot(JphReader *r, JphProfile *jp) {
    uint16_t len, isot;
    uint32_t psot;
    uint8_t tpsot, tnsot;
    if (!jph_read_be16(r, &len)) return EXR_ERROR_CORRUPT;
    if (len != 10u) return EXR_ERROR_CORRUPT;
    if (!jph_read_be16(r, &isot) || !jph_read_be32(r, &psot) ||
        !jph_read_u8(r, &tpsot) || !jph_read_u8(r, &tnsot))
        return EXR_ERROR_CORRUPT;
    if (isot != 0u || tpsot != 0u || tnsot > 1u) return EXR_ERROR_UNSUPPORTED;
    if (psot != 0u && psot < 14u) return EXR_ERROR_CORRUPT;
    jp->psot = psot;
    jp->saw_sot = 1;
    return EXR_SUCCESS;
}

static exr_result jph_finish_tile_payload(JphReader *r, JphProfile *jp) {
    const uint8_t *payload_start = r->p;
    const uint8_t *payload_end = NULL;
    JphReader er;
    uint16_t marker = 0u;

    if (jp->psot != 0u) {
        const uint8_t *tile_end;
        if (!jp->sot_start) return EXR_ERROR_CORRUPT;
        if ((size_t)(r->end - jp->sot_start) < (size_t)jp->psot)
            return EXR_ERROR_CORRUPT;
        tile_end = jp->sot_start + jp->psot;
        if (tile_end < payload_start) return EXR_ERROR_CORRUPT;
        jp->tile_data = payload_start;
        jp->tile_data_size = (size_t)(tile_end - payload_start);
        r->p = tile_end;
        er = *r;
        if (jph_next_marker(&er, &marker) != EXR_SUCCESS)
            return EXR_ERROR_CORRUPT;
        if (marker != JPH_MARK_EOC) return EXR_ERROR_UNSUPPORTED;
        r->p = er.p;
    } else {
        exr_result rc;
        er = *r;
        rc = jph_find_eoc(&er, &payload_end);
        if (rc != EXR_SUCCESS) return rc;
        if (!payload_end || payload_end < payload_start) return EXR_ERROR_CORRUPT;
        jp->tile_data = payload_start;
        jp->tile_data_size = (size_t)(payload_end - payload_start);
        r->p = er.p;
    }
    if (jp->tile_data_size == 0) return EXR_ERROR_CORRUPT;
    return EXR_SUCCESS;
}

static exr_result jph_bitreader_terminate_packet(exr_jph_bitreader *br) {
    exr_jph_bitreader_align(br);
    if (br->prev_ff) {
        uint8_t byte;
        if (!br->p || br->p >= br->end) return EXR_ERROR_CORRUPT;
        byte = *br->p++;
        if (byte & 0x80u) return EXR_ERROR_CORRUPT;
        br->prev_ff = 0;
    }
    br->bits = 0;
    br->bit_count = 0;
    return EXR_SUCCESS;
}

static exr_result jph_codeblock_list_append(const exr_allocator *a,
                                            JphCodeblockList *list,
                                            JphCodeblockSeg seg) {
    JphCodeblockSeg *new_items;
    size_t new_cap, new_bytes, old_bytes;
    if (!a || !list) return EXR_ERROR_INVALID_ARGUMENT;
    if (list->count == list->cap) {
        if (list->cap > (SIZE_MAX / 2u)) return EXR_ERROR_CORRUPT;
        new_cap = list->cap ? list->cap * 2u : 16u;
        if (exr_mul_ovf(new_cap, sizeof(*new_items), &new_bytes))
            return EXR_ERROR_CORRUPT;
        new_items = (JphCodeblockSeg *)exr_malloc(a, new_bytes);
        if (!new_items) return EXR_ERROR_OUT_OF_MEMORY;
        if (list->items) {
            if (exr_mul_ovf(list->count, sizeof(*list->items), &old_bytes)) {
                exr_free(a, new_items);
                return EXR_ERROR_CORRUPT;
            }
            memcpy(new_items, list->items, old_bytes);
            exr_free(a, list->items);
        }
        list->items = new_items;
        list->cap = new_cap;
    }
    list->items[list->count++] = seg;
    return EXR_SUCCESS;
}

static exr_result jph_validate_forward_stuffing(const uint8_t *data,
                                                size_t size,
                                                uint8_t max_after_ff) {
    size_t i;
    int prev_ff = 0;
    if (!data && size) return EXR_ERROR_INVALID_ARGUMENT;
    for (i = 0; i < size; ++i) {
        if (prev_ff && data[i] > max_after_ff) return EXR_ERROR_CORRUPT;
        prev_ff = (data[i] == 0xffu);
    }
    return EXR_SUCCESS;
}

static exr_result jph_validate_reverse_stuffing(const uint8_t *data,
                                                size_t size,
                                                int initial_unstuff) {
    size_t i;
    int unstuff = initial_unstuff;
    if (!data && size) return EXR_ERROR_INVALID_ARGUMENT;
    for (i = size; i > 0; --i) {
        uint8_t byte = data[i - 1u];
        if (unstuff && ((byte & 0x7fu) == 0x7fu) && (byte & 0x80u))
            return EXR_ERROR_CORRUPT;
        unstuff = byte > 0x8fu;
    }
    return EXR_SUCCESS;
}

static exr_result jph_split_ht_codeblock_streams(const uint8_t *data,
                                                 size_t data_size,
                                                 uint32_t length0,
                                                 uint32_t length1,
                                                 JphCodeblockStreams *out) {
    size_t total, scup_start;
    uint32_t scup;
    exr_result rc;
    if (!out || (!data && data_size)) return EXR_ERROR_INVALID_ARGUMENT;
    memset(out, 0, sizeof(*out));
    if (length0 < 2u || length0 >= 65535u || length1 >= 2047u)
        return EXR_ERROR_CORRUPT;
    if (exr_add_ovf((size_t)length0, (size_t)length1, &total))
        return EXR_ERROR_CORRUPT;
    if (total != data_size) return EXR_ERROR_CORRUPT;

    scup = ((uint32_t)data[length0 - 1u] << 4u) |
           ((uint32_t)data[length0 - 2u] & 0x0fu);
    if (scup < 2u || scup > length0 || scup > 4079u)
        return EXR_ERROR_CORRUPT;

    scup_start = (size_t)length0 - (size_t)scup;
    out->magsgn = data;
    out->magsgn_size = scup_start;
    out->mel = data + scup_start;
    out->mel_size = (size_t)scup - 1u;
    out->vlc = data + scup_start + 1u;
    out->vlc_size = (size_t)scup - 2u;
    out->refine = data + length0;
    out->refine_size = length1;
    out->scup = scup;

    rc = jph_validate_forward_stuffing(out->magsgn, out->magsgn_size, 0x7fu);
    if (rc != EXR_SUCCESS) return rc;
    rc = jph_validate_forward_stuffing(out->mel, out->mel_size, 0x8fu);
    if (rc != EXR_SUCCESS) return rc;
    rc = jph_validate_reverse_stuffing(out->vlc, out->vlc_size,
                                       (data[length0 - 1u] | 0x0fu) > 0x8fu);
    if (rc != EXR_SUCCESS) return rc;
    if (out->refine_size) {
        rc = jph_validate_forward_stuffing(out->refine, out->refine_size,
                                           0x7fu);
        if (rc != EXR_SUCCESS) return rc;
        rc = jph_validate_reverse_stuffing(out->refine, out->refine_size, 1);
        if (rc != EXR_SUCCESS) return rc;
    }
    return EXR_SUCCESS;
}

static exr_result jph_validate_ht_codeblock_segments(const uint8_t *data,
                                                     size_t data_size,
                                                     uint32_t missing_msbs,
                                                     uint32_t active_passes,
                                                     uint32_t length0,
                                                     uint32_t length1,
                                                     uint32_t width,
                                                     uint32_t height) {
    JphCodeblockStreams streams;
    if (!data && data_size) return EXR_ERROR_INVALID_ARGUMENT;
    if (width == 0u || height == 0u || width > 128u || height > 32u)
        return EXR_ERROR_CORRUPT;
    if (active_passes == 0u || active_passes > 3u)
        return EXR_ERROR_CORRUPT;
    if (active_passes > 1u && length1 == 0u)
        return EXR_ERROR_CORRUPT;
    if (missing_msbs > 62u)
        return EXR_ERROR_CORRUPT;
    if (missing_msbs >= 30u && active_passes > 1u)
        return EXR_ERROR_CORRUPT;
    {
        exr_result rc = jph_split_ht_codeblock_streams(data, data_size,
                                                       length0, length1,
                                                       &streams);
        if (rc != EXR_SUCCESS) return rc;
    }

    if (missing_msbs > 30u) return EXR_SUCCESS;
    if (missing_msbs == 29u && active_passes > 1u) active_passes = 1u;
    (void)streams;
    return EXR_SUCCESS;
}

static uint32_t jph_band_quant_index(uint32_t res, uint32_t band) {
    return res ? (res - 1u) * 3u + band : 0u;
}

static exr_result jph_band_kmax(const JphProfile *jp, uint32_t comp,
                                uint32_t res, uint32_t band,
                                uint32_t *out_kmax) {
    uint32_t idx, expn, bits, count, guard;
    const uint8_t *exps;
    if (!jp || !out_kmax) return EXR_ERROR_CORRUPT;
    /* Per-component QCC overrides the default QCD when present. */
    if (jp->qcc_count && comp < (uint32_t)jp->csiz && jp->qcc_count[comp]) {
        exps = jp->qcc_exp[comp];
        count = jp->qcc_count[comp];
        guard = jp->qcc_guard_bits[comp];
    } else {
        exps = jp->qcd_exp;
        count = jp->qcd_count;
        guard = jp->qcd_guard_bits;
    }
    if (count == 0) return EXR_ERROR_CORRUPT;
    idx = jph_band_quant_index(res, band);
    if (idx >= count) idx = count - 1u;
    expn = (uint32_t)(exps[idx] >> 3);
    bits = expn ? expn - 1u : 0u;
    *out_kmax = bits + guard;
    return EXR_SUCCESS;
}

static JphSize jph_component_size(const JphProfile *jp, uint32_t comp) {
    JphSize s;
    s.w = jph_divceil_u32(jp->xsiz, jp->xrsiz[comp]) -
          jph_divceil_u32(jp->xosiz, jp->xrsiz[comp]);
    s.h = jph_divceil_u32(jp->ysiz, jp->yrsiz[comp]) -
          jph_divceil_u32(jp->yosiz, jp->yrsiz[comp]);
    return s;
}

static JphSize jph_resolution_size(JphSize comp_size, uint32_t num_decomps,
                                   uint32_t res) {
    uint32_t shift = num_decomps - res;
    JphSize s;
    s.w = jph_ceil_div_pow2_u32(comp_size.w, shift);
    s.h = jph_ceil_div_pow2_u32(comp_size.h, shift);
    return s;
}

static exr_result jph_build_band_geoms(const JphProfile *jp, uint32_t comp,
                                       uint32_t res, JphBandGeom bands[4],
                                       JphSize *out_res_size) {
    JphSize comp_size = jph_component_size(jp, comp);
    JphSize rs = jph_resolution_size(comp_size, jp->num_decomps, res);
    uint32_t b;
    memset(bands, 0, sizeof(JphBandGeom) * 4u);
    if (out_res_size) *out_res_size = rs;

    if (res == 0u) {
        bands[0].w = rs.w;
        bands[0].h = rs.h;
        bands[0].cb_w = 128u;
        bands[0].cb_h = 32u;
        bands[0].exists = (rs.w != 0u && rs.h != 0u);
        return jph_band_kmax(jp, comp, 0, 0, &bands[0].kmax);
    }

    bands[1].w = rs.w >> 1u;
    bands[1].h = (rs.h + 1u) >> 1u;
    bands[2].w = (rs.w + 1u) >> 1u;
    bands[2].h = rs.h >> 1u;
    bands[3].w = rs.w >> 1u;
    bands[3].h = rs.h >> 1u;
    for (b = 1; b < 4; ++b) {
        bands[b].cb_w = 128u;
        bands[b].cb_h = 32u;
        bands[b].exists = (bands[b].w != 0u && bands[b].h != 0u);
        if (bands[b].exists) {
            exr_result rc = jph_band_kmax(jp, comp, res, b, &bands[b].kmax);
            if (rc != EXR_SUCCESS) return rc;
        }
    }
    return EXR_SUCCESS;
}

static uint32_t jph_subband_project(uint32_t v, uint32_t offset) {
    if (v <= offset) return 0;
    return (v - offset + 1u) >> 1u;
}

static exr_result jph_parse_precinct_packet(const exr_allocator *a,
                                            const JphProfile *jp,
                                            uint32_t comp, uint32_t res,
                                            uint32_t precinct_x,
                                            uint32_t precinct_y,
                                            exr_jph_bitreader *br,
                                            size_t *out_codeblocks,
                                            JphCodeblockCallback cb,
                                            void *cb_user) {
    enum { JPH_PREC_LOG = 15 };
    JphBandGeom bands[4];
    JphSize rs;
    JphCodeblockList codeblocks;
    int saw_nonempty_band = 0;
    uint32_t b;
    exr_result rc;

    if (!a) a = exr_default_allocator();
    if (out_codeblocks) *out_codeblocks = 0;
    memset(&codeblocks, 0, sizeof(codeblocks));
    rc = jph_build_band_geoms(jp, comp, res, bands, &rs);
    if (rc != EXR_SUCCESS) return rc;

    for (b = 0; b < 4; ++b) {
        uint32_t p0x, p1x, p0y, p1y, bx0, bx1, by0, by1;
        uint32_t cbx0, cbx1, cby0, cby1, cbw, cbh, x, y;
        exr_jph_tag_tree inc_tree, mmsb_tree;
        int trees_ready = 0;

        if (!bands[b].exists) continue;
        p0x = precinct_x << JPH_PREC_LOG;
        p0y = precinct_y << JPH_PREC_LOG;
        p1x = p0x + (1u << JPH_PREC_LOG);
        p1y = p0y + (1u << JPH_PREC_LOG);
        if (p0x > rs.w) p0x = rs.w;
        if (p0y > rs.h) p0y = rs.h;
        if (p1x > rs.w) p1x = rs.w;
        if (p1y > rs.h) p1y = rs.h;

        if (res == 0u) {
            bx0 = p0x;
            bx1 = p1x;
            by0 = p0y;
            by1 = p1y;
        } else {
            bx0 = jph_subband_project(p0x, b & 1u);
            bx1 = jph_subband_project(p1x, b & 1u);
            by0 = jph_subband_project(p0y, b >> 1u);
            by1 = jph_subband_project(p1y, b >> 1u);
        }
        if (bx1 > bands[b].w) bx1 = bands[b].w;
        if (by1 > bands[b].h) by1 = bands[b].h;
        if (bx1 <= bx0 || by1 <= by0) continue;

        cbx0 = bx0 / bands[b].cb_w;
        cbx1 = jph_divceil_u32(bx1, bands[b].cb_w);
        cby0 = by0 / bands[b].cb_h;
        cby1 = jph_divceil_u32(by1, bands[b].cb_h);
        cbw = cbx1 - cbx0;
        cbh = cby1 - cby0;
        if (cbw == 0u || cbh == 0u) continue;

        if (!saw_nonempty_band) {
            uint32_t packet_present = 0;
            rc = exr_jph_bitreader_read(br, 1, &packet_present);
            if (rc != EXR_SUCCESS) return rc;
            if (!packet_present) return jph_bitreader_terminate_packet(br);
            saw_nonempty_band = 1;
        }

        memset(&inc_tree, 0, sizeof(inc_tree));
        memset(&mmsb_tree, 0, sizeof(mmsb_tree));
        rc = exr_jph_tag_tree_init(a, &inc_tree, cbw, cbh);
        if (rc != EXR_SUCCESS) goto band_done;
        rc = exr_jph_tag_tree_init(a, &mmsb_tree, cbw, cbh);
        if (rc != EXR_SUCCESS) goto band_done;
        trees_ready = 1;

        for (y = 0; y < cbh; ++y) {
            for (x = 0; x < cbw; ++x) {
                uint32_t inc = 0, missing = 0, raw = 0, active = 0, groups = 0;
                uint32_t lengths[2] = {0, 0};
                uint32_t cb_global_x, cb_global_y;
                uint64_t cb_px0, cb_px1, cb_py0, cb_py1;
                JphCodeblockSeg seg;

                rc = exr_jph_tag_tree_decode(&inc_tree, br, x, y, 1, &inc);
                if (rc != EXR_SUCCESS) goto band_done;
                if (inc >= 1u) continue;

                rc = exr_jph_tag_tree_decode(&mmsb_tree, br, x, y,
                                             bands[b].kmax + 1u, &missing);
                if (rc != EXR_SUCCESS) goto band_done;
                if (missing > bands[b].kmax) {
                    rc = EXR_ERROR_CORRUPT;
                    goto band_done;
                }
                rc = exr_jph_packet_read_pass_count(br, &raw, &active, &groups);
                (void)raw;
                if (rc != EXR_SUCCESS) goto band_done;
                if (groups > bands[b].kmax - missing) {
                    rc = EXR_ERROR_CORRUPT;
                    goto band_done;
                }
                rc = exr_jph_packet_read_pass_lengths(br, active, groups,
                                                      lengths);
                if (rc != EXR_SUCCESS) goto band_done;

                cb_global_x = cbx0 + x;
                cb_global_y = cby0 + y;
                cb_px0 = (uint64_t)cb_global_x * bands[b].cb_w;
                cb_py0 = (uint64_t)cb_global_y * bands[b].cb_h;
                cb_px1 = cb_px0 + bands[b].cb_w;
                cb_py1 = cb_py0 + bands[b].cb_h;
                if (cb_px0 >= bands[b].w || cb_py0 >= bands[b].h) {
                    rc = EXR_ERROR_CORRUPT;
                    goto band_done;
                }
                if (cb_px1 > bands[b].w) cb_px1 = bands[b].w;
                if (cb_py1 > bands[b].h) cb_py1 = bands[b].h;
                if (cb_px1 <= cb_px0 || cb_py1 <= cb_py0 ||
                    cb_px1 - cb_px0 > UINT32_MAX ||
                    cb_py1 - cb_py0 > UINT32_MAX) {
                    rc = EXR_ERROR_CORRUPT;
                    goto band_done;
                }
                seg.missing_msbs = missing;
                seg.active_passes = active;
                seg.length0 = lengths[0];
                seg.length1 = lengths[1];
                seg.comp = comp;
                seg.res = res;
                seg.band = b;
                seg.x0 = (uint32_t)cb_px0;
                seg.y0 = (uint32_t)cb_py0;
                seg.width = (uint32_t)(cb_px1 - cb_px0);

                seg.height = (uint32_t)(cb_py1 - cb_py0);
                seg.data = NULL;
                seg.data_size = 0;
                rc = jph_codeblock_list_append(a, &codeblocks, seg);
                if (rc != EXR_SUCCESS) goto band_done;
            }
        }

band_done:
        if (trees_ready) {
            exr_jph_tag_tree_free(a, &inc_tree);
            exr_jph_tag_tree_free(a, &mmsb_tree);
        } else {
            exr_jph_tag_tree_free(a, &inc_tree);
        }
        if (rc != EXR_SUCCESS) goto done;
    }

    if (!saw_nonempty_band) {
        uint32_t bit = 0;
        rc = exr_jph_bitreader_read(br, 1, &bit);
        if (rc != EXR_SUCCESS) goto done;
    }
    rc = jph_bitreader_terminate_packet(br);
    if (rc != EXR_SUCCESS) goto done;
    for (b = 0; b < codeblocks.count; ++b) {
        size_t total;
        JphCodeblockSeg *seg = &codeblocks.items[b];
        if (exr_add_ovf((size_t)seg->length0, (size_t)seg->length1, &total)) {
            rc = EXR_ERROR_CORRUPT;
            goto done;
        }
        if ((size_t)(br->end - br->p) < total) {
            rc = EXR_ERROR_CORRUPT;
            goto done;
        }
        seg->data = br->p;
        seg->data_size = total;
        rc = jph_validate_ht_codeblock_segments(br->p, total,
                                                seg->missing_msbs,
                                                seg->active_passes,
                                                seg->length0, seg->length1,
                                                seg->width, seg->height);
        if (rc != EXR_SUCCESS) goto done;
        if (cb) {
            rc = cb(cb_user, seg);
            if (rc != EXR_SUCCESS) goto done;
        }
        br->p += total;
    }
    rc = EXR_SUCCESS;
    if (out_codeblocks) *out_codeblocks = codeblocks.count;

done:
    exr_free(a, codeblocks.items);
    return rc;
}

static exr_result jph_init_precinct_states(const JphProfile *jp,
                                           JphPrecinctState *states) {
    enum { JPH_PREC_LOG = 15 };
    uint32_t c, r;
    if (!jp || !states) return EXR_ERROR_INVALID_ARGUMENT;
    for (c = 0; c < jp->csiz; ++c) {
        JphSize comp_size = jph_component_size(jp, c);
        for (r = 0; r <= jp->num_decomps; ++r) {
            JphSize rs = jph_resolution_size(comp_size, jp->num_decomps, r);
            JphPrecinctState *st = states + (size_t)c * (jp->num_decomps + 1u) + r;
            st->num_w = jph_divceil_u32(rs.w, 1u << JPH_PREC_LOG);
            st->num_h = jph_divceil_u32(rs.h, 1u << JPH_PREC_LOG);
            st->cur_x = 0;
            st->cur_y = 0;
        }
    }
    return EXR_SUCCESS;
}

static int jph_precinct_top_left(const JphProfile *jp, uint32_t comp,
                                 uint32_t res, const JphPrecinctState *st,
                                 uint64_t *out_x, uint64_t *out_y) {
    enum { JPH_PREC_LOG = 15 };
    uint64_t down_x, down_y;
    if (!st || st->cur_y >= st->num_h || st->cur_x >= st->num_w) return 0;
    down_x = (uint64_t)jp->xrsiz[comp] << (jp->num_decomps - res);
    down_y = (uint64_t)jp->yrsiz[comp] << (jp->num_decomps - res);
    *out_x = down_x * ((uint64_t)st->cur_x << JPH_PREC_LOG);
    *out_y = down_y * ((uint64_t)st->cur_y << JPH_PREC_LOG);
    return 1;
}

static void jph_advance_precinct(JphPrecinctState *st) {
    if (++st->cur_x >= st->num_w) {
        st->cur_x = 0;
        st->cur_y++;
    }
}

typedef struct {
    const exr_allocator *a;
    const JphProfile *jp;
    JphPlaneD *planes;
    uint16_t num_planes;
    int use_i32; /* all components HALF -> int32 coefficient planes */
    int use_avx2;
    size_t codeblocks;
    JphDecodeWorkspace ws;
} JphDecodeState;

static void jph_decode_workspace_free(const exr_allocator *a,
                                      JphDecodeWorkspace *ws) {
    if (!a) a = exr_default_allocator();
    if (!ws) return;
    exr_free(a, ws->scratch);
    exr_free(a, ws->v_n32);
    exr_free(a, ws->buf32);
    exr_free(a, ws->inv53_temp32);
    exr_free(a, ws->inv53_ev64);
    exr_free(a, ws->inv53_od64);
    exr_free(a, ws->magsgn_bits);
    exr_free(a, ws->sigprop_bits);
    memset(ws, 0, sizeof(*ws));
}

static exr_result jph_ws_reserve_raw(const exr_allocator *a, void **ptr,
                                     size_t *cap, size_t count,
                                     size_t elem_size) {
    void *p;
    size_t bytes;
    if (count <= *cap) return EXR_SUCCESS;
    if (exr_mul_ovf(count, elem_size, &bytes)) return EXR_ERROR_CORRUPT;
    p = exr_malloc(a, bytes);
    if (!p) return EXR_ERROR_OUT_OF_MEMORY;
    exr_free(a, *ptr);
    *ptr = p;
    *cap = count;
    return EXR_SUCCESS;
}

static void jph_free_component_planes(const exr_allocator *a, JphPlaneD *planes,
                                      uint16_t num_planes) {
    uint16_t c;
    if (!a) a = exr_default_allocator();
    if (!planes) return;
    for (c = 0; c < num_planes; ++c) {
        exr_free(a, planes[c].d32);
        exr_free(a, planes[c].d64);
    }
    exr_free(a, planes);
}

static exr_result jph_alloc_component_planes(const exr_allocator *a,
                                             const JphProfile *jp, int use_i32,
                                             JphPlaneD **out_planes) {
    JphPlaneD *planes;
    uint16_t c;
    size_t elem = use_i32 ? sizeof(int32_t) : sizeof(int64_t);
    if (!out_planes || !jp || jp->csiz == 0) return EXR_ERROR_INVALID_ARGUMENT;
    if (!a) a = exr_default_allocator();
    *out_planes = NULL;
    planes = (JphPlaneD *)exr_calloc(a, jp->csiz, sizeof(*planes));
    if (!planes) return EXR_ERROR_OUT_OF_MEMORY;
    for (c = 0; c < jp->csiz; ++c) {
        JphSize s = jph_component_size(jp, c);
        size_t count, bytes;
        void *buf;
        planes[c].w = s.w;
        planes[c].h = s.h;
        if (exr_mul_ovf((size_t)s.w, (size_t)s.h, &count) ||
            exr_mul_ovf(count, elem, &bytes)) {
            jph_free_component_planes(a, planes, jp->csiz);
            return EXR_ERROR_CORRUPT;
        }
        buf = exr_calloc(a, bytes ? bytes : 1u, 1);
        if (!buf) {
            jph_free_component_planes(a, planes, jp->csiz);
            return EXR_ERROR_OUT_OF_MEMORY;
        }
        if (use_i32) planes[c].d32 = (int32_t *)buf;
        else planes[c].d64 = (int64_t *)buf;
    }
    *out_planes = planes;
    return EXR_SUCCESS;
}

static exr_result jph_find_component_for_channel(const JphProfile *jp,
                                                 const uint16_t *map,
                                                 uint16_t channel,
                                                 uint16_t *out_comp) {
    uint16_t c;
    if (!jp || !map || !out_comp) return EXR_ERROR_INVALID_ARGUMENT;
    for (c = 0; c < jp->csiz; ++c) {
        if (map[c] == channel) {
            *out_comp = c;
            return EXR_SUCCESS;
        }
    }
    return EXR_ERROR_CORRUPT;
}

static exr_result jph_store_sample(uint8_t *dst, size_t dst_size, size_t *off,
                                   exr_pixel_type pixel_type, int64_t v) {
    if (!dst || !off) return EXR_ERROR_INVALID_ARGUMENT;
    if (pixel_type == EXR_PIXEL_HALF) {
        uint16_t bits;
        if (*off > dst_size || dst_size - *off < 2u) return EXR_ERROR_CORRUPT;
        if (v < (int64_t)INT16_MIN || v > (int64_t)INT16_MAX) return EXR_ERROR_CORRUPT;
        bits = (uint16_t)(int32_t)(int16_t)v;
        dst[*off] = (uint8_t)bits;
        dst[*off + 1u] = (uint8_t)(bits >> 8);
        *off += 2u;
    } else if (pixel_type == EXR_PIXEL_UINT) {
        uint32_t bits;
        if (*off > dst_size || dst_size - *off < 4u) return EXR_ERROR_CORRUPT;
        if (v < 0) return EXR_ERROR_CORRUPT;
        if (v > (int64_t)UINT32_MAX) return EXR_ERROR_CORRUPT;
        bits = (uint32_t)v;
        exr_wr_u32(dst + *off, bits);
        *off += 4u;
    } else {
        uint32_t bits;
        if (*off > dst_size || dst_size - *off < 4u) return EXR_ERROR_CORRUPT;
        if (v < (int64_t)INT32_MIN || v > (int64_t)INT32_MAX) return EXR_ERROR_CORRUPT;
        bits = (uint32_t)(int32_t)v;
        exr_wr_u32(dst + *off, bits);
        *off += 4u;
    }
    return EXR_SUCCESS;
}

/* Scalar reference (source of truth) for the all-HALF int32->uint16 pack. */
void jph_pack_i32_to_half_scalar(uint8_t *dst, const int32_t *src, size_t n) {
    size_t i;
    for (i = 0; i < n; ++i) {
        uint16_t b = (uint16_t)src[i];
        dst[2u * i] = (uint8_t)b;
        dst[2u * i + 1u] = (uint8_t)(b >> 8);
    }
}

static exr_result jph_store_component_planes_to_block(
    const exr_codec_ctx *ctx, const JphProfile *jp, const uint16_t *map,
    const JphPlaneD *planes, uint8_t *dst, size_t dst_size) {
    size_t off = 0;
    int32_t line, file_c;
    void (*pack_i32_to_half)(uint8_t *, const int32_t *, size_t) =
        jph_pack_i32_to_half_scalar;
#if defined(EXR_X86)
    uint32_t caps = exr_cpu_caps();
    if (caps & EXR_SIMD_AVX2) pack_i32_to_half = jph_pack_i32_to_half_avx2;
    else if (caps & EXR_SIMD_SSE41) pack_i32_to_half = jph_pack_i32_to_half_sse41;
#elif defined(EXR_NEON)
    pack_i32_to_half = jph_pack_i32_to_half_neon;
#endif
    if (!ctx || !jp || !map || !planes || !dst) return EXR_ERROR_INVALID_ARGUMENT;
    for (line = 0; line < ctx->num_lines; ++line) {
        int32_t yy = ctx->y + line;
        for (file_c = 0; file_c < ctx->num_channels; ++file_c) {
            const exr_channel *ch = &ctx->channels[file_c];
            int32_t xs = ch->x_sampling <= 0 ? 1 : ch->x_sampling;
            int32_t ys = ch->y_sampling <= 0 ? 1 : ch->y_sampling;
            uint16_t comp = 0;
            int nx, x;
            int row_i;
            uint32_t row;
            exr_result rc;
            if ((yy % ys) != 0) continue;
            rc = jph_find_component_for_channel(jp, map, (uint16_t)file_c,
                                                &comp);
            if (rc != EXR_SUCCESS) return rc;
            nx = exr_num_samples(ctx->x, ctx->x + ctx->width - 1, xs);
            if (nx < 0) nx = 0;
            row_i = exr_num_samples(ctx->y, yy, ys) - 1;
            if (row_i < 0) return EXR_ERROR_CORRUPT;
            row = (uint32_t)row_i;
            if (row >= planes[comp].h || (uint32_t)nx > planes[comp].w)
                return EXR_ERROR_CORRUPT;
            /* Fast path: int32 plane is only used when every component is HALF,
             * so the whole contiguous row packs int32 -> uint16 (vectorised),
             * with a single bounds check instead of one per sample. */
            if (planes[comp].d32) {
                const int32_t *srow =
                    &planes[comp].d32[(size_t)row * planes[comp].w];
                size_t span = (size_t)(uint32_t)nx * 2u;
                if (off > dst_size || span > dst_size - off)
                    return EXR_ERROR_CORRUPT;
                pack_i32_to_half(dst + off, srow, (size_t)(uint32_t)nx);
                off += span;
                continue;
            }
            for (x = 0; x < nx; ++x) {
                size_t idx = (size_t)row * planes[comp].w + (uint32_t)x;
                int64_t sv = planes[comp].d64[idx];
                rc = jph_store_sample(dst, dst_size, &off, ch->pixel_type, sv);
                if (rc != EXR_SUCCESS) return rc;
            }
        }
    }
    return off == dst_size ? EXR_SUCCESS : EXR_ERROR_CORRUPT;
}

static exr_result jph_postprocess_component_planes(const exr_allocator *a,
                                                   const JphProfile *jp,
                                                   JphPlaneD *planes,
                                                   JphDecodeWorkspace *ws) {
    uint16_t c;
    if (!jp || !planes) return EXR_ERROR_INVALID_ARGUMENT;
    for (c = 0; c < jp->csiz; ++c) {
        exr_result rc;
        int bounded = 0;
        if (planes[c].d32)
        {
            unsigned bps = ((unsigned)jp->ssiz[c] & 0x7fu) + 1u;
            bounded = (bps <= 16u);
            rc = jph_inverse_53_2d_i32_ws(a, planes[c].d32, planes[c].w,
                                          planes[c].h, jp->num_decomps, ws,
                                          bounded);
        }
        else if (planes[c].d64)
            rc = jph_inverse_53_2d_i64(a, planes[c].d64, planes[c].w,
                                       planes[c].h, jp->num_decomps);
        else
            return EXR_ERROR_CORRUPT;
        if (rc != EXR_SUCCESS) return rc;
    }
    if (jp->mc_trans) {
        size_t count;
        exr_result rc;
        if (jp->csiz < 3u) return EXR_ERROR_CORRUPT;
        if (planes[0].w != planes[1].w || planes[0].w != planes[2].w ||
            planes[0].h != planes[1].h || planes[0].h != planes[2].h)
            return EXR_ERROR_CORRUPT;
        if (exr_mul_ovf((size_t)planes[0].w, (size_t)planes[0].h, &count))
            return EXR_ERROR_CORRUPT;
        if (planes[0].d32)
            rc = exr_jph_inverse_rct_i32(planes[0].d32, planes[1].d32,
                                         planes[2].d32, count);
        else
            rc = jph_inverse_rct_i64(planes[0].d64, planes[1].d64,
                                     planes[2].d64, count);
        if (rc != EXR_SUCCESS) return rc;
    }
    for (c = 0; c < jp->csiz; ++c) {
        uint32_t bit_depth = (uint32_t)(jp->ssiz[c] & 0x7fu) + 1u;
        if (jp->nlt_type && jp->nlt_type[c] == 3u) {
            size_t count;
            exr_result rc;
            if (exr_mul_ovf((size_t)planes[c].w, (size_t)planes[c].h, &count))
                return EXR_ERROR_CORRUPT;
            if (planes[c].d32)
                rc = exr_jph_apply_nlt_type3_i32(planes[c].d32, count,
                                                 bit_depth);
            else
                rc = jph_apply_nlt_type3_i64(planes[c].d64, count, bit_depth);
            if (rc != EXR_SUCCESS) return rc;
        } else if (jp->nlt_type && jp->nlt_type[c] != 0u) {
            return EXR_ERROR_CORRUPT;
        }
    }
    return EXR_SUCCESS;
}

/* ----------------------------------------------------------------------------
 * HT block entropy decoder.
 *
 * The HT block decoder implements the JPEG 2000 Part 15 (HTJ2K) tier-2
 * decoder, in particular the cleanup, significance propagation, and magnitude
 * refinement passes. The bulk of this section is a clean-room C11 port of
 * the corresponding decoders in OpenJPH (BSD-2-Clause). The OpenEXR profile
 * is restricted so a small, well-defined subset of the spec is needed; the
 * following features are intentionally not supported:
 *
 *   - > 3 coding passes
 *   - stripe-causal codeblock layout
 *   - non-reversible (irreversible) wavelet path
 *   - 64-bit precision codeblocks
 *   - non-uniform quantizer steps
 *
 * Tables are derived at runtime from the spec-derived init logic in
 * jph_vlc_init_tables() / jph_uvlc_init_tables() and are the same numerical
 * content as the OpenJPH table0.h / table1.h generators, just computed once
 * at first use.
 *
 * Portions of this section are derived from OpenJPH (Aous Naman, Kakadu
 * Software Pty Ltd, University of New South Wales), BSD-2-Clause. See
 * src/exr_jph.c license header.
 * ------------------------------------------------------------------------- */

#define JPH_VLC_TBL_SIZE 1024u
#define JPH_UVLC_TBL0_SIZE (256u + 64u)
#define JPH_UVLC_TBL1_SIZE 256u
#define JPH_QUAD_STRIDE_QUADS 8u
#define JPH_QUAD_MAX_PER_ROW 513u
#define JPH_V_N_SIZE 516u

typedef struct {
    uint16_t vlc_tbl0[JPH_VLC_TBL_SIZE];
    uint16_t vlc_tbl1[JPH_VLC_TBL_SIZE];
    uint16_t uvlc_tbl0[JPH_UVLC_TBL0_SIZE];
    uint16_t uvlc_tbl1[JPH_UVLC_TBL1_SIZE];
    uint8_t uvlc_bias[JPH_UVLC_TBL0_SIZE];
    int initialized;
} JphHtTables;

static JphHtTables g_jph_ht_tables;

static const uint8_t jph_uvlc_dec[8] = {
    (uint8_t)(3u | (5u << 2) | (5u << 5)),  /* 000 */
    (uint8_t)(1u | (0u << 2) | (1u << 5)),  /* 001 */
    (uint8_t)(2u | (0u << 2) | (2u << 5)),  /* 010 */
    (uint8_t)(1u | (0u << 2) | (1u << 5)),  /* 011 */
    (uint8_t)(3u | (1u << 2) | (3u << 5)),  /* 100 */
    (uint8_t)(1u | (0u << 2) | (1u << 5)),  /* 101 */
    (uint8_t)(2u | (0u << 2) | (2u << 5)),  /* 110 */
    (uint8_t)(1u | (0u << 2) | (1u << 5))   /* 111 */
};

static void jph_uvlc_init_tables_impl(JphHtTables *t) {
    uint32_t i;
    for (i = 0; i < JPH_UVLC_TBL0_SIZE; ++i) {
        uint32_t mode = i >> 6;
        uint32_t vlc = i & 0x3Fu;

        if (mode == 0u) {
            t->uvlc_tbl0[i] = 0;
            t->uvlc_bias[i] = 0;
        } else if (mode <= 2u) {
            uint32_t d = jph_uvlc_dec[vlc & 0x7u];
            uint32_t total_prefix = d & 0x3u;
            uint32_t total_suffix = (d >> 2) & 0x7u;
            uint32_t u0_suffix_len = (mode == 1u) ? total_suffix : 0u;
            uint32_t u0 = (mode == 1u) ? (d >> 5) : 0u;
            uint32_t u1 = (mode == 1u) ? 0u : (d >> 5);
            t->uvlc_tbl0[i] = (uint16_t)(total_prefix |
                                         (total_suffix << 3) |
                                         (u0_suffix_len << 7) |
                                         (u0 << 10) |
                                         (u1 << 13));
            t->uvlc_bias[i] = 0;
        } else if (mode == 3u) {
            uint32_t d0 = jph_uvlc_dec[vlc & 0x7u];
            vlc >>= d0 & 0x3u;
            uint32_t d1 = jph_uvlc_dec[vlc & 0x7u];

            uint32_t total_prefix, u0_suffix_len, total_suffix, u0, u1;
            if ((d0 & 0x3u) == 3u) {
                total_prefix = (d0 & 0x3u) + 1u;
                u0_suffix_len = (d0 >> 2) & 0x7u;
                total_suffix = u0_suffix_len;
                u0 = d0 >> 5;
                u1 = (vlc & 1u) + 1u;
                t->uvlc_bias[i] = 4;
            } else {
                total_prefix = (d0 & 0x3u) + (d1 & 0x3u);
                u0_suffix_len = (d0 >> 2) & 0x7u;
                total_suffix = u0_suffix_len + ((d1 >> 2) & 0x7u);
                u0 = d0 >> 5;
                u1 = d1 >> 5;
                t->uvlc_bias[i] = 0;
            }
            t->uvlc_tbl0[i] = (uint16_t)(total_prefix |
                                         (total_suffix << 3) |
                                         (u0_suffix_len << 7) |
                                         (u0 << 10) |
                                         (u1 << 13));
        } else {
            uint32_t d0 = jph_uvlc_dec[vlc & 0x7u];
            vlc >>= d0 & 0x3u;
            uint32_t d1 = jph_uvlc_dec[vlc & 0x7u];

            uint32_t total_prefix = (d0 & 0x3u) + (d1 & 0x3u);
            uint32_t u0_suffix_len = (d0 >> 2) & 0x7u;
            uint32_t total_suffix = u0_suffix_len + ((d1 >> 2) & 0x7u);
            uint32_t u0 = (d0 >> 5) + 2u;
            uint32_t u1 = (d1 >> 5) + 2u;
            t->uvlc_tbl0[i] = (uint16_t)(total_prefix |
                                         (total_suffix << 3) |
                                         (u0_suffix_len << 7) |
                                         (u0 << 10) |
                                         (u1 << 13));
            t->uvlc_bias[i] = 10;
        }
    }
    for (i = 0; i < JPH_UVLC_TBL1_SIZE; ++i) {
        uint32_t mode = i >> 6;
        uint32_t vlc = i & 0x3Fu;

        if (mode == 0u) {
            t->uvlc_tbl1[i] = 0;
        } else if (mode <= 2u) {
            uint32_t d = jph_uvlc_dec[vlc & 0x7u];
            uint32_t total_prefix = d & 0x3u;
            uint32_t total_suffix = (d >> 2) & 0x7u;
            uint32_t u0_suffix_len = (mode == 1u) ? total_suffix : 0u;
            uint32_t u0 = (mode == 1u) ? (d >> 5) : 0u;
            uint32_t u1 = (mode == 1u) ? 0u : (d >> 5);
            t->uvlc_tbl1[i] = (uint16_t)(total_prefix |
                                         (total_suffix << 3) |
                                         (u0_suffix_len << 7) |
                                         (u0 << 10) |
                                         (u1 << 13));
        } else {
            uint32_t d0 = jph_uvlc_dec[vlc & 0x7u];
            vlc >>= d0 & 0x3u;
            uint32_t d1 = jph_uvlc_dec[vlc & 0x7u];

            uint32_t total_prefix = (d0 & 0x3u) + (d1 & 0x3u);
            uint32_t u0_suffix_len = (d0 >> 2) & 0x7u;
            uint32_t total_suffix = u0_suffix_len + ((d1 >> 2) & 0x7u);
            uint32_t u0 = d0 >> 5;
            uint32_t u1 = d1 >> 5;
            t->uvlc_tbl1[i] = (uint16_t)(total_prefix |
                                         (total_suffix << 3) |
                                         (u0_suffix_len << 7) |
                                         (u0 << 10) |
                                         (u1 << 13));
        }
    }
}

/* The VLC table entries are derived from ITU-T T.814 Tables A.20 / A.21 and
 * the corresponding open source implementation in OpenJPH's table0.h and
 * table1.h. The numerical content of those tables is reproduced here in
 * compact, unambiguous form. Each row is:
 *   c_q  rho  u_off  e_k  e_1  cwd  cwd_len
 * 7-bit codeword (lsb-aligned) with the indicated length, mapped to the
 * (rho, u_off, e_k, e_1) tuple in the (c_q) context.
 */
static const uint8_t jph_vlc_src_table0[][7] = {
#include "exr_jph_table0.inc"
};

static const uint8_t jph_vlc_src_table1[][7] = {
#include "exr_jph_table1.inc"
};

static void jph_vlc_init_tables_impl(JphHtTables *t) {
    size_t i, j;
    size_t tbl0_size = sizeof(jph_vlc_src_table0) /
                       sizeof(jph_vlc_src_table0[0]);
    size_t tbl1_size = sizeof(jph_vlc_src_table1) /
                       sizeof(jph_vlc_src_table1[0]);
    for (i = 0; i < JPH_VLC_TBL_SIZE; ++i) {
        uint32_t cwd = (uint32_t)(i & 0x7Fu);
        uint32_t c_q = (uint32_t)(i >> 7);
        for (j = 0; j < tbl0_size; ++j) {
            const uint8_t *e = jph_vlc_src_table0[j];
            if (e[0] == c_q) {
                uint32_t mask = (1u << e[6]) - 1u;
                if (e[5] == (cwd & mask)) {
                    t->vlc_tbl0[i] = (uint16_t)((e[1] << 4) | (e[2] << 3) |
                                                (e[3] << 12) | (e[4] << 8) |
                                                e[6]);
                    break;
                }
            }
        }
        for (j = 0; j < tbl1_size; ++j) {
            const uint8_t *e = jph_vlc_src_table1[j];
            if (e[0] == c_q) {
                uint32_t mask = (1u << e[6]) - 1u;
                if (e[5] == (cwd & mask)) {
                    t->vlc_tbl1[i] = (uint16_t)((e[1] << 4) | (e[2] << 3) |
                                                (e[3] << 12) | (e[4] << 8) |
                                                e[6]);
                    break;
                }
            }
        }
    }
}

static exr_result jph_ht_ensure_tables(JphHtTables **out) {
    JphHtTables *t = &g_jph_ht_tables;
    if (!t->initialized) {
        memset(t, 0, sizeof(*t));
        jph_vlc_init_tables_impl(t);
        jph_uvlc_init_tables_impl(t);
        t->initialized = 1;
    }
    *out = t;
    return EXR_SUCCESS;
}

/* ----- VLC reverse reader (reads backward, unstuffing as it goes) ----- */
typedef struct {
    const uint8_t *data;
    const uint8_t *start;
    uint64_t tmp;
    uint32_t bits;
    uint32_t unstuff;
    uint32_t size; /* remaining bytes in VLC segment */
} JphVlcRev;

static exr_result jph_vlc_rev_read(JphVlcRev *v) {
    uint32_t val = 0;
    uint32_t out_bits = 0;
    uint32_t tmp = 0;
    uint32_t u = v->unstuff;
    if (!v || !v->data) return EXR_ERROR_INVALID_ARGUMENT;
    if (v->bits > 32u) return EXR_SUCCESS;

    if (v->size > 3u) {
        const uint8_t *p = v->data - 3;
        val = ((uint32_t)p[0]) | ((uint32_t)p[1] << 8) |
              ((uint32_t)p[2] << 16) | ((uint32_t)p[3] << 24);
        v->data -= 4;
        v->size -= 4;
    } else if (v->size > 0u) {
        uint32_t i = 24;
        while (v->size > 0u) {
            uint32_t b = *v->data--;
            val |= (b << i);
            v->size--;
            i -= 8;
        }
    }

    {
        uint8_t b = (uint8_t)(val >> 24);
        uint32_t nb = (u && ((b & 0x7Fu) == 0x7Fu)) ? 7u : 8u;
        u = (b > 0x8Fu) ? 1u : 0u;
        tmp = b;
        out_bits += nb;
    }
    {
        uint8_t b = (uint8_t)(val >> 16);
        uint32_t nb = (u && ((b & 0x7Fu) == 0x7Fu)) ? 7u : 8u;
        u = (b > 0x8Fu) ? 1u : 0u;
        tmp |= (uint32_t)b << out_bits;
        out_bits += nb;
    }
    {
        uint8_t b = (uint8_t)(val >> 8);
        uint32_t nb = (u && ((b & 0x7Fu) == 0x7Fu)) ? 7u : 8u;
        u = (b > 0x8Fu) ? 1u : 0u;
        tmp |= (uint32_t)b << out_bits;
        out_bits += nb;
    }
    {
        uint8_t b = (uint8_t)val;
        uint32_t nb = (u && ((b & 0x7Fu) == 0x7Fu)) ? 7u : 8u;
        v->unstuff = (b > 0x8Fu) ? 1u : 0u;
        tmp |= (uint32_t)b << out_bits;
        out_bits += nb;
    }

    v->tmp |= (uint64_t)tmp << v->bits;
    v->bits += out_bits;
    return EXR_SUCCESS;
}

static exr_result jph_vlc_rev_init(JphVlcRev *v, const uint8_t *buf,
                                   uint32_t lcup, uint32_t scup) {
    uint32_t i, num, tnum;
    if (!v || !buf) return EXR_ERROR_INVALID_ARGUMENT;
    if (scup < 2u || lcup < scup) return EXR_ERROR_CORRUPT;
    memset(v, 0, sizeof(*v));
    v->data = buf + lcup - 2u;
    v->start = buf;
    v->size = scup - 2u;
    {
        uint8_t d = *v->data--;
        v->tmp = (uint64_t)(d >> 4);
        v->bits = 4u - (((v->tmp & 7u) == 7u) ? 1u : 0u);
        v->unstuff = ((d | 0x0Fu) > 0x8Fu) ? 1u : 0u;
    }
    /* align to 4-byte boundary (matching OpenJPH's alignment logic) */
    num = 1u + ((uint32_t)(uintptr_t)(v->data) & 3u);
    tnum = num < v->size ? num : v->size;
    for (i = 0; i < tnum; ++i) {
        uint8_t d = *v->data--;
        uint32_t nb = 8u - ((v->unstuff && ((d & 0x7Fu) == 0x7Fu)) ? 1u : 0u);
        v->tmp |= (uint64_t)d << v->bits;
        v->bits += nb;
        v->unstuff = (d > 0x8Fu) ? 1u : 0u;
    }
    v->size -= tnum;
    if (v->bits <= 32u) {
        exr_result rc = jph_vlc_rev_read(v);
        if (rc != EXR_SUCCESS) return rc;
    }
    return EXR_SUCCESS;
}

static uint32_t jph_vlc_rev_fetch(JphVlcRev *v) {
    while (v->bits < 32u) {
        (void)jph_vlc_rev_read(v);
    }
    return (uint32_t)v->tmp;
}

static uint64_t jph_vlc_rev_fetch64(JphVlcRev *v) {
    while (v->bits < 32u) {
        (void)jph_vlc_rev_read(v);
    }
    return v->tmp;
}

static uint32_t jph_vlc_rev_advance(JphVlcRev *v, uint32_t n) {
    if (n > v->bits) n = v->bits;
    v->tmp >>= n;
    v->bits -= n;
    return (uint32_t)v->tmp;
}

static uint64_t jph_vlc_rev_advance64(JphVlcRev *v, uint32_t n) {
    if (n > v->bits) n = v->bits;
    v->tmp >>= n;
    v->bits -= n;
    return v->tmp;
}

static exr_result jph_mrp_rev_init(JphVlcRev *v, const uint8_t *buf,
                                   uint32_t lcup, uint32_t len2) {
    uint32_t i, num;
    if (!v || !buf) return EXR_ERROR_INVALID_ARGUMENT;
    memset(v, 0, sizeof(*v));
    v->data = buf + lcup + len2 - 1u;
    v->start = buf + lcup;
    v->size = len2;
    v->unstuff = 1u;
    num = 1u + ((uint32_t)(uintptr_t)(v->data) & 3u);
    for (i = 0; i < num; ++i) {
        uint64_t d = 0;
        uint32_t nb;
        if (v->size > 0u) {
            d = *v->data--;
            v->size--;
        }
        nb = 8u - ((v->unstuff && (((uint32_t)d & 0x7fu) == 0x7fu)) ? 1u : 0u);
        v->tmp |= d << v->bits;
        v->bits += nb;
        v->unstuff = (d > 0x8fu) ? 1u : 0u;
    }
    return jph_vlc_rev_read(v);
}

static uint32_t jph_mrp_rev_fetch(JphVlcRev *v) {
    if (v->bits < 32u) {
        (void)jph_vlc_rev_read(v);
        if (v->bits < 32u) (void)jph_vlc_rev_read(v);
    }
    return (uint32_t)v->tmp;
}

static uint32_t jph_popcount32(uint32_t v) {
    v = v - ((v >> 1u) & 0x55555555u);
    v = (v & 0x33333333u) + ((v >> 2u) & 0x33333333u);
    return (((v + (v >> 4u)) & 0x0f0f0f0fu) * 0x01010101u) >> 24u;
}

static uint32_t jph_load_u32_from_u16(const uint16_t *p) {
    return ((uint32_t)p[0]) | ((uint32_t)p[1] << 16u);
}

static uint32_t jph_mask32(uint32_t nbits) {
    return nbits >= 32u ? UINT32_MAX : ((UINT32_C(1) << nbits) - 1u);
}

static inline uint64_t jph_mask64(uint32_t nbits) {
    return nbits >= 64u ? UINT64_MAX : ((UINT64_C(1) << nbits) - 1u);
}

/* ----- MagSgn forward reader (unstuffed contiguous bit buffer) -------------
 *
 * The whole segment is unstuffed once, up front, into a caller-provided bit
 * buffer (LSB-first; a 0xff byte is followed by a 7-bit byte whose MSB is the
 * stuffed bit and dropped). Reads are then pure shifts at a bit cursor, with no
 * per-fetch refill branch -- faster than the old lazy accumulator and, crucially,
 * a contiguous random-accessible layout the SIMD magnitude/sign path will gather
 * from. Past the real bits the stream yields the `fill` pattern (1s for MagSgn,
 * 0s for SigProp), matching the original feed-on-exhaustion behaviour.
 *
 * The buffer must be zero-initialised and hold >= size/8 + 3 uint64 words. */
typedef struct {
    const uint64_t *buf; /* unstuffed bits; bit k = buf[k>>6] >> (k&63) */
    uint64_t real_bits;  /* count of valid unstuffed bits */
    uint64_t cursor;     /* current bit position */
    uint64_t fill_word;  /* bits beyond real_bits: ~0 (MagSgn) or 0 (SigProp) */
} JphMagSgn;

/* Unstuff `size` bytes of `data` into `out` (LSB-first); returns the bit count.
 * `out` must be zeroed and hold >= size/8 + 3 words. */
#if defined(__BYTE_ORDER__) && __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
static int jph_word_has_ff(uint64_t v) {
    uint64_t x = ~v; /* 0x00 in each lane that held 0xFF */
    return ((x - UINT64_C(0x0101010101010101)) & ~x &
            UINT64_C(0x8080808080808080)) != 0u;
}
#endif

static uint64_t jph_unstuff_bits(const uint8_t *data, uint32_t size,
                                 uint64_t *out) {
    uint64_t nbits = 0u;
    uint32_t idx, prevff = 0u;
#if defined(__BYTE_ORDER__) && __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
    /* Common-case fast path: with no 0xFF byte every byte contributes 8 bits at
     * a byte-aligned offset, so the host-order bit buffer is byte-identical to
     * the input. Copy and scan in one pass; if a stuffed byte appears, clear the
     * partial copy and fall back to the general bit packer below. */
    if (data) {
        uint32_t i = 0u;
        uint8_t *out8 = (uint8_t *)out;
        for (; i + 8u <= size; i += 8u) {
            uint64_t v;
            memcpy(&v, data + i, 8u);
            out[i >> 3u] = v;
            if (jph_word_has_ff(v)) {
                goto slow_unstuff;
            }
        }
        for (; i < size; ++i) {
            uint8_t b = data[i];
            out8[i] = b;
            if (b == 0xFFu) {
                goto slow_unstuff;
            }
        }
        return (uint64_t)size * 8u;
    }
#endif
slow_unstuff:
    /* Slow path (stuffed stream): accumulate bits in a register and flush whole
     * words, instead of a per-byte read-modify-write of out[] plus a split-store
     * branch. `acc` holds the `accbits` not-yet-flushed low bits of word `w`. */
    {
        uint64_t acc = 0u;
        uint32_t accbits = 0u;
        size_t w = 0u;
        size_t out_words = (size_t)size / 8u + 3u;
        for (idx = 0u; idx < size; ++idx) {
            uint8_t b = data ? data[idx] : 0u;
            uint32_t ub = prevff ? 7u : 8u;
            uint64_t uv = (uint64_t)(b & (uint8_t)((1u << ub) - 1u));
            uint32_t old = accbits;
            acc |= uv << old;
            accbits += ub;
            nbits += ub;
            if (accbits >= 64u) {
                out[w++] = acc;
                accbits -= 64u;
                /* carry the bits of uv that didn't fit (shift is 1..8 here). */
                acc = accbits ? (uv >> (64u - old)) : 0u;
            }
            prevff = (b == 0xffu) ? 1u : 0u;
        }
        if (accbits) out[w++] = acc;
        if (w < out_words) memset(out + w, 0, (out_words - w) * sizeof(uint64_t));
    }
    return nbits;
}

static exr_result jph_forward_bits_init(JphMagSgn *m, const uint8_t *data,
                                        uint32_t size, uint8_t fill,
                                        uint64_t *bitbuf) {
    if (!m || !bitbuf) return EXR_ERROR_INVALID_ARGUMENT;
    m->buf = bitbuf;
    m->cursor = 0u;
    m->fill_word = fill ? ~UINT64_C(0) : UINT64_C(0);
    m->real_bits = jph_unstuff_bits(data, size, bitbuf);
    return EXR_SUCCESS;
}

static exr_result jph_magsgn_init(JphMagSgn *m, const uint8_t *data,
                                  uint32_t size, uint64_t *bitbuf) {
    return jph_forward_bits_init(m, data, size, 0xffu, bitbuf);
}

/* Read 64 unstuffed bits at an arbitrary absolute bit position (no cursor move),
 * with the same fill semantics as jph_magsgn_fetch64. */
static uint64_t jph_magsgn_fetch64_at(const JphMagSgn *m, uint64_t c) {
    uint64_t v, avail;
    uint32_t off;
    if (c >= m->real_bits) return m->fill_word;
    off = (uint32_t)(c & 63u);
    v = m->buf[c >> 6u] >> off;
    if (off) v |= m->buf[(c >> 6u) + 1u] << (64u - off);
    avail = m->real_bits - c;
    if (avail < 64u && m->fill_word) v |= ~UINT64_C(0) << avail;
    return v;
}

static uint64_t jph_magsgn_fetch64(JphMagSgn *m) {
    if (!m) return 0u;
    return jph_magsgn_fetch64_at(m, m->cursor);
}

static uint32_t jph_magsgn_fetch(JphMagSgn *m) {
    return (uint32_t)jph_magsgn_fetch64(m);
}

static void jph_magsgn_advance(JphMagSgn *m, uint32_t n) {
    if (m) m->cursor += n;
}

/* Per-quad MagSgn reader for the int32 path. A whole HT quad of this path
 * consumes <= 4*31 = 124 bits, so one 128-bit window at the quad's start cursor
 * covers it and the 4 samples are extracted at running bit offsets -- replacing
 * the 4 per-sample fetch+advance with (usually) a single 64-bit read. The upper
 * 64 bits are fetched lazily, only when a sample actually crosses bit 64 (rare
 * for HALF data, whose quad magnitudes are small). Bit-identical to the
 * per-sample core of jph_decode_block. */
typedef struct {
    uint64_t lo, hi, base;
    uint32_t off;
    int hi_loaded;
} JphQuadMs;

static inline void jph_quad_ms_begin(JphQuadMs *q, const JphMagSgn *m) {
    q->base = m->cursor;
    q->lo = jph_magsgn_fetch64_at(m, q->base);
    q->hi = 0u;
    q->off = 0u;
    q->hi_loaded = 0;
}

static inline uint32_t jph_quad_ms_sample(JphQuadMs *q, const JphMagSgn *m,
                                          uint32_t inf, uint32_t bit,
                                          uint32_t U_q, uint32_t p,
                                          uint32_t *v_n_out) {
    uint32_t val = 0u, v_n = 0u;
    if (inf & (1u << (4u + bit))) {
        uint32_t o = q->off;
        uint32_t m_n = U_q - ((inf >> (12u + bit)) & 1u);
        uint32_t ms;
        /* Caller must ensure U_q <= 31 so total consumed bits across all four
         * samples stays <= 4*31 = 124 < 128 (the two-word window).  Guard here
         * to prevent UB from a shift >= 64 if that invariant is ever violated. */
        if (o + m_n > 127u) {
            *v_n_out = 0u;
            return 0u;
        }
        if (o + m_n > 64u && !q->hi_loaded) {
            q->hi = jph_magsgn_fetch64_at(m, q->base + 64u);
            q->hi_loaded = 1;
        }
        ms = (o < 64u) ? (uint32_t)((q->lo >> o) | (o ? (q->hi << (64u - o)) : 0u))
                       : (uint32_t)(q->hi >> (o - 64u));
        val = ms << 31;
        v_n = ms & jph_mask32(m_n);
        v_n |= ((inf >> (8u + bit)) & 1u) << m_n;
        v_n |= 1u;
        val |= (v_n + 2u) << (p - 1u);
        q->off = o + m_n;
    }
    *v_n_out = v_n;
    return val;
}

static inline void jph_quad_ms_end(JphQuadMs *q, JphMagSgn *m) {
    m->cursor = q->base + q->off;
}

/* uint64 words needed for jph_unstuff_bits over `size` bytes (+ fetch slack). */
static size_t jph_magsgn_words(uint32_t size) { return (size_t)size / 8u + 3u; }


static uint64_t jph_decode_magsgn_sample64(JphMagSgn *magsgn, uint32_t inf,
                                           uint32_t bit, uint32_t u_q,
                                           uint32_t p, uint64_t *v_n,
                                           exr_result *rc) {
    uint64_t val = 0u;
    *v_n = 0u;
    if (rc) *rc = EXR_SUCCESS;
    if (inf & (1u << (4u + bit))) {
        uint64_t ms_val = jph_magsgn_fetch64(magsgn);
        uint32_t m_n = u_q - ((inf >> (12u + bit)) & 1u);
        uint64_t mask;
        if (m_n >= 63u || p == 0u) {
            if (rc) *rc = EXR_ERROR_CORRUPT;
            return 0u;
        }
        mask = jph_mask64(m_n);
        jph_magsgn_advance(magsgn, m_n);
        val = ms_val << 63u;
        *v_n = ms_val & mask;
        *v_n |= (uint64_t)((inf >> (8u + bit)) & 1u) << m_n;
        *v_n |= 1u;
        val |= (*v_n + 2u) << (p - 1u);
    }
    return val;
}

static inline int jph_clz64(uint64_t v);
static inline int jph_clz32(uint32_t v);

static exr_result jph_decode_block64_cleanup(const JphCodeblockSeg *seg,
                                             const JphHtTables *htab,
                                             int64_t *out,
                                             uint32_t out_stride,
                                             uint32_t kmax) {
    uint32_t width = seg->width;
    uint32_t height = seg->height;
    uint32_t missing_msbs = seg->missing_msbs;
    uint32_t num_passes = seg->active_passes;
    uint32_t lengths1 = seg->length0;
    uint32_t lengths2 = seg->length1;
    uint32_t lcup, scup;
    uint32_t p, mmsbp2;
    uint32_t sstr;
    uint16_t *scratch = NULL;
    uint64_t *v_n_scratch = NULL;
    uint64_t *buf = NULL;
    uint64_t *magsgn_bits = NULL;
    uint64_t *sigprop_bits = NULL;
    size_t scratch_count, v_n_count, buf_count;
    exr_jph_mel_reader mel;
    JphVlcRev vlc;
    JphMagSgn magsgn;
    uint32_t run, c_q;
    uint64_t vlc_val;
    uint16_t *sp;
    uint64_t *dp;
    uint64_t prev_v_n;
    int i;
    exr_result rc;

    /* Defense-in-depth: HT codeblocks are at most 128x32 (callers/validation
     * enforce this). Re-check locally so the fixed-size scratch math below
     * cannot be driven OOB if a future caller skips the guard. */
    if (width == 0u || height == 0u || width > 128u || height > 32u)
        return EXR_ERROR_CORRUPT;

    if (num_passes > 1u && lengths2 == 0u) num_passes = 1u;
    if (num_passes < 1u || num_passes > 3u) return EXR_ERROR_UNSUPPORTED;
    if (missing_msbs > 62u) return EXR_ERROR_CORRUPT;
    if (lengths1 < 2u) return EXR_ERROR_CORRUPT;

    lcup = lengths1;
    scup = ((uint32_t)seg->data[lcup - 1u] << 4) |
           (seg->data[lcup - 2u] & 0x0Fu);
    if (scup < 2u || scup > lcup || scup > 4079u) return EXR_ERROR_CORRUPT;

    p = 62u - missing_msbs;
    if (p == 0u) return EXR_ERROR_UNSUPPORTED;
    mmsbp2 = missing_msbs + 2u;
    sstr = ((width + 2u) + 7u) & ~7u;

    scratch_count = (size_t)sstr * (((height + 1u) >> 1u) + 1u);
    v_n_count = JPH_V_N_SIZE;
    buf_count = (size_t)sstr * (height < 4u ? 4u : height);

    scratch = (uint16_t *)exr_calloc(exr_default_allocator(), scratch_count,
                                    sizeof(uint16_t));
    v_n_scratch = (uint64_t *)exr_calloc(exr_default_allocator(), v_n_count,
                                         sizeof(uint64_t));
    buf = (uint64_t *)exr_calloc(exr_default_allocator(),
                                 buf_count ? buf_count : 1u, sizeof(uint64_t));
    magsgn_bits = (uint64_t *)exr_calloc(exr_default_allocator(),
                                         jph_magsgn_words(lcup - scup),
                                         sizeof(uint64_t));
    sigprop_bits = (uint64_t *)exr_calloc(exr_default_allocator(),
                                          jph_magsgn_words(lengths2),
                                          sizeof(uint64_t));
    if (!scratch || !v_n_scratch || !buf || !magsgn_bits || !sigprop_bits) {
        rc = EXR_ERROR_OUT_OF_MEMORY;
        goto done;
    }

    exr_jph_mel_init(&mel, seg->data + lcup - scup, (size_t)scup - 1u);
    rc = jph_vlc_rev_init(&vlc, seg->data, lcup, scup);
    if (rc != EXR_SUCCESS) goto done;
    rc = jph_magsgn_init(&magsgn, seg->data, lcup - scup, magsgn_bits);
    if (rc != EXR_SUCCESS) goto done;

    if (exr_jph_mel_get_run(&mel, &run, &i) != EXR_SUCCESS) i = 0;
    run = i ? ((run << 1u) | 1u) : (run << 1u);

    c_q = 0u;
    sp = scratch;
    {
        uint32_t x = 0u;
        while (x < width) {
            uint16_t t0, t1;
            uint32_t uvlc_mode, uvlc_entry, len, tmp;
            uint32_t q0_suffix_len, q0, q1, idx, u_bias;
            uint32_t cond0, cond1, u_ext;

            vlc_val = jph_vlc_rev_fetch64(&vlc);
            t0 = htab->vlc_tbl0[c_q + ((uint32_t)vlc_val & 0x7Fu)];
            if (c_q == 0u) {
                run -= 2u;
                t0 = (run == UINT32_MAX) ? t0 : 0u;
                if ((int32_t)run < 0) {
                    if (exr_jph_mel_get_run(&mel, &run, &i) != EXR_SUCCESS) i = 0;
                    run = i ? ((run << 1u) | 1u) : (run << 1u);
                }
            }
            sp[0] = t0;
            x += 2u;
            c_q = ((t0 & 0x10u) << 3u) | ((t0 & 0xE0u) << 2u);
            vlc_val = jph_vlc_rev_advance64(&vlc, t0 & 0x7u);

            t1 = htab->vlc_tbl0[c_q + ((uint32_t)vlc_val & 0x7Fu)];
            if (c_q == 0u && x < width) {
                run -= 2u;
                t1 = (run == UINT32_MAX) ? t1 : 0u;
                if ((int32_t)run < 0) {
                    if (exr_jph_mel_get_run(&mel, &run, &i) != EXR_SUCCESS) i = 0;
                    run = i ? ((run << 1u) | 1u) : (run << 1u);
                }
            }
            t1 = (x < width) ? t1 : 0u;
            sp[2] = t1;
            x += 2u;
            c_q = ((t1 & 0x10u) << 3u) | ((t1 & 0xE0u) << 2u);
            vlc_val = jph_vlc_rev_advance64(&vlc, t1 & 0x7u);

            uvlc_mode = ((t0 & 0x8u) << 3u) | ((t1 & 0x8u) << 4u);
            if (uvlc_mode == 0xC0u) {
                run -= 2u;
                uvlc_mode += (run == UINT32_MAX) ? 0x40u : 0u;
                if ((int32_t)run < 0) {
                    if (exr_jph_mel_get_run(&mel, &run, &i) != EXR_SUCCESS) i = 0;
                    run = i ? ((run << 1u) | 1u) : (run << 1u);
                }
            }
            idx = uvlc_mode + ((uint32_t)vlc_val & 0x3Fu);
            uvlc_entry = htab->uvlc_tbl0[idx];
            u_bias = htab->uvlc_bias[idx];
            vlc_val = jph_vlc_rev_advance64(&vlc, uvlc_entry & 0x7u);
            uvlc_entry >>= 3u;
            len = uvlc_entry & 0xFu;
            tmp = (uint32_t)(vlc_val & ((UINT32_C(1) << len) - 1u));
            vlc_val = jph_vlc_rev_advance64(&vlc, len);
            uvlc_entry >>= 4u;
            q0_suffix_len = uvlc_entry & 0x7u;
            uvlc_entry >>= 3u;
            q0 = (uvlc_entry & 0x7u) +
                 (tmp & ((UINT32_C(1) << q0_suffix_len) - 1u));
            q1 = (uvlc_entry >> 3u) + (tmp >> q0_suffix_len);

            cond0 = q0 - (u_bias & 0x3u) > 32u;
            u_ext = cond0 ? ((uint32_t)vlc_val & 0xFu) : 0u;
            vlc_val = jph_vlc_rev_advance64(&vlc, cond0 ? 4u : 0u);
            q0 += u_ext << 2u;
            sp[1] = (uint16_t)(1u + q0);

            cond1 = q1 - (u_bias >> 2u) > 32u;
            u_ext = cond1 ? ((uint32_t)vlc_val & 0xFu) : 0u;
            vlc_val = jph_vlc_rev_advance64(&vlc, cond1 ? 4u : 0u);
            q1 += u_ext << 2u;
            sp[3] = (uint16_t)(1u + q1);
            sp += 4u;
        }
    }
    sp[0] = sp[1] = 0u;

    {
        uint32_t y;
        for (y = 2u; y < height; y += 2u) {
            uint32_t x = 0u;
            sp = scratch + (y >> 1u) * sstr;
            c_q = 0u;
            while (x < width) {
                uint16_t t0, t1;
                uint32_t uvlc_entry, len, tmp;
                uint32_t q0, q1, q0_suffix_len, cond0, cond1, u_ext;

                c_q |= ((sp[0 - (int32_t)sstr] & 0xA0u) << 2u);
                c_q |= ((sp[2 - (int32_t)sstr] & 0x20u) << 4u);
                vlc_val = jph_vlc_rev_fetch64(&vlc);
                t0 = htab->vlc_tbl1[c_q + ((uint32_t)vlc_val & 0x7Fu)];
                if (c_q == 0u) {
                    run -= 2u;
                    t0 = (run == UINT32_MAX) ? t0 : 0u;
                    if ((int32_t)run < 0) {
                        if (exr_jph_mel_get_run(&mel, &run, &i) != EXR_SUCCESS)
                            i = 0;
                        run = i ? ((run << 1u) | 1u) : (run << 1u);
                    }
                }
                sp[0] = t0;
                x += 2u;

                c_q = ((t0 & 0x40u) << 2u) | ((t0 & 0x80u) << 1u);
                c_q |= sp[0 - (int32_t)sstr] & 0x80u;
                c_q |= ((sp[2 - (int32_t)sstr] & 0xA0u) << 2u);
                c_q |= ((sp[4 - (int32_t)sstr] & 0x20u) << 4u);
                vlc_val = jph_vlc_rev_advance64(&vlc, t0 & 0x7u);

                t1 = htab->vlc_tbl1[c_q + ((uint32_t)vlc_val & 0x7Fu)];
                if (c_q == 0u && x < width) {
                    run -= 2u;
                    t1 = (run == UINT32_MAX) ? t1 : 0u;
                    if ((int32_t)run < 0) {
                        if (exr_jph_mel_get_run(&mel, &run, &i) != EXR_SUCCESS)
                            i = 0;
                        run = i ? ((run << 1u) | 1u) : (run << 1u);
                    }
                }
                t1 = (x < width) ? t1 : 0u;
                sp[2] = t1;
                x += 2u;

                c_q = ((t1 & 0x40u) << 2u) | ((t1 & 0x80u) << 1u);
                c_q |= sp[2 - (int32_t)sstr] & 0x80u;
                vlc_val = jph_vlc_rev_advance64(&vlc, t1 & 0x7u);

                uvlc_entry = htab->uvlc_tbl1[((t0 & 0x8u) << 3u) |
                                             ((t1 & 0x8u) << 4u) |
                                             ((uint32_t)vlc_val & 0x3Fu)];
                vlc_val = jph_vlc_rev_advance64(&vlc, uvlc_entry & 0x7u);
                uvlc_entry >>= 3u;
                len = uvlc_entry & 0xFu;
                tmp = (uint32_t)(vlc_val & ((UINT32_C(1) << len) - 1u));
                vlc_val = jph_vlc_rev_advance64(&vlc, len);
                uvlc_entry >>= 4u;
                q0_suffix_len = uvlc_entry & 0x7u;
                uvlc_entry >>= 3u;
                q0 = (uvlc_entry & 0x7u) +
                     (tmp & ((UINT32_C(1) << q0_suffix_len) - 1u));
                q1 = (uvlc_entry >> 3u) + (tmp >> q0_suffix_len);

                cond0 = q0 > 32u;
                u_ext = cond0 ? ((uint32_t)vlc_val & 0xFu) : 0u;
                vlc_val = jph_vlc_rev_advance64(&vlc, cond0 ? 4u : 0u);
                sp[1] = (uint16_t)(q0 + (u_ext << 2u));

                cond1 = q1 > 32u;
                u_ext = cond1 ? ((uint32_t)vlc_val & 0xFu) : 0u;
                vlc_val = jph_vlc_rev_advance64(&vlc, cond1 ? 4u : 0u);
                sp[3] = (uint16_t)(q1 + (u_ext << 2u));
                sp += 4u;
            }
            sp[0] = sp[1] = 0u;
        }
    }

    {
        uint32_t x = 0u;
        uint64_t *vp = v_n_scratch;
        prev_v_n = 0u;
        sp = scratch;
        dp = buf;
        while (x < width) {
            uint32_t inf = sp[0];
            uint32_t u_q = sp[1];
            uint64_t v_n;
            exr_result sample_rc;
            if (u_q > mmsbp2) {
                rc = EXR_ERROR_CORRUPT;
                goto done;
            }
            dp[0] = jph_decode_magsgn_sample64(&magsgn, inf, 0u, u_q, p,
                                                &v_n, &sample_rc);
            if (sample_rc != EXR_SUCCESS) { rc = sample_rc; goto done; }
            if (1u < height) {
                dp[sstr] =
                    jph_decode_magsgn_sample64(&magsgn, inf, 1u, u_q, p,
                                               &v_n, &sample_rc);
            } else {
                (void)jph_decode_magsgn_sample64(&magsgn, inf, 1u, u_q, p,
                                                 &v_n, &sample_rc);
            }
            if (sample_rc != EXR_SUCCESS) { rc = sample_rc; goto done; }
            vp[0] = prev_v_n | v_n;
            prev_v_n = 0u;
            ++dp;
            ++x;
            if (x >= width) {
                ++vp;
                break;
            }
            dp[0] = jph_decode_magsgn_sample64(&magsgn, inf, 2u, u_q, p,
                                                &v_n, &sample_rc);
            if (sample_rc != EXR_SUCCESS) { rc = sample_rc; goto done; }
            if (1u < height) {
                dp[sstr] =
                    jph_decode_magsgn_sample64(&magsgn, inf, 3u, u_q, p,
                                               &v_n, &sample_rc);
            } else {
                (void)jph_decode_magsgn_sample64(&magsgn, inf, 3u, u_q, p,
                                                 &v_n, &sample_rc);
            }
            if (sample_rc != EXR_SUCCESS) { rc = sample_rc; goto done; }
            prev_v_n = v_n;
            ++dp;
            ++x;
            sp += 2u;
            ++vp;
        }
        vp[0] = prev_v_n;
    }

    {
        uint32_t y;
        for (y = 2u; y < height; y += 2u) {
            uint32_t x = 0u;
            uint64_t *vp = v_n_scratch;
            prev_v_n = 0u;
            sp = scratch + (y >> 1u) * sstr;
            dp = buf + y * sstr;
            while (x < width) {
                uint32_t inf = sp[0];
                uint32_t u_q = sp[1];
                uint32_t gamma = inf & 0xF0u;
                uint64_t emax_src;
                uint32_t emax, kappa, u_q_eff;
                uint64_t v_n;
                exr_result sample_rc;
                gamma &= gamma - 0x10u;
                emax_src = vp[0] | vp[1] | 2u; /* nonzero (| 2u) */
                emax = 63u - (uint32_t)jph_clz64(emax_src);
                kappa = gamma ? emax : 1u;
                u_q_eff = u_q + kappa;
                if (u_q_eff > mmsbp2) {
                    rc = EXR_ERROR_CORRUPT;
                    goto done;
                }
                dp[0] = jph_decode_magsgn_sample64(&magsgn, inf, 0u, u_q_eff,
                                                    p, &v_n, &sample_rc);
                if (sample_rc != EXR_SUCCESS) { rc = sample_rc; goto done; }
                if (y + 1u < height) {
                    dp[sstr] = jph_decode_magsgn_sample64(&magsgn, inf, 1u,
                                                          u_q_eff, p, &v_n,
                                                          &sample_rc);
                } else {
                    (void)jph_decode_magsgn_sample64(&magsgn, inf, 1u, u_q_eff,
                                                     p, &v_n, &sample_rc);
                }
                if (sample_rc != EXR_SUCCESS) { rc = sample_rc; goto done; }
                vp[0] = prev_v_n | v_n;
                prev_v_n = 0u;
                ++dp;
                ++x;
                if (x >= width) {
                    ++vp;
                    break;
                }
                dp[0] = jph_decode_magsgn_sample64(&magsgn, inf, 2u, u_q_eff,
                                                    p, &v_n, &sample_rc);
                if (sample_rc != EXR_SUCCESS) { rc = sample_rc; goto done; }
                if (y + 1u < height) {
                    dp[sstr] = jph_decode_magsgn_sample64(&magsgn, inf, 3u,
                                                          u_q_eff, p, &v_n,
                                                          &sample_rc);
                } else {
                    (void)jph_decode_magsgn_sample64(&magsgn, inf, 3u, u_q_eff,
                                                     p, &v_n, &sample_rc);
                }
                if (sample_rc != EXR_SUCCESS) { rc = sample_rc; goto done; }
                prev_v_n = v_n;
                ++dp;
                ++x;
                sp += 2u;
                ++vp;
            }
            vp[0] = prev_v_n;
        }
    }

    /* ---- SPP pass (significance propagation) ---- */
    if (num_passes > 1u) {
        uint16_t *sigma = scratch;
        uint32_t mstr = (width + 3u) >> 2u;
        uint16_t prev_row_sig[256u + 8u];
        JphMagSgn sigprop;

        mstr = ((mstr + 2u) + 7u) & ~7u;
        if ((size_t)mstr * (((height + 3u) >> 2u) + 1u) > scratch_count) {
            rc = EXR_ERROR_CORRUPT;
            goto done;
        }

        {
            uint32_t y;
            for (y = 0u; y < height; y += 4u) {
                uint16_t *ssp = scratch + (y >> 1u) * sstr;
                uint16_t *sdp = sigma + (y >> 2u) * mstr;
                uint32_t x;
                for (x = 0u; x < width; x += 4u, ssp += 4u, ++sdp) {
                    uint32_t t0 = 0u, t1 = 0u;
                    t0 = ((ssp[0] & 0x30u) >> 4u) |
                         ((ssp[0] & 0xc0u) >> 2u);
                    t0 |= ((ssp[2] & 0x30u) << 4u) |
                          ((ssp[2] & 0xc0u) << 6u);
                    t1 = ((ssp[sstr] & 0x30u) >> 2u) |
                         (ssp[sstr] & 0xc0u);
                    t1 |= ((ssp[2u + sstr] & 0x30u) << 6u) |
                          ((ssp[2u + sstr] & 0xc0u) << 8u);
                    sdp[0] = (uint16_t)(t0 | t1);
                }
                sdp[0] = 0u;
            }
            {
                uint16_t *sdp = sigma + (y >> 2u) * mstr;
                uint32_t x;
                for (x = 0u; x < width; x += 4u, ++sdp) sdp[0] = 0u;
                sdp[0] = 0u;
            }
        }

        memset(prev_row_sig, 0, sizeof(prev_row_sig));
        rc = jph_forward_bits_init(&sigprop, seg->data + lengths1, lengths2, 0u,
                                   sigprop_bits);
        if (rc != EXR_SUCCESS) goto done;

        {
            uint32_t y;
            for (y = 0u; y < height; y += 4u) {
                uint32_t row_pattern = 0xffffu;
                uint32_t prev = 0u;
                uint16_t *prev_sig = prev_row_sig;
                uint16_t *cur_sig = sigma + (y >> 2u) * mstr;
                uint64_t *dpp = buf + y * sstr;
                if (height - y < 4u) {
                    row_pattern = 0x7777u;
                    if (height - y < 3u) {
                        row_pattern = 0x3333u;
                        if (height - y < 2u) row_pattern = 0x1111u;
                    }
                }

                for (uint32_t x = 0u; x < width; x += 4u, ++cur_sig, ++prev_sig) {
                    uint32_t s = x + 4u - width;
                    uint32_t pattern = row_pattern >> ((s < 4u ? s : 0u) * 4u);
                    uint32_t ps = jph_load_u32_from_u16(prev_sig);
                    uint32_t ns = jph_load_u32_from_u16(cur_sig + mstr);
                    uint32_t u = (ps & 0x88888888u) >> 3u;
                    uint32_t cs = jph_load_u32_from_u16(cur_sig);
                    uint32_t mbr, t, new_sig;

                    u |= (ns & 0x11111111u) << 3u;
                    mbr = cs;
                    mbr |= (cs & 0x77777777u) << 1u;
                    mbr |= (cs & 0xeeeeeeeeu) >> 1u;
                    mbr |= u;
                    t = mbr;
                    mbr |= t << 4u;
                    mbr |= t >> 4u;
                    mbr |= prev >> 12u;
                    mbr &= pattern;
                    mbr &= ~cs;

                    new_sig = mbr;
                    if (new_sig) {
                        uint32_t cwd = jph_magsgn_fetch(&sigprop);
                        uint32_t cnt = 0u;
                        uint32_t col_mask = 0x0fu;
                        uint32_t inv_sig = ~cs & pattern;
                        for (int bi = 0; bi < 16; bi += 4, col_mask <<= 4u) {
                            uint32_t sample_mask;
                            if ((col_mask & new_sig) == 0u) continue;

                            sample_mask = 0x1111u & col_mask;
                            if (new_sig & sample_mask) {
                                new_sig &= ~sample_mask;
                                if (cwd & 1u) new_sig |= (0x33u << bi) & inv_sig;
                                cwd >>= 1u;
                                ++cnt;
                            }
                            sample_mask <<= 1u;
                            if (new_sig & sample_mask) {
                                new_sig &= ~sample_mask;
                                if (cwd & 1u) new_sig |= (0x76u << bi) & inv_sig;
                                cwd >>= 1u;
                                ++cnt;
                            }
                            sample_mask <<= 1u;
                            if (new_sig & sample_mask) {
                                new_sig &= ~sample_mask;
                                if (cwd & 1u) new_sig |= (0xecu << bi) & inv_sig;
                                cwd >>= 1u;
                                ++cnt;
                            }
                            sample_mask <<= 1u;
                            if (new_sig & sample_mask) {
                                new_sig &= ~sample_mask;
                                if (cwd & 1u) new_sig |= (0xc8u << bi) & inv_sig;
                                cwd >>= 1u;
                                ++cnt;
                            }
                        }

                        if (new_sig) {
                            uint64_t *rdp = dpp + x;
                            uint64_t val = (uint64_t)3u << (p - 2u);
                            col_mask = 0x0fu;
                            for (int bi = 0; bi < 4; ++bi, ++rdp, col_mask <<= 4u) {
                                uint32_t sample_mask;
                                if ((col_mask & new_sig) == 0u) continue;

                                sample_mask = 0x1111u & col_mask;
                                if (new_sig & sample_mask) {
                                    rdp[0] = ((uint64_t)(cwd & 1u) << 63u) | val;
                                    cwd >>= 1u;
                                    ++cnt;
                                }
                                sample_mask <<= 1u;
                                if ((new_sig & sample_mask) && y + 1u < height) {
                                    rdp[sstr] = ((uint64_t)(cwd & 1u) << 63u) | val;
                                    cwd >>= 1u;
                                    ++cnt;
                                }
                                sample_mask <<= 1u;
                                if ((new_sig & sample_mask) && y + 2u < height) {
                                    rdp[2u * sstr] = ((uint64_t)(cwd & 1u) << 63u) | val;
                                    cwd >>= 1u;
                                    ++cnt;
                                }
                                sample_mask <<= 1u;
                                if ((new_sig & sample_mask) && y + 3u < height) {
                                    rdp[3u * sstr] = ((uint64_t)(cwd & 1u) << 63u) | val;
                                    cwd >>= 1u;
                                    ++cnt;
                                }
                            }
                        }
                        jph_magsgn_advance(&sigprop, cnt);
                    }

                    new_sig |= cs;
                    *prev_sig = (uint16_t)new_sig;
                    t = new_sig;
                    new_sig |= (t & 0x7777u) << 1u;
                    new_sig |= (t & 0xeeeeu) >> 1u;
                    prev = (new_sig | u) & 0xf000u;
                }
            }
        }

        if (num_passes > 2u) {
            JphVlcRev magref;
            rc = jph_mrp_rev_init(&magref, seg->data, lengths1, lengths2);
            if (rc != EXR_SUCCESS) goto done;

            for (uint32_t y = 0u; y < height; y += 4u) {
                uint16_t *cur_sig = sigma + (y >> 2u) * mstr;
                uint64_t *dpp = buf + y * sstr;
                uint64_t half = (uint64_t)1u << (p - 2u);
                for (uint32_t x = 0u; x < width; x += 8u, cur_sig += 2u) {
                    uint32_t cwd = jph_mrp_rev_fetch(&magref);
                    uint32_t sig = jph_load_u32_from_u16(cur_sig);
                    uint32_t col_mask = 0x0fu;
                    if (sig) {
                        for (int j = 0; j < 8; ++j, col_mask <<= 4u) {
                            uint32_t sample_mask;
                            uint64_t *rdp;
                            if ((sig & col_mask) == 0u) continue;
                            rdp = dpp + x + (uint32_t)j;
                            sample_mask = 0x11111111u & col_mask;
                            for (int k = 0; k < 4; ++k) {
                                if (sig & sample_mask) {
                                    uint64_t sym = (uint64_t)(cwd & 1u);
                                    sym = ((uint64_t)(1u - (cwd & 1u))) << (p - 1u);
                                    sym |= half;
                                    rdp[0] ^= sym;
                                    cwd >>= 1u;
                                }
                                sample_mask <<= 1u;
                                rdp += sstr;
                            }
                        }
                    }
                    (void)jph_vlc_rev_advance(&magref, jph_popcount32(sig));
                }
            }
        }
    }

    {
        uint32_t y;
        uint32_t shift = (kmax < 63u) ? 63u - kmax : 0u;
        for (y = 0u; y < height; ++y) {
            uint32_t x;
            for (x = 0u; x < width; ++x) {
                uint64_t v = buf[y * sstr + x];
                uint64_t mag = (v & UINT64_C(0x7fffffffffffffff)) >> shift;
                /* Reversible 5/3 coefficients of a 32-bit component can exceed
                 * int32; keep full int64 width here. The mask is 63-bit so
                 * `mag` always fits a signed int64. The final sample is range-
                 * checked when narrowed to int32 at store time. */
                out[y * out_stride + x] =
                    (v & UINT64_C(0x8000000000000000)) ? -(int64_t)mag
                                                       : (int64_t)mag;
            }
        }
    }
    rc = EXR_SUCCESS;

done:
    exr_free(exr_default_allocator(), scratch);
    exr_free(exr_default_allocator(), v_n_scratch);
    exr_free(exr_default_allocator(), buf);
    exr_free(exr_default_allocator(), magsgn_bits);
    exr_free(exr_default_allocator(), sigprop_bits);
    return rc;
}

/* Decode a single HT codeblock and write the resulting signed coefficient
 * samples into `out` (int64; 32-bit components can exceed int32), indexed by
 * row*stride + col. kmax is the band's K_max (for shift normalization). */
static exr_result jph_decode_block_core(const JphCodeblockSeg *seg,
                                        const JphHtTables *htab,
                                        int64_t *out64, int32_t *out32,
                                        uint32_t out_stride, uint32_t kmax,
                                        const exr_allocator *a,
                                        JphDecodeWorkspace *ws,
                                        int use_avx2) {
    uint32_t width = seg->width;
    uint32_t height = seg->height;
    uint32_t missing_msbs = seg->missing_msbs;
    uint32_t num_passes = seg->active_passes;
    uint32_t lengths1 = seg->length0;
    uint32_t lengths2 = seg->length1;
    uint32_t lcup, scup;
    uint32_t p, mmsbp2;
    uint32_t sstr;
    uint16_t *scratch = NULL;
    uint32_t *v_n_scratch = NULL;
    uint32_t *buf = NULL;
    uint64_t *magsgn_bits = NULL;
    uint64_t *sigprop_bits = NULL;
    size_t scratch_count, v_n_count, buf_count;
    exr_jph_mel_reader mel;
    JphVlcRev vlc;
    JphMagSgn magsgn = {0};
#if defined(EXR_X86)
    JphFrwdAvx2 magsgn_frwd; /* AVX2 forward-magsgn fast path only */
#endif
    uint32_t run, vlc_val, c_q;
    uint16_t *sp;
    uint32_t *dp;
    int64_t prev_v_n;
    int use_four16 = 0;
    int use_frwd16 = 0;
    int use_frwd32 = 0;
    int i;
    exr_result rc;
    (void)i;

    if (!a) a = exr_default_allocator();
    if (!seg || !htab || (!out64 && !out32)) return EXR_ERROR_INVALID_ARGUMENT;
    /* Defense-in-depth: HT codeblocks are at most 128x32 (callers/validation
     * enforce this).  Re-check locally so the fixed-size scratch math below
     * cannot be driven OOB if a future caller skips the guard. */
    if (width == 0u || height == 0u) return EXR_ERROR_INVALID_ARGUMENT;
    if (width > 128u || height > 32u) return EXR_ERROR_CORRUPT;

    if (num_passes > 1u && lengths2 == 0u) num_passes = 1u;
    if (num_passes < 1u || num_passes > 3u) return EXR_ERROR_UNSUPPORTED;

    if (missing_msbs >= 30u || kmax > 30u) {
        if (out32) return EXR_ERROR_UNSUPPORTED;
        return jph_decode_block64_cleanup(seg, htab, out64, out_stride, kmax);
    }
    if (missing_msbs == 29u && num_passes > 1u) num_passes = 1u;

    if (lengths1 < 2u) return EXR_ERROR_CORRUPT;
    lcup = lengths1;
    scup = ((uint32_t)seg->data[lcup - 1u] << 4) |
           (seg->data[lcup - 2u] & 0x0Fu);
    if (scup < 2u || scup > lcup || scup > 4079u) return EXR_ERROR_CORRUPT;

    p = 30u - missing_msbs;
    mmsbp2 = missing_msbs + 2u;
    use_frwd16 = use_avx2 && mmsbp2 < 16u;
#if defined(EXR_X86)
    use_frwd32 = use_avx2 && mmsbp2 >= 16u;
#endif
#if defined(EXR_X86) && defined(EXR_JPH_ENABLE_AVX2_FOUR_QUAD16) && \
    EXR_JPH_ENABLE_AVX2_FOUR_QUAD16
    use_four16 = !use_frwd16 && use_avx2 && mmsbp2 < 16u;
#endif
#if !defined(EXR_X86)
    /* The AVX2 forward-magsgn fast paths these gate are compiled out off x86. */
    (void)use_frwd16;
    (void)use_four16;
#endif

    sstr = ((width + 2u) + 7u) & ~7u;

    scratch_count = (size_t)sstr * (((height + 1u) >> 1u) + 1u);
    v_n_count = (use_frwd32 ? 2u : 1u) * JPH_V_N_SIZE;
    /* The vector MagSgn step writes to the second row of every quad pair.  For
     * odd-height codeblocks, keep one padded row so the forward-reader vector
     * path can consume the final paired row instead of falling back to the
     * pre-unstuffed scalar reader. */
    {
        uint32_t buf_rows = height + (height & 1u);
        if (buf_rows < 4u) buf_rows = 4u;
        buf_count = (size_t)sstr * buf_rows;
    }

    if (ws) {
        size_t magsgn_words =
            (use_frwd16 || use_frwd32) ? 0u : jph_magsgn_words(lcup - scup);
        size_t sigprop_words = jph_magsgn_words(lengths2);
        rc = jph_ws_reserve_raw(a, (void **)&ws->scratch,
                                &ws->scratch_cap, scratch_count,
                                sizeof(uint16_t));
        if (rc != EXR_SUCCESS) goto done;
        rc = jph_ws_reserve_raw(a, (void **)&ws->v_n32,
                                &ws->v_n32_cap, v_n_count, sizeof(uint32_t));
        if (rc != EXR_SUCCESS) goto done;
        rc = jph_ws_reserve_raw(a, (void **)&ws->buf32,
                                &ws->buf32_cap, buf_count ? buf_count : 1u,
                                sizeof(uint32_t));
        if (rc != EXR_SUCCESS) goto done;
        if (!use_frwd16 && !use_frwd32) {
            rc = jph_ws_reserve_raw(a, (void **)&ws->magsgn_bits,
                                    &ws->magsgn_bits_cap, magsgn_words,
                                    sizeof(uint64_t));
            if (rc != EXR_SUCCESS) goto done;
        }
        rc = jph_ws_reserve_raw(a, (void **)&ws->sigprop_bits,
                                &ws->sigprop_bits_cap, sigprop_words,
                                sizeof(uint64_t));
        if (rc != EXR_SUCCESS) goto done;
        scratch = ws->scratch;
        v_n_scratch = ws->v_n32;
        buf = ws->buf32;
        magsgn_bits = ws->magsgn_bits;
        sigprop_bits = ws->sigprop_bits;
        if (!use_frwd16 && !use_frwd32)
            memset(magsgn_bits, 0, magsgn_words * sizeof(uint64_t));
        memset(sigprop_bits, 0, sigprop_words * sizeof(uint64_t));
    } else {
        size_t magsgn_words =
            (use_frwd16 || use_frwd32) ? 0u : jph_magsgn_words(lcup - scup);
        scratch = (uint16_t *)exr_calloc(a, scratch_count, sizeof(uint16_t));
        v_n_scratch = (uint32_t *)exr_calloc(a, v_n_count,
                                             sizeof(uint32_t));
        buf = (uint32_t *)exr_calloc(a, buf_count ? buf_count : 1u,
                                     sizeof(uint32_t));
        if (!use_frwd16 && !use_frwd32)
            magsgn_bits = (uint64_t *)exr_calloc(a, magsgn_words,
                                                 sizeof(uint64_t));
        sigprop_bits = (uint64_t *)exr_calloc(a,
                                              jph_magsgn_words(lengths2),
                                              sizeof(uint64_t));
    }
    if (!scratch || !v_n_scratch || !buf ||
        (!use_frwd16 && !use_frwd32 && !magsgn_bits) || !sigprop_bits) {
        rc = EXR_ERROR_OUT_OF_MEMORY;
        goto done;
    }

    exr_jph_mel_init(&mel, seg->data + lcup - scup, (size_t)scup - 1u);
    rc = jph_vlc_rev_init(&vlc, seg->data, lcup, scup);
    if (rc != EXR_SUCCESS) goto done;
#if defined(EXR_X86)
    if (use_frwd16 || use_frwd32) {
        jph_frwd_init_ff_avx2(&magsgn_frwd, seg->data, (int)(lcup - scup));
    } else
#endif
    {
        rc = jph_magsgn_init(&magsgn, seg->data, lcup - scup, magsgn_bits);
        if (rc != EXR_SUCCESS) goto done;
    }

    if (exr_jph_mel_get_run(&mel, &run, &i) != EXR_SUCCESS) i = 0;
    if (i) run = (run << 1) | 1u;
    else run = run << 1;
    (void)i;

    /* Initial row of quads */
    c_q = 0u;
    sp = scratch;
    {
        uint32_t x = 0u;
        while (x < width) {
            uint16_t t0, t1;
            uint32_t uvlc_mode, uvlc_entry, len, tmp;
            uint32_t q0_suffix_len, q0, q1;

            vlc_val = jph_vlc_rev_fetch(&vlc);
            t0 = htab->vlc_tbl0[c_q + (vlc_val & 0x7Fu)];

            if (c_q == 0u) {
                run -= 2u;
                t0 = (run == UINT32_MAX) ? t0 : 0u;
                if ((int32_t)run < 0) {
                    if (exr_jph_mel_get_run(&mel, &run, &i) != EXR_SUCCESS) i = 0;
                    if (i) run = (run << 1) | 1u;
                    else run = run << 1;
                }
            }
            sp[0] = t0;
            x += 2u;
            c_q = ((t0 & 0x10u) << 3u) | ((t0 & 0xE0u) << 2u);
            vlc_val = jph_vlc_rev_advance(&vlc, t0 & 0x7u);

            t1 = 0u;
            vlc_val = jph_vlc_rev_fetch(&vlc);
            t1 = htab->vlc_tbl0[c_q + (vlc_val & 0x7Fu)];
            if (c_q == 0u && x < width) {
                run -= 2u;
                t1 = (run == UINT32_MAX) ? t1 : 0u;
                if ((int32_t)run < 0) {
                    if (exr_jph_mel_get_run(&mel, &run, &i) != EXR_SUCCESS) i = 0;
                    if (i) run = (run << 1) | 1u;
                    else run = run << 1;
                }
            }
            t1 = (x < width) ? t1 : 0u;
            sp[2] = t1;
            x += 2u;
            c_q = ((t1 & 0x10u) << 3u) | ((t1 & 0xE0u) << 2u);
            vlc_val = jph_vlc_rev_advance(&vlc, t1 & 0x7u);

            uvlc_mode = ((t0 & 0x8u) << 3u) | ((t1 & 0x8u) << 4u);
            if (uvlc_mode == 0xC0u) {
                run -= 2u;
                uvlc_mode += (run == UINT32_MAX) ? 0x40u : 0u;
                if ((int32_t)run < 0) {
                    if (exr_jph_mel_get_run(&mel, &run, &i) != EXR_SUCCESS) i = 0;
                    if (i) run = (run << 1) | 1u;
                    else run = run << 1;
                }
            }
            uvlc_entry = htab->uvlc_tbl0[uvlc_mode + (vlc_val & 0x3Fu)];
            vlc_val = jph_vlc_rev_advance(&vlc, uvlc_entry & 0x7u);
            uvlc_entry >>= 3u;
            len = uvlc_entry & 0xFu;
            tmp = vlc_val & ((1u << len) - 1u);
            vlc_val = jph_vlc_rev_advance(&vlc, len);
            uvlc_entry >>= 4u;
            q0_suffix_len = uvlc_entry & 0x7u;
            uvlc_entry >>= 3u;
            /* Quad 0 gets the low q0_suffix_len bits of tmp; quad 1 gets the
             * high (remaining) bits. OpenJPH uses ~(0xFFU<<len) as mask. */
            q0 = (uvlc_entry & 0x7u) + (tmp & ((1u << q0_suffix_len) - 1u));
            sp[1] = (uint16_t)(1u + q0);
            q1 = (uvlc_entry >> 3u) + (tmp >> q0_suffix_len);
            sp[3] = (uint16_t)(1u + q1);
            sp += 4u;
        }
    }
    memset(sp, 0, 8u * sizeof(uint16_t));

    /* Non-initial rows of quads */
    {
        uint32_t y;
        for (y = 2u; y < height; y += 2u) {
            uint32_t x = 0u;
            sp = scratch + (y >> 1u) * sstr;
            c_q = 0u;
            while (x < width) {
                uint16_t t0, t1;
                uint32_t uvlc_entry, len, tmp;
                uint32_t q0, q1, q0_suffix_len;

                c_q |= ((sp[0 - (int32_t)sstr] & 0xA0u) << 2u);
                c_q |= ((sp[2 - (int32_t)sstr] & 0x20u) << 4u);

                vlc_val = jph_vlc_rev_fetch(&vlc);
                t0 = htab->vlc_tbl1[c_q + (vlc_val & 0x7Fu)];

                if (c_q == 0u) {
                    run -= 2u;
                    t0 = (run == UINT32_MAX) ? t0 : 0u;
                    if ((int32_t)run < 0) {
                        if (exr_jph_mel_get_run(&mel, &run, &i) !=
                            EXR_SUCCESS)
                            i = 0;
                        if (i) run = (run << 1) | 1u;
                        else run = run << 1;
                    }
                }
                sp[0] = t0;
                x += 2u;

                c_q = ((t0 & 0x40u) << 2u) | ((t0 & 0x80u) << 1u);
                c_q |= sp[0 - (int32_t)sstr] & 0x80u;
                c_q |= ((sp[2 - (int32_t)sstr] & 0xA0u) << 2u);
                c_q |= ((sp[4 - (int32_t)sstr] & 0x20u) << 4u);

                vlc_val = jph_vlc_rev_advance(&vlc, t0 & 0x7u);

                t1 = 0u;
                vlc_val = jph_vlc_rev_fetch(&vlc);
                t1 = htab->vlc_tbl1[c_q + (vlc_val & 0x7Fu)];
                if (c_q == 0u && x < width) {
                    run -= 2u;
                    t1 = (run == UINT32_MAX) ? t1 : 0u;
                    if ((int32_t)run < 0) {
                        if (exr_jph_mel_get_run(&mel, &run, &i) !=
                            EXR_SUCCESS)
                            i = 0;
                        if (i) run = (run << 1) | 1u;
                        else run = run << 1;
                    }
                }
                t1 = (x < width) ? t1 : 0u;
                sp[2] = t1;
                x += 2u;

                c_q = ((t1 & 0x40u) << 2u) | ((t1 & 0x80u) << 1u);
                c_q |= sp[2 - (int32_t)sstr] & 0x80u;
                vlc_val = jph_vlc_rev_advance(&vlc, t1 & 0x7u);

                {
                    uint32_t uvlc_mode =
                        ((t0 & 0x8u) << 3u) | ((t1 & 0x8u) << 4u);
                            uvlc_entry = htab->uvlc_tbl1[uvlc_mode + (vlc_val & 0x3Fu)];
                    vlc_val = jph_vlc_rev_advance(&vlc, uvlc_entry & 0x7u);
                    uvlc_entry >>= 3u;
                    len = uvlc_entry & 0xFu;
                    tmp = vlc_val & ((1u << len) - 1u);
                    vlc_val = jph_vlc_rev_advance(&vlc, len);
                    uvlc_entry >>= 4u;
                    q0_suffix_len = uvlc_entry & 0x7u;
                    uvlc_entry >>= 3u;
                    q0 = (uvlc_entry & 0x7u) +
                         (tmp & ((1u << q0_suffix_len) - 1u));
                    sp[1] = (uint16_t)q0;
                    q1 = (uvlc_entry >> 3u) + (tmp >> q0_suffix_len);
                    sp[3] = (uint16_t)q1;
                }
                sp += 4u;
            }
            memset(sp, 0, 8u * sizeof(uint16_t));
        }
    }

    /* Step 2: decode MagSgn for the initial row */
    {
        uint32_t x = 0u;
        uint32_t *vp = v_n_scratch;
        prev_v_n = 0;
        sp = scratch;
        dp = buf;
#if defined(EXR_X86)
        if (use_frwd32) {
            while (x < width) {
                uint32_t u_q[2];
                uint32_t bottom_vn[4];
                u_q[0] = sp[1];
                u_q[1] = sp[3];
                if (u_q[0] > mmsbp2 || u_q[1] > mmsbp2) {
                    rc = EXR_ERROR_CORRUPT;
                    goto done;
                }
                jph_decode_two_quad32_frwd_avx2(dp, dp + sstr, bottom_vn, sp,
                                                u_q, &magsgn_frwd, p);
                for (uint32_t qn = 0u; qn < 2u; ++qn) {
                    vp[0] = (uint32_t)(prev_v_n | bottom_vn[2u * qn]);
                    prev_v_n = bottom_vn[2u * qn + 1u];
                    ++vp;
                }
                dp += 4u;
                x += 4u;
                sp += 4u;
            }
        } else if (use_frwd16) {
            while (x < width) {
                uint32_t u_q[4];
                uint16_t bottom_vn[8];
                u_q[0] = sp[1];
                u_q[1] = sp[3];
                u_q[2] = sp[5];
                u_q[3] = sp[7];
                if (u_q[0] > mmsbp2 || u_q[1] > mmsbp2 ||
                    u_q[2] > mmsbp2 || u_q[3] > mmsbp2) {
                    rc = EXR_ERROR_CORRUPT;
                    goto done;
                }
                jph_decode_four_quad16_frwd_avx2(dp, dp + sstr, bottom_vn, sp,
                                                 u_q, &magsgn_frwd, p - 16u);
                for (uint32_t qn = 0u; qn < 4u; ++qn) {
                    vp[0] = (uint32_t)(prev_v_n | bottom_vn[2u * qn]);
                    prev_v_n = bottom_vn[2u * qn + 1u];
                    ++vp;
                }
                dp += 8u;
                x += 8u;
                sp += 8u;
            }
        } else if (use_four16 && height > 1u) {
            while (x + 8u <= width) {
                uint32_t u_q[4];
                uint16_t bottom_vn[8];
                u_q[0] = sp[1];
                u_q[1] = sp[3];
                u_q[2] = sp[5];
                u_q[3] = sp[7];
                if (u_q[0] > mmsbp2 || u_q[1] > mmsbp2 ||
                    u_q[2] > mmsbp2 || u_q[3] > mmsbp2) {
                    rc = EXR_ERROR_CORRUPT;
                    goto done;
                }
                jph_decode_four_quad16_avx2(dp, dp + sstr, bottom_vn, sp, u_q,
                                            magsgn.buf, magsgn.real_bits,
                                            &magsgn.cursor, p - 16u);
                for (uint32_t qn = 0u; qn < 4u; ++qn) {
                    vp[0] = (uint32_t)(prev_v_n | bottom_vn[2u * qn]);
                    prev_v_n = bottom_vn[2u * qn + 1u];
                    ++vp;
                }
                dp += 8u;
                x += 8u;
                sp += 8u;
            }
        }
#endif /* EXR_X86 */
        while (x < width) {
            uint32_t inf = sp[0];
            uint32_t U_q = sp[1];
            uint32_t v_n = 0u, ignore;
            JphQuadMs q;

            if (U_q > mmsbp2) {
                rc = EXR_ERROR_CORRUPT;
                goto done;
            }
            jph_quad_ms_begin(&q, &magsgn);
            dp[0] = jph_quad_ms_sample(&q, &magsgn, inf, 0u, U_q, p, &ignore);
            {
                uint32_t val =
                    jph_quad_ms_sample(&q, &magsgn, inf, 1u, U_q, p, &v_n);
                if (1u < height) dp[sstr] = val;
            }
            vp[0] = (uint32_t)(prev_v_n | v_n);
            prev_v_n = 0;
            ++dp;
            ++x;
            if (x >= width) {
                jph_quad_ms_end(&q, &magsgn);
                ++vp;
                break;
            }
            dp[0] = jph_quad_ms_sample(&q, &magsgn, inf, 2u, U_q, p, &ignore);
            {
                uint32_t val =
                    jph_quad_ms_sample(&q, &magsgn, inf, 3u, U_q, p, &v_n);
                if (1u < height) dp[sstr] = val;
            }
            jph_quad_ms_end(&q, &magsgn);
            prev_v_n = v_n;
            ++dp;
            ++x;
            sp += 2u;
            ++vp;
        }
        vp[0] = (uint32_t)prev_v_n;
    }

    /* Non-initial rows of MagSgn */
    {
        uint32_t y;
        for (y = 2u; y < height; y += 2u) {
            uint32_t x = 0u;
            uint32_t *vp = v_n_scratch;
            prev_v_n = 0;
            sp = scratch + (y >> 1u) * sstr;
            dp = buf + y * sstr;
#if defined(EXR_X86)
            if (use_frwd32) {
                while (x < width) {
                    uint32_t u_q[2];
                    uint32_t bottom_vn[4];
                    for (uint32_t qn = 0u; qn < 2u; ++qn) {
                        uint32_t inf = sp[2u * qn];
                        uint32_t gamma = inf & 0xF0u;
                        uint32_t emax_src = vp[qn] | vp[qn + 1u] | 2u;
                        uint32_t emax = 31u - (uint32_t)jph_clz32(emax_src);
                        uint32_t kappa;
                        gamma &= gamma - 0x10u;
                        kappa = gamma ? emax : 1u;
                        u_q[qn] = (uint32_t)sp[2u * qn + 1u] + kappa;
                    }
                    if (u_q[0] > mmsbp2 || u_q[1] > mmsbp2) {
                        rc = EXR_ERROR_CORRUPT;
                        goto done;
                    }
                    jph_decode_two_quad32_frwd_avx2(dp, dp + sstr, bottom_vn,
                                                    sp, u_q, &magsgn_frwd, p);
                    for (uint32_t qn = 0u; qn < 2u; ++qn) {
                        vp[0] = (uint32_t)(prev_v_n | bottom_vn[2u * qn]);
                        prev_v_n = bottom_vn[2u * qn + 1u];
                        ++vp;
                    }
                    dp += 4u;
                    x += 4u;
                    sp += 4u;
                }
            } else if (use_frwd16) {
                while (x < width) {
                    uint32_t u_q[4];
                    uint16_t bottom_vn[8];
                    for (uint32_t qn = 0u; qn < 4u; ++qn) {
                        uint32_t inf = sp[2u * qn];
                        uint32_t gamma = inf & 0xF0u;
                        uint32_t emax_src = vp[qn] | vp[qn + 1u] | 2u;
                        uint32_t emax = 31u - (uint32_t)jph_clz32(emax_src);
                        uint32_t kappa;
                        gamma &= gamma - 0x10u;
                        kappa = gamma ? emax : 1u;
                        u_q[qn] = (uint32_t)sp[2u * qn + 1u] + kappa;
                    }
                    if (u_q[0] > mmsbp2 || u_q[1] > mmsbp2 ||
                        u_q[2] > mmsbp2 || u_q[3] > mmsbp2) {
                        rc = EXR_ERROR_CORRUPT;
                        goto done;
                    }
                    jph_decode_four_quad16_frwd_avx2(dp, dp + sstr, bottom_vn,
                                                     sp, u_q, &magsgn_frwd,
                                                     p - 16u);
                    for (uint32_t qn = 0u; qn < 4u; ++qn) {
                        vp[0] = (uint32_t)(prev_v_n | bottom_vn[2u * qn]);
                        prev_v_n = bottom_vn[2u * qn + 1u];
                        ++vp;
                    }
                    dp += 8u;
                    x += 8u;
                    sp += 8u;
                }
            } else if (use_four16 && y + 1u < height) {
                while (x + 8u <= width) {
                    uint32_t u_q[4];
                    uint16_t bottom_vn[8];
                    for (uint32_t qn = 0u; qn < 4u; ++qn) {
                        uint32_t inf = sp[2u * qn];
                        uint32_t gamma = inf & 0xF0u;
                        uint32_t emax_src = vp[qn] | vp[qn + 1u] | 2u;
                        uint32_t emax = 31u - (uint32_t)jph_clz32(emax_src);
                        uint32_t kappa;
                        gamma &= gamma - 0x10u;
                        kappa = gamma ? emax : 1u;
                        u_q[qn] = (uint32_t)sp[2u * qn + 1u] + kappa;
                    }
                    if (u_q[0] > mmsbp2 || u_q[1] > mmsbp2 ||
                        u_q[2] > mmsbp2 || u_q[3] > mmsbp2) {
                        rc = EXR_ERROR_CORRUPT;
                        goto done;
                    }
                    jph_decode_four_quad16_avx2(dp, dp + sstr, bottom_vn, sp,
                                                u_q, magsgn.buf,
                                                magsgn.real_bits,
                                                &magsgn.cursor, p - 16u);
                    for (uint32_t qn = 0u; qn < 4u; ++qn) {
                        vp[0] = (uint32_t)(prev_v_n | bottom_vn[2u * qn]);
                        prev_v_n = bottom_vn[2u * qn + 1u];
                        ++vp;
                    }
                    dp += 8u;
                    x += 8u;
                    sp += 8u;
                }
            }
#endif /* EXR_X86 */
            while (x < width) {
                uint32_t inf = sp[0];
                uint32_t u_q = sp[1];
                uint32_t gamma = inf & 0xF0u;
                gamma &= gamma - 0x10u;
                uint32_t emax = vp[0] | vp[1];
                int lz = 0;
                uint32_t v, U_q, kappa;
                v = emax | 2u;
                if ((v & 0xFFFF0000u) == 0u) {
                    lz += 16u;
                    v <<= 16u;
                }
                if ((v & 0xFF000000u) == 0u) {
                    lz += 8u;
                    v <<= 8u;
                }
                if ((v & 0xF0000000u) == 0u) {
                    lz += 4u;
                    v <<= 4u;
                }
                if ((v & 0xC0000000u) == 0u) {
                    lz += 2u;
                    v <<= 2u;
                }
                if ((v & 0x80000000u) == 0u) lz += 1u;
                emax = 31u - (uint32_t)lz;
                /* gamma is 0 when the quad's first sample is significant; in
                 * that case kappa is 1, otherwise kappa is emax. */
                kappa = gamma ? emax : 1u;
                U_q = u_q + kappa;
                if (U_q > mmsbp2) {
                    rc = EXR_ERROR_CORRUPT;
                    goto done;
                }

                {
                    uint32_t v_n = 0u, ignore;
                    JphQuadMs q;
                    jph_quad_ms_begin(&q, &magsgn);
                    dp[0] = jph_quad_ms_sample(&q, &magsgn, inf, 0u, U_q, p,
                                               &ignore);
                    {
                        uint32_t val = jph_quad_ms_sample(&q, &magsgn, inf, 1u,
                                                          U_q, p, &v_n);
                        if (y + 1u < height) dp[sstr] = val;
                    }
                    vp[0] = (uint32_t)(prev_v_n | v_n);
                    prev_v_n = 0;
                    ++dp;
                    ++x;
                    if (x >= width) {
                        jph_quad_ms_end(&q, &magsgn);
                        ++vp;
                        break;
                    }
                    dp[0] = jph_quad_ms_sample(&q, &magsgn, inf, 2u, U_q, p,
                                               &ignore);
                    {
                        uint32_t val = jph_quad_ms_sample(&q, &magsgn, inf, 3u,
                                                          U_q, p, &v_n);
                        if (y + 1u < height) dp[sstr] = val;
                    }
                    jph_quad_ms_end(&q, &magsgn);
                    prev_v_n = v_n;
                    ++dp;
                    ++x;
                }
                sp += 2u;
                ++vp;
            }
            vp[0] = (uint32_t)prev_v_n;
        }
    }

    if (num_passes > 1u) {
        uint16_t *sigma = scratch;
        uint32_t mstr = (width + 3u) >> 2u;
        uint16_t prev_row_sig[256u + 8u];
        JphMagSgn sigprop;

        mstr = ((mstr + 2u) + 7u) & ~7u;
        if ((size_t)mstr * (((height + 3u) >> 2u) + 1u) > scratch_count) {
            rc = EXR_ERROR_CORRUPT;
            goto done;
        }

        {
            uint32_t y;
            for (y = 0u; y < height; y += 4u) {
                uint16_t *ssp = scratch + (y >> 1u) * sstr;
                uint16_t *sdp = sigma + (y >> 2u) * mstr;
                uint32_t x;
                for (x = 0u; x < width; x += 4u, ssp += 4u, ++sdp) {
                    uint32_t t0 = 0u, t1 = 0u;
                    t0 = ((ssp[0] & 0x30u) >> 4u) |
                         ((ssp[0] & 0xc0u) >> 2u);
                    t0 |= ((ssp[2] & 0x30u) << 4u) |
                          ((ssp[2] & 0xc0u) << 6u);
                    t1 = ((ssp[sstr] & 0x30u) >> 2u) |
                         (ssp[sstr] & 0xc0u);
                    t1 |= ((ssp[2u + sstr] & 0x30u) << 6u) |
                          ((ssp[2u + sstr] & 0xc0u) << 8u);
                    sdp[0] = (uint16_t)(t0 | t1);
                }
                sdp[0] = 0u;
            }
            {
                uint16_t *sdp = sigma + (y >> 2u) * mstr;
                uint32_t x;
                for (x = 0u; x < width; x += 4u, ++sdp) sdp[0] = 0u;
                sdp[0] = 0u;
            }
        }

        memset(prev_row_sig, 0, sizeof(prev_row_sig));
        rc = jph_forward_bits_init(&sigprop, seg->data + lengths1, lengths2, 0u,
                                   sigprop_bits);
        if (rc != EXR_SUCCESS) goto done;

        {
            uint32_t y;
            for (y = 0u; y < height; y += 4u) {
                uint32_t row_pattern = 0xffffu;
                uint32_t prev = 0u;
                uint16_t *prev_sig = prev_row_sig;
                uint16_t *cur_sig = sigma + (y >> 2u) * mstr;
                uint32_t *dpp = buf + y * sstr;
                if (height - y < 4u) {
                    row_pattern = 0x7777u;
                    if (height - y < 3u) {
                        row_pattern = 0x3333u;
                        if (height - y < 2u) row_pattern = 0x1111u;
                    }
                }

                for (uint32_t x = 0u; x < width; x += 4u, ++cur_sig, ++prev_sig) {
                    uint32_t s = x + 4u - width;
                    uint32_t pattern = row_pattern >> ((s < 4u ? s : 0u) * 4u);
                    uint32_t ps = jph_load_u32_from_u16(prev_sig);
                    uint32_t ns = jph_load_u32_from_u16(cur_sig + mstr);
                    uint32_t u = (ps & 0x88888888u) >> 3u;
                    uint32_t cs = jph_load_u32_from_u16(cur_sig);
                    uint32_t mbr, t, new_sig;

                    u |= (ns & 0x11111111u) << 3u;
                    mbr = cs;
                    mbr |= (cs & 0x77777777u) << 1u;
                    mbr |= (cs & 0xeeeeeeeeu) >> 1u;
                    mbr |= u;
                    t = mbr;
                    mbr |= t << 4u;
                    mbr |= t >> 4u;
                    mbr |= prev >> 12u;
                    mbr &= pattern;
                    mbr &= ~cs;

                    new_sig = mbr;
                    if (new_sig) {
                        uint32_t cwd = jph_magsgn_fetch(&sigprop);
                        uint32_t cnt = 0u;
                        uint32_t col_mask = 0x0fu;
                        uint32_t inv_sig = ~cs & pattern;
                        for (int bi = 0; bi < 16; bi += 4, col_mask <<= 4u) {
                            uint32_t sample_mask;
                            if ((col_mask & new_sig) == 0u) continue;

                            sample_mask = 0x1111u & col_mask;
                            if (new_sig & sample_mask) {
                                new_sig &= ~sample_mask;
                                if (cwd & 1u) new_sig |= (0x33u << bi) & inv_sig;
                                cwd >>= 1u;
                                ++cnt;
                            }
                            sample_mask <<= 1u;
                            if (new_sig & sample_mask) {
                                new_sig &= ~sample_mask;
                                if (cwd & 1u) new_sig |= (0x76u << bi) & inv_sig;
                                cwd >>= 1u;
                                ++cnt;
                            }
                            sample_mask <<= 1u;
                            if (new_sig & sample_mask) {
                                new_sig &= ~sample_mask;
                                if (cwd & 1u) new_sig |= (0xecu << bi) & inv_sig;
                                cwd >>= 1u;
                                ++cnt;
                            }
                            sample_mask <<= 1u;
                            if (new_sig & sample_mask) {
                                new_sig &= ~sample_mask;
                                if (cwd & 1u) new_sig |= (0xc8u << bi) & inv_sig;
                                cwd >>= 1u;
                                ++cnt;
                            }
                        }

                        if (new_sig) {
                            uint32_t *rdp = dpp + x;
                            uint32_t val = 3u << (p - 2u);
                            col_mask = 0x0fu;
                            for (int bi = 0; bi < 4; ++bi, ++rdp, col_mask <<= 4u) {
                                uint32_t sample_mask;
                                if ((col_mask & new_sig) == 0u) continue;

                                sample_mask = 0x1111u & col_mask;
                                if (new_sig & sample_mask) {
                                    rdp[0] = (cwd << 31u) | val;
                                    cwd >>= 1u;
                                    ++cnt;
                                }
                                sample_mask <<= 1u;
                                if ((new_sig & sample_mask) && y + 1u < height) {
                                    rdp[sstr] = (cwd << 31u) | val;
                                    cwd >>= 1u;
                                    ++cnt;
                                }
                                sample_mask <<= 1u;
                                if ((new_sig & sample_mask) && y + 2u < height) {
                                    rdp[2u * sstr] = (cwd << 31u) | val;
                                    cwd >>= 1u;
                                    ++cnt;
                                }
                                sample_mask <<= 1u;
                                if ((new_sig & sample_mask) && y + 3u < height) {
                                    rdp[3u * sstr] = (cwd << 31u) | val;
                                    cwd >>= 1u;
                                    ++cnt;
                                }
                            }
                        }
                        jph_magsgn_advance(&sigprop, cnt);
                    }

                    new_sig |= cs;
                    *prev_sig = (uint16_t)new_sig;
                    t = new_sig;
                    new_sig |= (t & 0x7777u) << 1u;
                    new_sig |= (t & 0xeeeeu) >> 1u;
                    prev = (new_sig | u) & 0xf000u;
                }
            }
        }

        if (num_passes > 2u) {
            JphVlcRev magref;
            rc = jph_mrp_rev_init(&magref, seg->data, lengths1, lengths2);
            if (rc != EXR_SUCCESS) goto done;

            for (uint32_t y = 0u; y < height; y += 4u) {
                uint16_t *cur_sig = sigma + (y >> 2u) * mstr;
                uint32_t *dpp = buf + y * sstr;
                uint32_t half = 1u << (p - 2u);
                for (uint32_t x = 0u; x < width; x += 8u, cur_sig += 2u) {
                    uint32_t cwd = jph_mrp_rev_fetch(&magref);
                    uint32_t sig = jph_load_u32_from_u16(cur_sig);
                    uint32_t col_mask = 0x0fu;
                    if (sig) {
                        for (int j = 0; j < 8; ++j, col_mask <<= 4u) {
                            uint32_t sample_mask;
                            uint32_t *rdp;
                            if ((sig & col_mask) == 0u) continue;
                            rdp = dpp + x + (uint32_t)j;
                            sample_mask = 0x11111111u & col_mask;
                            for (int k = 0; k < 4; ++k) {
                                if (sig & sample_mask) {
                                    uint32_t sym = cwd & 1u;
                                    sym = (1u - sym) << (p - 1u);
                                    sym |= half;
                                    rdp[0] ^= sym;
                                    cwd >>= 1u;
                                }
                                sample_mask <<= 1u;
                                rdp += sstr;
                            }
                        }
                    }
                    (void)jph_vlc_rev_advance(&magref, jph_popcount32(sig));
                }
            }
        }
    }

    /* Convert OpenJPH's sign/magnitude codeblock words back to signed
     * reversible-transform coefficients. */
    {
        uint32_t y;
        uint32_t shift = (kmax < 31u) ? 31u - kmax : 0u;
        for (y = 0u; y < height; ++y) {
            const uint32_t *brow = buf + (size_t)y * sstr;
            int64_t *orow64 = out64 ? out64 + (size_t)y * out_stride : NULL;
            int32_t *orow32 = out32 ? out32 + (size_t)y * out_stride : NULL;
            uint32_t x;
#if defined(EXR_X86)
            if (use_avx2 && orow64) {
                jph_extract_signmag_i32_to_i64_avx2(orow64, brow, width, shift);
                continue;
            }
            if (use_avx2 && orow32) {
                jph_extract_signmag_i32_to_i32_avx2(orow32, brow, width, shift);
                continue;
            }
#elif defined(EXR_NEON)
            if (orow64) {
                jph_extract_signmag_i32_to_i64_neon(orow64, brow, width, shift);
                continue;
            }
#endif
            for (x = 0u; x < width; ++x) {
                uint32_t v = brow[x];
                int32_t mag = (int32_t)((v & 0x7fffffffu) >> shift);
                int32_t outv = (v & 0x80000000u) ? -mag : mag;
                if (orow32) orow32[x] = outv;
                else orow64[x] = outv;
            }
        }
    }

    rc = EXR_SUCCESS;

done:
    if (!ws) {
        exr_free(a, scratch);
        exr_free(a, v_n_scratch);
        exr_free(a, buf);
        exr_free(a, magsgn_bits);
        exr_free(a, sigprop_bits);
    }
    return rc;
}

static exr_result jph_decode_block(const JphCodeblockSeg *seg,
                                    const JphHtTables *htab,
                                    int64_t *out, uint32_t out_stride,
                                    uint32_t kmax, const exr_allocator *a) {
    int use_avx2 = 0;
#if defined(EXR_X86)
    use_avx2 = (exr_cpu_caps() & EXR_SIMD_AVX2) != 0;
#endif
    return jph_decode_block_core(seg, htab, out, NULL, out_stride, kmax, a,
                                 NULL, use_avx2);
}

static exr_result jph_decode_block_i32(const JphCodeblockSeg *seg,
                                       const JphHtTables *htab,
                                       int32_t *out, uint32_t out_stride,
                                       uint32_t kmax, const exr_allocator *a,
                                       JphDecodeWorkspace *ws, int use_avx2) {
    return jph_decode_block_core(seg, htab, NULL, out, out_stride, kmax, a, ws,
                                 use_avx2);
}

/* Subband-to-component-plane coordinate mapping. */
static exr_result jph_subband_origin(const JphCodeblockSeg *seg,
                                     const JphBandGeom *band,
                                     const JphPlaneD *plane,
                                     uint32_t num_decomps,
                                     uint32_t *out_row_off,
                                     uint32_t *out_col_off) {
    uint32_t row_off, col_off;
    if (!seg || !band || !plane || !out_row_off || !out_col_off ||
        (!plane->d32 && !plane->d64)) {
        return EXR_ERROR_INVALID_ARGUMENT;
    }
    if (seg->band == 0u) {
        if (seg->res != 0u) return EXR_ERROR_CORRUPT;
        row_off = 0u;
        col_off = 0u;
    } else {
        JphSize comp_size, rs;
        if (seg->res == 0u) return EXR_ERROR_CORRUPT;
        comp_size.w = plane->w;
        comp_size.h = plane->h;
        rs = jph_resolution_size(comp_size, num_decomps, seg->res);
        if (seg->band == 1u) {
            row_off = 0u;
            col_off = (rs.w + 1u) >> 1u;
        } else if (seg->band == 2u) {
            row_off = (rs.h + 1u) >> 1u;
            col_off = 0u;
        } else if (seg->band == 3u) {
            row_off = (rs.h + 1u) >> 1u;
            col_off = (rs.w + 1u) >> 1u;
        } else {
            return EXR_ERROR_CORRUPT;
        }
    }
    if (seg->x0 > band->w || seg->y0 > band->h) return EXR_ERROR_CORRUPT;
    if (seg->width > band->w - seg->x0) return EXR_ERROR_CORRUPT;
    if (seg->height > band->h - seg->y0) return EXR_ERROR_CORRUPT;
    if (row_off + seg->y0 > plane->h ||
        seg->height > plane->h - (row_off + seg->y0))
        return EXR_ERROR_CORRUPT;
    if (col_off + seg->x0 > plane->w ||
        seg->width > plane->w - (col_off + seg->x0))
        return EXR_ERROR_CORRUPT;

    *out_row_off = row_off;
    *out_col_off = col_off;
    return EXR_SUCCESS;
}

static exr_result jph_subband_to_plane(const JphCodeblockSeg *seg,
                                       const JphBandGeom *band,
                                       const int64_t *cb, uint32_t cb_stride,
                                       JphPlaneD *plane,
                                       uint32_t num_decomps) {
    uint32_t row_off, col_off;
    uint32_t x, y;
    exr_result rc;
    if (!cb) return EXR_ERROR_INVALID_ARGUMENT;
    rc = jph_subband_origin(seg, band, plane, num_decomps, &row_off, &col_off);
    if (rc != EXR_SUCCESS) return rc;

    for (y = 0u; y < seg->height; ++y) {
        uint32_t g_row = row_off + seg->y0 + y;
        for (x = 0u; x < seg->width; ++x) {
            uint32_t g_col = col_off + seg->x0 + x;
            int64_t v = cb[y * cb_stride + x];
            /* All-half coefficients fit int32 (block32 path, kmax<=30). */
            if (plane->d32) plane->d32[g_row * plane->w + g_col] = (int32_t)v;
            else plane->d64[g_row * plane->w + g_col] = v;
        }
    }
    return EXR_SUCCESS;
}

static exr_result jph_decode_codeblock(void *user,
                                       const JphCodeblockSeg *seg) {
    JphDecodeState *st = (JphDecodeState *)user;
    JphBandGeom bands[4];
    JphHtTables *htab = NULL;
    int64_t *cb = NULL;
    uint32_t cb_stride, cb_size;
    exr_result rc;
    if (!st || !seg || !seg->data || seg->data_size == 0)
        return EXR_ERROR_CORRUPT;
    if (!st->jp || seg->comp >= st->num_planes || !st->planes ||
        (!st->planes[seg->comp].d32 && !st->planes[seg->comp].d64))
        return EXR_ERROR_CORRUPT;
    if (seg->width == 0u || seg->height == 0u) return EXR_ERROR_CORRUPT;
    if (seg->width > 128u || seg->height > 32u) return EXR_ERROR_CORRUPT;
    rc = jph_build_band_geoms(st->jp, seg->comp, seg->res, bands, NULL);
    if (rc != EXR_SUCCESS) return rc;
    if (seg->band >= 4u || !bands[seg->band].exists)
        return EXR_ERROR_CORRUPT;
    if (seg->x0 > bands[seg->band].w || seg->y0 > bands[seg->band].h ||
        seg->width > bands[seg->band].w - seg->x0 ||
        seg->height > bands[seg->band].h - seg->y0)
        return EXR_ERROR_CORRUPT;
    rc = jph_ht_ensure_tables(&htab);
    if (rc != EXR_SUCCESS) return rc;

    cb_stride = (seg->width + 7u) & ~7u;
    if (cb_stride == 0u || cb_stride > 128u) return EXR_ERROR_CORRUPT;
    {
        size_t cb_bytes = 0;
        if (exr_mul_ovf((size_t)cb_stride, (size_t)seg->height, &cb_bytes))
            return EXR_ERROR_CORRUPT;
        if (exr_mul_ovf(cb_bytes, sizeof(int64_t), &cb_bytes))
            return EXR_ERROR_CORRUPT;
        cb_size = (uint32_t)cb_bytes;
    }
    if (st->planes[seg->comp].d32 && seg->missing_msbs < 30u &&
        bands[seg->band].kmax <= 30u) {
        uint32_t row_off, col_off;
        JphPlaneD *plane = &st->planes[seg->comp];
        int32_t *dst;
        rc = jph_subband_origin(seg, &bands[seg->band], plane,
                                st->jp->num_decomps, &row_off, &col_off);
        if (rc != EXR_SUCCESS) return rc;
        dst = plane->d32 + (size_t)(row_off + seg->y0) * plane->w +
              (col_off + seg->x0);
        rc = jph_decode_block_i32(seg, htab, dst, plane->w,
                                  bands[seg->band].kmax, st->a, &st->ws,
                                  st->use_avx2);
        if (rc != EXR_SUCCESS) return rc;
        if (exr_add_ovf(st->codeblocks, 1u, &st->codeblocks))
            return EXR_ERROR_CORRUPT;
        return EXR_SUCCESS;
    }

    cb = (int64_t *)exr_calloc(st->a ? st->a : exr_default_allocator(),
                               cb_size ? cb_size : 1u, 1);
    if (!cb) return EXR_ERROR_OUT_OF_MEMORY;
    rc = jph_decode_block(seg, htab, cb, cb_stride, bands[seg->band].kmax,
                          st->a);
    if (rc != EXR_SUCCESS) {
        exr_free(st->a ? st->a : exr_default_allocator(), cb);
        return rc;
    }
    rc = jph_subband_to_plane(seg, &bands[seg->band], cb, cb_stride,
                              &st->planes[seg->comp], st->jp->num_decomps);
    exr_free(st->a ? st->a : exr_default_allocator(), cb);
    if (rc != EXR_SUCCESS) return rc;
    if (exr_add_ovf(st->codeblocks, 1u, &st->codeblocks))
        return EXR_ERROR_CORRUPT;
    return EXR_SUCCESS;
}

/* ---- GPU seam: code-block plan collector -------------------------------- */
typedef struct {
    const exr_allocator *a;
    const JphProfile *jp;
    JphPlaneD *planes;
    uint16_t num_planes;
    const uint8_t *tile_base;
    exr_jph_cb_record *records;
    size_t count;
    size_t cap;
    /* When gpu_fn is set, jph_decode_tile_payload decodes the collected blocks
     * on the GPU and continues to scatter + inverse transform + store, instead
     * of returning the plan to the caller. */
    exr_jph_gpu_block_decode_fn gpu_fn;
    void *gpu_user;
} JphCollectState;

static exr_result jph_collect_codeblock(void *user,
                                        const JphCodeblockSeg *seg) {
    JphCollectState *st = (JphCollectState *)user;
    JphBandGeom bands[4];
    JphPlaneD *plane;
    uint32_t row_off, col_off, kmax;
    exr_jph_cb_record *rec;
    exr_result rc;
    if (!st || !seg || !seg->data || seg->data_size == 0)
        return EXR_ERROR_CORRUPT;
    if (seg->comp >= st->num_planes) return EXR_ERROR_CORRUPT;
    if (seg->width == 0u || seg->height == 0u ||
        seg->width > 128u || seg->height > 32u)
        return EXR_ERROR_CORRUPT;
    plane = &st->planes[seg->comp];
    rc = jph_build_band_geoms(st->jp, seg->comp, seg->res, bands, NULL);
    if (rc != EXR_SUCCESS) return rc;
    if (seg->band >= 4u || !bands[seg->band].exists) return EXR_ERROR_CORRUPT;
    kmax = bands[seg->band].kmax;
    rc = jph_subband_origin(seg, &bands[seg->band], plane,
                            st->jp->num_decomps, &row_off, &col_off);
    if (rc != EXR_SUCCESS) return rc;

    if (st->count == st->cap) {
        size_t ncap = st->cap ? st->cap * 2u : 256u;
        exr_jph_cb_record *nr = (exr_jph_cb_record *)exr_malloc(
            st->a, ncap * sizeof(*nr));
        if (!nr) return EXR_ERROR_OUT_OF_MEMORY;
        if (st->records) {
            memcpy(nr, st->records, st->count * sizeof(*nr));
            exr_free(st->a, st->records);
        }
        st->records = nr;
        st->cap = ncap;
    }
    rec = &st->records[st->count++];
    rec->width = seg->width;
    rec->height = seg->height;
    rec->missing_msbs = seg->missing_msbs;
    rec->active_passes = seg->active_passes;
    rec->length0 = seg->length0;
    rec->length1 = seg->length1;
    rec->kmax = kmax;
    rec->comp = seg->comp;
    rec->res = seg->res;
    rec->band = seg->band;
    rec->x0 = seg->x0;
    rec->y0 = seg->y0;
    rec->dst_row = row_off + seg->y0;
    rec->dst_col = col_off + seg->x0;
    rec->plane_w = plane->w;
    rec->plane_h = plane->h;
    rec->data_offset = (size_t)(seg->data - st->tile_base);
    rec->data_size = seg->data_size;
    rec->i32_eligible =
        (plane->d32 != NULL && seg->missing_msbs < 30u && kmax <= 30u) ? 1 : 0;
    return EXR_SUCCESS;
}

static exr_result jph_parse_tile_packets(const exr_codec_ctx *ctx,
                                         const JphProfile *jp,
                                         size_t *out_codeblocks,
                                         JphCodeblockCallback cb,
                                         void *cb_user) {
    const exr_allocator *a = ctx->alloc ? ctx->alloc : exr_default_allocator();
    exr_jph_bitreader br;
    JphPrecinctState *states = NULL;
    size_t state_count, total_codeblocks = 0;
    uint32_t r;
    exr_result rc;

    if (out_codeblocks) *out_codeblocks = 0;
    if (jp->num_decomps > 31u) return EXR_ERROR_CORRUPT;
    if (exr_mul_ovf((size_t)jp->csiz, (size_t)jp->num_decomps + 1u,
                    &state_count))
        return EXR_ERROR_CORRUPT;
    states = (JphPrecinctState *)exr_calloc(a, state_count, sizeof(*states));
    if (!states) return EXR_ERROR_OUT_OF_MEMORY;
    rc = jph_init_precinct_states(jp, states);
    if (rc != EXR_SUCCESS) goto done;

    exr_jph_bitreader_init(&br, jp->tile_data, jp->tile_data_size);
    for (r = 0; r <= jp->num_decomps; ++r) {
        while (1) {
            uint32_t c, best_c = 0;
            uint64_t best_x = UINT64_MAX, best_y = UINT64_MAX;
            int found = 0;
            for (c = 0; c < jp->csiz; ++c) {
                JphPrecinctState *st =
                    states + (size_t)c * (jp->num_decomps + 1u) + r;
                uint64_t x = 0, y = 0;
                if (!jph_precinct_top_left(jp, c, r, st, &x, &y)) continue;
                if (!found || y < best_y || (y == best_y && x < best_x)) {
                    found = 1;
                    best_c = c;
                    best_x = x;
                    best_y = y;
                }
            }
            if (!found) break;
    if (br.p >= br.end) {
        rc = EXR_ERROR_CORRUPT;
        goto done;
    }
            {
                JphPrecinctState *st =
                    states + (size_t)best_c * (jp->num_decomps + 1u) + r;
                size_t packet_codeblocks = 0;
                rc = jph_parse_precinct_packet(a, jp, best_c, r, st->cur_x,
                                               st->cur_y, &br,
                                               &packet_codeblocks, cb,
                                               cb_user);
                if (rc != EXR_SUCCESS) goto done;
                if (exr_add_ovf(total_codeblocks, packet_codeblocks,
                                &total_codeblocks)) {
                    rc = EXR_ERROR_CORRUPT;
                    goto done;
                }
                jph_advance_precinct(st);
            }
        }
    }
    if (br.p != br.end) rc = EXR_ERROR_CORRUPT;
    else if (out_codeblocks) *out_codeblocks = total_codeblocks;

done:
    exr_free(a, states);
    return rc;
}

static exr_result jph_decode_tile_payload(const exr_codec_ctx *ctx,
                                          const JphProfile *jp,
                                          const uint16_t *map,
                                          uint8_t *dst, size_t dst_size,
                                          JphCollectState *collect) {
    size_t expected = 0;
    size_t codeblocks = 0;
    JphDecodeState decode_state;
    exr_result rc;

    (void)map;
    memset(&decode_state, 0, sizeof(decode_state));
    if (!jp->tile_data || jp->tile_data_size == 0) return EXR_ERROR_CORRUPT;
    rc = exr_block_uncompressed_size(ctx->channels, ctx->num_channels, ctx->x,
                                     ctx->y, ctx->width, ctx->num_lines,
                                     &expected);
    if (rc != EXR_SUCCESS) return rc;
    if (!collect && expected != dst_size) return EXR_ERROR_CORRUPT;
    if (jp->mc_trans != 0u && ctx->num_channels < 3) return EXR_ERROR_CORRUPT;
    decode_state.a = ctx->alloc ? ctx->alloc : exr_default_allocator();
    decode_state.jp = jp;
    decode_state.num_planes = jp->csiz;
    /* All-HALF components keep reversible coefficients within int32, so use the
     * narrower (and pre-float, proven) int32 coefficient pipeline; any 32-bit
     * (float/uint) component falls back to int64. */
    {
        int32_t i;
        decode_state.use_i32 = 1;
        for (i = 0; i < ctx->num_channels; ++i)
            if (ctx->channels[i].pixel_type != EXR_PIXEL_HALF)
                decode_state.use_i32 = 0;
    }
#if defined(EXR_X86)
    decode_state.use_avx2 = (exr_cpu_caps() & EXR_SIMD_AVX2) != 0;
#endif
    rc = jph_alloc_component_planes(decode_state.a, jp, decode_state.use_i32,
                                    &decode_state.planes);
    if (rc != EXR_SUCCESS) return rc;
    if (collect) {
        /* GPU seam: walk the packets recording a code-block plan. */
        collect->jp = jp;
        collect->planes = decode_state.planes;
        collect->num_planes = decode_state.num_planes;
        collect->tile_base = jp->tile_data;
        rc = jph_parse_tile_packets(ctx, jp, &codeblocks, jph_collect_codeblock,
                                    collect);
        if (rc != EXR_SUCCESS) goto done;
        if (!collect->gpu_fn) goto done; /* pure-collect: hand plan to caller */

        /* Whole-image GPU decode: decode all blocks on the device, scatter the
         * tiles into the component planes, then inverse-transform + store. */
        {
            exr_jph_cb_plan plan;
            size_t *offs = NULL, total = 0, i;
            int32_t *coeffs = NULL;
            int all_elig = 1;
            for (i = 0; i < collect->count; ++i)
                if (!collect->records[i].i32_eligible) { all_elig = 0; break; }
            if (!all_elig) { rc = EXR_ERROR_UNSUPPORTED; goto done; }

            offs = (size_t *)exr_malloc(decode_state.a,
                                        (collect->count ? collect->count : 1u) *
                                            sizeof(size_t));
            if (!offs) { rc = EXR_ERROR_OUT_OF_MEMORY; goto done; }
            for (i = 0; i < collect->count; ++i) {
                uint32_t st = (collect->records[i].width + 7u) & ~7u;
                offs[i] = total;
                total += (size_t)st * collect->records[i].height;
            }
            coeffs = (int32_t *)exr_calloc(decode_state.a, total ? total : 1u,
                                           sizeof(int32_t));
            if (!coeffs) { exr_free(decode_state.a, offs); rc = EXR_ERROR_OUT_OF_MEMORY; goto done; }

            memset(&plan, 0, sizeof(plan));
            plan.records = collect->records;
            plan.num_records = collect->count;
            plan.data = (uint8_t *)(uintptr_t)jp->tile_data;
            plan.data_size = jp->tile_data_size;
            plan.num_components = (int)jp->csiz;
            rc = collect->gpu_fn(collect->gpu_user, &plan, offs, total, coeffs);
            if (rc != EXR_SUCCESS) { exr_free(decode_state.a, offs); exr_free(decode_state.a, coeffs); goto done; }

            /* scatter each tile into its component plane (int32 path) */
            for (i = 0; i < collect->count; ++i) {
                const exr_jph_cb_record *rcd = &collect->records[i];
                uint32_t st = (rcd->width + 7u) & ~7u, yy, xx;
                JphPlaneD *pl = &decode_state.planes[rcd->comp];
                const int32_t *tile = coeffs + offs[i];
                if (!pl->d32) { rc = EXR_ERROR_UNSUPPORTED; break; }
                for (yy = 0; yy < rcd->height; ++yy) {
                    int32_t *drow = pl->d32 +
                        (size_t)(rcd->dst_row + yy) * pl->w + rcd->dst_col;
                    const int32_t *srow = tile + (size_t)yy * st;
                    for (xx = 0; xx < rcd->width; ++xx) drow[xx] = srow[xx];
                }
            }
            exr_free(decode_state.a, offs);
            exr_free(decode_state.a, coeffs);
            if (rc != EXR_SUCCESS) goto done;
        }
        rc = jph_postprocess_component_planes(decode_state.a, jp,
                                              decode_state.planes,
                                              &decode_state.ws);
        if (rc != EXR_SUCCESS) goto done;
        rc = jph_store_component_planes_to_block(ctx, jp, map,
                                                 decode_state.planes, dst,
                                                 dst_size);
        goto done;
    }
    rc = jph_parse_tile_packets(ctx, jp, &codeblocks, jph_decode_codeblock,
                                &decode_state);
    if (rc != EXR_SUCCESS) goto done;
    if (decode_state.codeblocks != codeblocks) {
        rc = EXR_ERROR_CORRUPT;
        goto done;
    }
    if (codeblocks == 0 || codeblocks == decode_state.codeblocks) {
        rc = jph_postprocess_component_planes(decode_state.a, jp,
                                              decode_state.planes,
                                              &decode_state.ws);
        if (rc != EXR_SUCCESS) goto done;
        rc = jph_store_component_planes_to_block(ctx, jp, map,
                                                 decode_state.planes, dst,
                                                 dst_size);
    } else {
        rc = EXR_ERROR_CORRUPT;
    }

done:
    jph_decode_workspace_free(decode_state.a, &decode_state.ws);
    jph_free_component_planes(decode_state.a, decode_state.planes,
                              decode_state.num_planes);
    return rc;
}

static exr_result jph_validate_profile(const exr_codec_ctx *ctx,
                                       const uint16_t *map,
                                       const uint8_t *src, size_t src_size,
                                       uint8_t *dst, size_t dst_size,
                                       JphCollectState *collect) {
    const exr_allocator *a = ctx->alloc ? ctx->alloc : exr_default_allocator();
    JphReader r;
    JphProfile jp;
    uint16_t marker = 0u;
    exr_result rc = EXR_SUCCESS;
    int done = 0;
    uint16_t c;

    memset(&jp, 0, sizeof(jp));
    r.p = src;
    r.end = src + src_size;

    rc = jph_next_marker(&r, &marker);
    if (rc != EXR_SUCCESS) goto done;
    if (marker != JPH_MARK_SOC) { rc = EXR_ERROR_CORRUPT; goto done; }
    rc = jph_next_marker(&r, &marker);
    if (rc != EXR_SUCCESS) goto done;
    if (marker != JPH_MARK_SIZ) { rc = EXR_ERROR_CORRUPT; goto done; }
    rc = jph_parse_siz(a, &r, &jp);
    if (rc != EXR_SUCCESS) goto done;

    while (!done) {
        rc = jph_next_marker(&r, &marker);
        if (rc != EXR_SUCCESS) goto done;
        switch (marker) {
        case JPH_MARK_CAP:
            rc = jph_parse_cap(&r, &jp);
            break;
        case JPH_MARK_PRF:
        case JPH_MARK_CPF:
        case JPH_MARK_TLM:
        case JPH_MARK_PLM:
        case JPH_MARK_PLT:
        case JPH_MARK_CRG:
        case JPH_MARK_COM:
            rc = jph_skip_segment(&r);
            break;
        case JPH_MARK_COD:
            rc = jph_parse_cod(&r, &jp);
            break;
        case JPH_MARK_QCD:
            rc = jph_parse_quant(&r, &jp, 0);
            if (rc == EXR_SUCCESS) jp.saw_qcd = 1;
            break;
        case JPH_MARK_QCC:
            if (!jp.saw_siz) rc = EXR_ERROR_CORRUPT;
            else rc = jph_parse_quant(&r, &jp, 1);
            break;
        case JPH_MARK_NLT:
            rc = jph_parse_nlt(&r, &jp);
            break;
        case JPH_MARK_SOT:
            jp.sot_start = r.p - 2;
            rc = jph_parse_sot(&r, &jp);
            if (rc != EXR_SUCCESS) break;
            while (1) {
                rc = jph_next_marker(&r, &marker);
                if (rc != EXR_SUCCESS) break;
                if (marker == JPH_MARK_SOD) {
                    jp.saw_sod = 1;
                    rc = jph_finish_tile_payload(&r, &jp);
                    if (rc != EXR_SUCCESS) break;
                    done = 1;
                    break;
                }
                if (marker == JPH_MARK_PLT || marker == JPH_MARK_COM) {
                    rc = jph_skip_segment(&r);
                    if (rc != EXR_SUCCESS) break;
                    continue;
                }
                /* Benign tile-part markers that don't affect decode: skip. */
                if (marker == JPH_MARK_DFS || marker == JPH_MARK_ADS ||
                    marker == JPH_MARK_ATK || marker == JPH_MARK_POC) {
                    rc = jph_skip_segment(&r);
                    if (rc != EXR_SUCCESS) break;
                    continue;
                }
                rc = EXR_ERROR_UNSUPPORTED;
                break;
            }
            break;
        case JPH_MARK_COC:
        case JPH_MARK_RGN:
        case JPH_MARK_POC:
        case JPH_MARK_PPM:
        case JPH_MARK_PPT:
        case JPH_MARK_DFS:
        case JPH_MARK_ADS:
        case JPH_MARK_ATK:
            rc = jph_skip_segment(&r);
            break;
        default:
            rc = EXR_ERROR_UNSUPPORTED;
            break;
        }
        if (rc != EXR_SUCCESS) goto done;
    }

    if (!jp.saw_cod || !jp.saw_qcd || !jp.saw_sot || !jp.saw_sod) {
        rc = EXR_ERROR_CORRUPT;
        goto done;
    }
    if (jp.qcd_count != (uint8_t)(1u + 3u * jp.num_decomps)) {
        rc = EXR_ERROR_CORRUPT;
        goto done;
    }
    /* Any per-component QCC must describe the same subband structure. */
    if (jp.qcc_count) {
        uint16_t qc;
        for (qc = 0; qc < jp.csiz; ++qc) {
            if (jp.qcc_count[qc] != 0u &&
                jp.qcc_count[qc] != (uint8_t)(1u + 3u * jp.num_decomps)) {
                rc = EXR_ERROR_CORRUPT;
                goto done;
            }
        }
    }
    if (jp.csiz != (uint16_t)ctx->num_channels) { rc = EXR_ERROR_CORRUPT; goto done; }
    if ((jp.rsiz & 0x4000u) == 0) { rc = EXR_ERROR_UNSUPPORTED; goto done; }
    if (jp.xosiz != 0u || jp.yosiz != 0u || jp.xtosiz != 0u || jp.ytosiz != 0u)
    { rc = EXR_ERROR_UNSUPPORTED; goto done; }
    /* Tile codestreams: xsiz/ysiz are the tile dimensions, not the full
     * image. ctx->width and ctx->num_lines describe this chunk's region. */
    if (jp.xsiz > (uint32_t)ctx->width || jp.ysiz > (uint32_t)ctx->num_lines) {
        rc = EXR_ERROR_CORRUPT; goto done;
    }
    if (jp.xtsiz != jp.xsiz || jp.ytsiz != jp.ysiz) {
        rc = EXR_ERROR_UNSUPPORTED; goto done;
    }
    for (c = 0; c < jp.csiz; ++c) {
        rc = jph_validate_siz_component(ctx, &jp, map, c);
        if (rc != EXR_SUCCESS) goto done;
    }
    rc = jph_decode_tile_payload(ctx, &jp, map, dst, dst_size, collect);

done:
    exr_free(a, jp.ssiz);
    exr_free(a, jp.xrsiz);
    exr_free(a, jp.yrsiz);
    exr_free(a, jp.nlt_type);
    exr_free(a, jp.qcc_exp);
    exr_free(a, jp.qcc_count);
    exr_free(a, jp.qcc_guard_bits);
    return rc;
}

static exr_result jph_parse_ht_header(const exr_codec_ctx *ctx,
                                      const uint8_t *src, size_t src_size,
                                      uint16_t **out_map,
                                      size_t *out_codestream_off) {
    const exr_allocator *a = ctx->alloc ? ctx->alloc : exr_default_allocator();
    size_t payload_size, prefix_size = 6, header_size, need_map_bytes;
    uint16_t nch, i;
    uint8_t *seen = NULL;
    uint16_t *map = NULL;

    *out_map = NULL;
    *out_codestream_off = 0;
    if (ctx->num_channels <= 0 || ctx->num_channels > EXR_MAX_CHANNELS)
        return EXR_ERROR_CORRUPT;
    if (src_size < 8) return EXR_ERROR_CORRUPT;
    if (jph_be16(src) != 0x4854u) return EXR_ERROR_CORRUPT;
    payload_size = (size_t)jph_be32(src + 2);
    if (exr_add_ovf(prefix_size, payload_size, &header_size))
        return EXR_ERROR_CORRUPT;
    if (header_size > src_size) return EXR_ERROR_CORRUPT;
    if (payload_size < 2) return EXR_ERROR_CORRUPT;
    nch = jph_be16(src + 6);
    if (nch != (uint16_t)ctx->num_channels) return EXR_ERROR_CORRUPT;
    if (exr_mul_ovf((size_t)nch, 2u, &need_map_bytes))
        return EXR_ERROR_CORRUPT;
    if (payload_size < 2u + need_map_bytes) return EXR_ERROR_CORRUPT;

    map = (uint16_t *)exr_malloc(a, (size_t)nch * sizeof(*map));
    seen = (uint8_t *)exr_calloc(a, (size_t)nch, 1);
    if (!map || !seen) {
        exr_free(a, map);
        exr_free(a, seen);
        return EXR_ERROR_OUT_OF_MEMORY;
    }
    for (i = 0; i < nch; ++i) {
        uint16_t file_i = jph_be16(src + 8u + (size_t)i * 2u);
        if (file_i >= nch || seen[file_i]) {
            exr_free(a, map);
            exr_free(a, seen);
            return EXR_ERROR_CORRUPT;
        }
        map[i] = file_i;
        seen[file_i] = 1;
    }
    exr_free(a, seen);
    *out_map = map;
    *out_codestream_off = header_size;
    return EXR_SUCCESS;
}

exr_result exr_jph_decompress(const exr_codec_ctx *ctx, const uint8_t *src,
                              size_t src_size, uint8_t *dst, size_t dst_size) {
    uint16_t *map = NULL;
    size_t codestream_off = 0;
    exr_result rc;

    if (!ctx || !src) return EXR_ERROR_INVALID_ARGUMENT;
    rc = jph_parse_ht_header(ctx, src, src_size, &map, &codestream_off);
    if (rc != EXR_SUCCESS) return rc;
    if (codestream_off >= src_size) {
        exr_free(ctx->alloc ? ctx->alloc : exr_default_allocator(), map);
        return EXR_ERROR_CORRUPT;
    }
    rc = jph_validate_profile(ctx, map, src + codestream_off,
                              src_size - codestream_off, dst, dst_size, NULL);
    exr_free(ctx->alloc ? ctx->alloc : exr_default_allocator(), map);
    return rc;
}

/* ---- GPU seam: public-internal entry points ----------------------------- */
exr_result exr_jph_collect_codeblocks(const exr_codec_ctx *ctx,
                                      const uint8_t *src, size_t src_size,
                                      exr_jph_cb_plan *out) {
    const exr_allocator *a;
    uint16_t *map = NULL;
    size_t codestream_off = 0;
    JphCollectState cs;
    exr_result rc;

    if (!ctx || !src || !out) return EXR_ERROR_INVALID_ARGUMENT;
    a = ctx->alloc ? ctx->alloc : exr_default_allocator();
    memset(out, 0, sizeof(*out));
    memset(&cs, 0, sizeof(cs));
    cs.a = a;

    rc = jph_parse_ht_header(ctx, src, src_size, &map, &codestream_off);
    if (rc != EXR_SUCCESS) return rc;
    if (codestream_off >= src_size) {
        exr_free(a, map);
        return EXR_ERROR_CORRUPT;
    }
    /* jph_validate_profile -> jph_decode_tile_payload(collect) records the plan
     * (and leaves cs.tile_base pointing into the validated codestream). */
    rc = jph_validate_profile(ctx, map, src + codestream_off,
                              src_size - codestream_off, NULL, 0, &cs);
    exr_free(a, map);
    if (rc != EXR_SUCCESS) {
        exr_free(a, cs.records);
        return rc;
    }
    /* Copy the tile codestream so record data_offsets stay valid after the
     * caller frees src. tile_base points within [src, src+src_size). */
    if (cs.count > 0 && cs.tile_base) {
        size_t base_off = (size_t)(cs.tile_base - src);
        size_t copy_size = src_size - base_off;
        out->data = (uint8_t *)exr_malloc(a, copy_size ? copy_size : 1u);
        if (!out->data) {
            exr_free(a, cs.records);
            return EXR_ERROR_OUT_OF_MEMORY;
        }
        memcpy(out->data, cs.tile_base, copy_size);
        out->data_size = copy_size;
    }
    out->records = cs.records;
    out->num_records = cs.count;
    out->num_components = ctx->num_channels;
    return EXR_SUCCESS;
}

void exr_jph_cb_plan_free(const exr_allocator *a, exr_jph_cb_plan *plan) {
    if (!plan) return;
    if (!a) a = exr_default_allocator();
    exr_free(a, plan->records);
    exr_free(a, plan->data);
    memset(plan, 0, sizeof(*plan));
}

exr_result exr_jph_decompress_gpu(const exr_codec_ctx *ctx, const uint8_t *src,
                                  size_t src_size, uint8_t *dst, size_t dst_size,
                                  exr_jph_gpu_block_decode_fn fn, void *user) {
    const exr_allocator *a;
    uint16_t *map = NULL;
    size_t codestream_off = 0;
    JphCollectState cs;
    exr_result rc;
    if (!ctx || !src || !dst || !fn) return EXR_ERROR_INVALID_ARGUMENT;
    a = ctx->alloc ? ctx->alloc : exr_default_allocator();
    memset(&cs, 0, sizeof(cs));
    cs.a = a;
    cs.gpu_fn = fn;
    cs.gpu_user = user;
    rc = jph_parse_ht_header(ctx, src, src_size, &map, &codestream_off);
    if (rc != EXR_SUCCESS) return rc;
    if (codestream_off >= src_size) { exr_free(a, map); return EXR_ERROR_CORRUPT; }
    rc = jph_validate_profile(ctx, map, src + codestream_off,
                              src_size - codestream_off, dst, dst_size, &cs);
    exr_free(a, map);
    exr_free(a, cs.records);
    return rc;
}

exr_result exr_jph_ht_tables(const uint16_t **vlc0, const uint16_t **vlc1,
                             const uint16_t **uvlc0, const uint16_t **uvlc1) {
    JphHtTables *t = NULL;
    exr_result rc = jph_ht_ensure_tables(&t);
    if (rc != EXR_SUCCESS) return rc;
    if (vlc0) *vlc0 = t->vlc_tbl0;
    if (vlc1) *vlc1 = t->vlc_tbl1;
    if (uvlc0) *uvlc0 = t->uvlc_tbl0;
    if (uvlc1) *uvlc1 = t->uvlc_tbl1;
    return EXR_SUCCESS;
}

exr_result exr_jph_decode_one_block_i32(const exr_jph_cb_record *rec,
                                        const uint8_t *data, int32_t *out,
                                        uint32_t out_stride) {
    JphCodeblockSeg seg;
    JphHtTables *htab = NULL;
    exr_result rc;
    if (!rec || !data || !out) return EXR_ERROR_INVALID_ARGUMENT;
    rc = jph_ht_ensure_tables(&htab);
    if (rc != EXR_SUCCESS) return rc;
    memset(&seg, 0, sizeof(seg));
    seg.comp = rec->comp;
    seg.res = rec->res;
    seg.band = rec->band;
    seg.x0 = rec->x0;
    seg.y0 = rec->y0;
    seg.missing_msbs = rec->missing_msbs;
    seg.active_passes = rec->active_passes;
    seg.length0 = rec->length0;
    seg.length1 = rec->length1;
    seg.width = rec->width;
    seg.height = rec->height;
    seg.data = data + rec->data_offset;
    seg.data_size = rec->data_size;
    /* Scalar reference (use_avx2=0): bit-identical to the SIMD path and to the
     * GPU port, which mirrors the scalar reader/coder. */
    return jph_decode_block_i32(&seg, htab, out, out_stride, rec->kmax,
                                exr_default_allocator(), NULL, 0);
}

/* ----------------------------------------------------------------------------
 * Forward transforms (encode path).
 * ------------------------------------------------------------------------- */

static exr_result jph_forward_53_i32(const int32_t *src, size_t n,
                                     int32_t *low, size_t low_count,
                                     int32_t *high, size_t high_count) {
    size_t nl = (n + 1u) / 2u, nh = n / 2u, i;
    if (!src || !low || !high) return EXR_ERROR_INVALID_ARGUMENT;
    if (low_count != nl || high_count != nh) return EXR_ERROR_INVALID_ARGUMENT;
    if (n == 0) return EXR_SUCCESS;
    /* High-pass (predict): odd samples */
    for (i = 0; i < nh; ++i) {
        int64_t e0 = (int64_t)src[2u * i];
        int64_t e1 = (i + 1u < nl) ? (int64_t)src[2u * (i + 1u)] : e0;
        high[i] = (int32_t)((int64_t)src[2u * i + 1u] -
                            jph_floor_div_pow2(e0 + e1, 1));
    }
    /* Low-pass (update): even samples */
    for (i = 0; i < nl; ++i) {
        int64_t dl = (nh > 0) ? ((i > 0) ? (int64_t)high[i - 1u] : (int64_t)high[0]) : 0;
        int64_t dr = (nh > 0) ? ((i < nh) ? (int64_t)high[i] : (int64_t)high[nh - 1u]) : 0;
        low[i] = (int32_t)((int64_t)src[2u * i] +
                           jph_floor_div_pow2(dl + dr + 2, 2));
    }
    return EXR_SUCCESS;
}

/* Forward declarations for static functions defined after the 2D wrapper. */
static exr_result jph_forward_53_vert_i32(const int32_t *data, size_t width,
                                           size_t rw, size_t lh, size_t hh,
                                           int32_t *temp);
static exr_result jph_forward_53_1d_i32(const int32_t *src, size_t n,
                                        int32_t *low, size_t lc, int32_t *high,
                                        size_t hc, int use_simd, int64_t *ev,
                                        int64_t *od);

exr_result exr_jph_forward_53_2d_i32(const exr_allocator *a,
                                      int32_t *data, size_t width,
                                      size_t height, unsigned levels) {
    unsigned level;
    int use_simd = 0;
    if (!a) a = exr_default_allocator();
    if (!data && width && height) return EXR_ERROR_INVALID_ARGUMENT;
    if (levels > 32) return EXR_ERROR_INVALID_ARGUMENT;
    if (width == 0 || height == 0 || levels == 0) return EXR_SUCCESS;
#if defined(EXR_X86)
    use_simd = (exr_cpu_caps() & EXR_SIMD_AVX2) != 0;
#endif

    for (level = 1; level <= levels; ++level) {
        size_t rw = jph_ceil_div_pow2_size(width, level - 1u);
        size_t rh = jph_ceil_div_pow2_size(height, level - 1u);
        size_t lw = (rw + 1u) / 2u, hw = rw / 2u;
        size_t lh = (rh + 1u) / 2u, hh = rh / 2u;
        size_t temp_count, temp_bytes, scratch_len, scratch_bytes;
        int32_t *temp = NULL, *col_low = NULL, *col_high = NULL;
        int64_t *ev = NULL, *od = NULL;
        size_t y;
        exr_result rc = EXR_SUCCESS;

        if (rw == 0 || rh == 0) return EXR_ERROR_CORRUPT;
        if (exr_mul_ovf(rw, rh, &temp_count) ||
            exr_mul_ovf(temp_count, sizeof(int32_t), &temp_bytes))
            return EXR_ERROR_CORRUPT;
        scratch_len = rw > rh ? rw : rh;
        if (exr_mul_ovf(scratch_len, sizeof(int32_t), &scratch_bytes))
            return EXR_ERROR_CORRUPT;

        temp = (int32_t *)exr_malloc(a, temp_bytes);
        col_low = (int32_t *)exr_malloc(a, scratch_bytes);
        col_high = (int32_t *)exr_malloc(a, scratch_bytes);
        if (use_simd) {
            if (exr_mul_ovf(scratch_len, sizeof(int64_t), &scratch_bytes)) {
                exr_free(a, temp); exr_free(a, col_low); exr_free(a, col_high);
                return EXR_ERROR_CORRUPT;
            }
            ev = (int64_t *)exr_malloc(a, scratch_bytes);
            od = (int64_t *)exr_malloc(a, scratch_bytes);
        }
        if (!temp || !col_low || !col_high ||
            (use_simd && (!ev || !od))) {
            exr_free(a, temp); exr_free(a, col_low);
            exr_free(a, col_high); exr_free(a, ev); exr_free(a, od);
            return EXR_ERROR_OUT_OF_MEMORY;
        }

        /* Vertical (column) analysis, row-wise across all columns -- no gather/
         * scatter. data's interleaved rows -> temp's lh low-rows / hh high-rows. */
#if defined(EXR_X86)
        if (use_simd)
            rc = jph_forward_53_vert_i32_avx2(data, width, rw, lh, hh, temp);
        else
#endif
            rc = jph_forward_53_vert_i32(data, width, rw, lh, hh, temp);
        if (rc != EXR_SUCCESS) goto done_fwd32;
        /* Horizontal (row) analysis: each temp row is contiguous. */
        for (y = 0; y < rh; ++y) {
            rc = jph_forward_53_1d_i32(temp + y * rw, rw, col_low, lw, col_high,
                                       hw, use_simd, ev, od);
            if (rc != EXR_SUCCESS) goto done_fwd32;
            for (size_t x = 0; x < lw; ++x) data[y * width + x] = col_low[x];
            for (size_t x = 0; x < hw; ++x) data[y * width + lw + x] = col_high[x];
        }

done_fwd32:
        exr_free(a, temp); exr_free(a, col_low);
        exr_free(a, col_high); exr_free(a, ev); exr_free(a, od);
        if (rc != EXR_SUCCESS) return rc;
    }
    return EXR_SUCCESS;
}

/* Forward RCT: R/G/B → Y/Cb/Cr (inverse of existing inverse_rct). */
exr_result exr_jph_forward_rct_i32(int32_t *c0, int32_t *c1, int32_t *c2,
                                   size_t count) {
    size_t i = 0;
    if ((!c0 || !c1 || !c2) && count) return EXR_ERROR_INVALID_ARGUMENT;
#if defined(EXR_X86)
    if (exr_cpu_caps() & EXR_SIMD_AVX2)
        i = jph_forward_rct_i32_avx2(c0, c1, c2, count);
    else if (exr_cpu_caps() & EXR_SIMD_SSE2)
        i = jph_forward_rct_i32_sse2(c0, c1, c2, count);
#endif
    for (; i < count; ++i) {
        int64_t r = c0[i], g = c1[i], b = c2[i];
        int64_t y  = jph_floor_div_pow2(r + b + 2 * g, 2);
        int64_t db = b - g;
        int64_t dr = r - g;
        if (y < INT32_MIN || y > INT32_MAX ||
            db < INT32_MIN || db > INT32_MAX ||
            dr < INT32_MIN || dr > INT32_MAX)
            return EXR_ERROR_CORRUPT;
        c0[i] = (int32_t)y;
        c1[i] = (int32_t)db;
        c2[i] = (int32_t)dr;
    }
    return EXR_SUCCESS;
}

static exr_result jph_forward_rct_i64(int64_t *c0, int64_t *c1, int64_t *c2,
                                      size_t count) {
    size_t i;
    if ((!c0 || !c1 || !c2) && count) return EXR_ERROR_INVALID_ARGUMENT;
    for (i = 0; i < count; ++i) {
        int64_t r = c0[i], g = c1[i], b = c2[i];
        c0[i] = jph_floor_div_pow2(r + b + 2 * g, 2);
        c1[i] = b - g;
        c2[i] = r - g;
    }
    return EXR_SUCCESS;
}

/* Forward NLT type 3: binary complement of negative values.
 * Inverse of exr_jph_apply_nlt_type3_i32. */
exr_result exr_jph_forward_nlt_type3_i32(int32_t *data, size_t count,
                                         uint32_t bit_depth) {
    size_t i;
    int64_t bias;
    if (!data && count) return EXR_ERROR_INVALID_ARGUMENT;
    if (bit_depth == 0 || bit_depth > 32u) return EXR_ERROR_INVALID_ARGUMENT;
    bias = ((int64_t)1 << (bit_depth - 1u)) + 1;
    for (i = 0; i < count; ++i) {
        int64_t v = data[i];
        if (v == INT32_MIN) return EXR_ERROR_CORRUPT;
        if (v < 0) {
            int64_t neg = -v - bias;
            if (neg < INT32_MIN || neg > INT32_MAX) return EXR_ERROR_CORRUPT;
            data[i] = (int32_t)neg;
        }
    }
    return EXR_SUCCESS;
}

/* Row-wise (column-parallel) forward reversible 5/3 vertical pass, int32. Source
 * of truth for jph_forward_53_vert_i32_avx2 (must stay bit-identical). Reads the
 * rh = lh+hh interleaved rows of `data` (stride `width`) -- even rows 2i, odd
 * rows 2i+1 -- and writes the subband layout into `temp` (stride rw): lh low-rows
 * [0..lh) then hh high-rows [lh..lh+hh). Per column this is exactly
 * jph_forward_53_i32; doing it row-wise avoids the strided column gather/scatter
 * (columns are the natural SIMD axis). `data` and `temp` are distinct buffers. */
static exr_result jph_forward_53_vert_i32(const int32_t *data, size_t width,
                                           size_t rw, size_t lh, size_t hh,
                                           int32_t *temp) {
    size_t i, c;
    /* Phase 1: high rows -> temp[(lh+i)*rw] */
    for (i = 0u; i < hh; ++i) {
        const int32_t *e0 = data + (2u * i) * width;
        const int32_t *e1 = data + (2u * (i + 1u < lh ? i + 1u : i)) * width;
        const int32_t *od = data + (2u * i + 1u) * width;
        int32_t *hd = temp + (lh + i) * rw;
        for (c = 0u; c < rw; ++c)
            hd[c] = (int32_t)((int64_t)od[c] -
                              jph_floor_div_pow2((int64_t)e0[c] + (int64_t)e1[c], 1));
    }
    /* Phase 2: low rows -> temp[i*rw] */
    for (i = 0u; i < lh; ++i) {
        const int32_t *e0 = data + (2u * i) * width;
        int32_t *ld = temp + i * rw;
        if (hh == 0u) {
            for (c = 0u; c < rw; ++c) ld[c] = e0[c];
        } else {
            const int32_t *hl = temp + (lh + (i > 0u ? i - 1u : 0u)) * rw;
            const int32_t *hr = temp + (lh + (i < hh ? i : hh - 1u)) * rw;
            for (c = 0u; c < rw; ++c)
                ld[c] = (int32_t)((int64_t)e0[c] +
                                  jph_floor_div_pow2((int64_t)hl[c] + (int64_t)hr[c] + 2, 2));
        }
    }
    return EXR_SUCCESS;
}

/* Forward 1D 5/3 int32: AVX2 when available (bit-identical), else scalar. */
static exr_result jph_forward_53_1d_i32(const int32_t *src, size_t n,
                                        int32_t *low, size_t lc, int32_t *high,
                                        size_t hc, int use_simd, int64_t *ev,
                                        int64_t *od) {
#if defined(EXR_X86)
    if (use_simd)
        return jph_forward_53_i32_avx2(src, n, low, lc, high, hc, ev, od);
#else
    (void)use_simd; (void)ev; (void)od;
#endif
    return jph_forward_53_i32(src, n, low, lc, high, hc);
}

/* ---- int64 forward variants (for >=32-bit-precision components) ----------
 * Mirror the exported i32 forward transforms but keep full int64 width so that
 * a 32-bit component's reversible-5/3 coefficients (which can exceed int32) are
 * encoded losslessly. They produce identical results to the i32 versions for
 * values that fit int32, so half/all-float output is unchanged. */
static exr_result jph_forward_53_i64(const int64_t *src, size_t n,
                                     int64_t *low, size_t low_count,
                                     int64_t *high, size_t high_count) {
    size_t nl = (n + 1u) / 2u, nh = n / 2u, i;
    if (!src || !low || !high) return EXR_ERROR_INVALID_ARGUMENT;
    if (low_count != nl || high_count != nh) return EXR_ERROR_INVALID_ARGUMENT;
    if (n == 0) return EXR_SUCCESS;
    for (i = 0; i < nh; ++i) {
        int64_t e0 = src[2u * i];
        int64_t e1 = (i + 1u < nl) ? src[2u * (i + 1u)] : e0;
        high[i] = src[2u * i + 1u] - jph_floor_div_pow2(e0 + e1, 1);
    }
    for (i = 0; i < nl; ++i) {
        int64_t dl = (nh > 0) ? ((i > 0) ? high[i - 1u] : high[0]) : 0;
        int64_t dr = (nh > 0) ? ((i < nh) ? high[i] : high[nh - 1u]) : 0;
        low[i] = src[2u * i] + jph_floor_div_pow2(dl + dr + 2, 2);
    }
    return EXR_SUCCESS;
}

/* Forward 1D 5/3: AVX2 when available (bit-identical), else scalar. col_low
 * doubles as the source for the column pass, so the SIMD kernel reads `src`
 * which may alias the `low` output — pass a distinct source for that case. */
static exr_result jph_forward_53_1d_i64(const int64_t *src, size_t n,
                                        int64_t *low, size_t lc, int64_t *high,
                                        size_t hc, int use_simd, int64_t *ev,
                                        int64_t *od) {
#if defined(EXR_X86)
    if (use_simd)
        return jph_forward_53_i64_avx2(src, n, low, lc, high, hc, ev, od);
#elif defined(EXR_NEON)
    if (use_simd)
        return jph_forward_53_i64_neon(src, n, low, lc, high, hc, ev, od);
#else
    (void)use_simd; (void)ev; (void)od;
#endif
    return jph_forward_53_i64(src, n, low, lc, high, hc);
}

/* Row-wise (column-parallel) forward reversible 5/3 vertical pass, int64. Source
 * of truth for jph_forward_53_vert_i64_avx2 (must stay bit-identical). Reads the
 * rh = lh+hh interleaved rows of `data` (stride `width`) -- even rows 2i, odd
 * rows 2i+1 -- and writes the subband layout into `temp` (stride rw): lh low-rows
 * [0..lh) then hh high-rows [lh..lh+hh). Per column this is exactly
 * jph_forward_53_i64; doing it row-wise avoids the strided column gather/scatter
 * (columns are the natural SIMD axis). `data` and `temp` are distinct buffers. */
EXR_NO_SANITIZE_SIO
exr_result jph_forward_53_vert_i64(const int64_t *data, size_t width,
                                   size_t rw, size_t lh, size_t hh,
                                   int64_t *temp) {
    size_t i, c;
    /* Phase 1: high rows -> temp[(lh+i)*rw]. high[i] = odd[i] - floor((e0+e1)/2)
     * where e1 (even[i+1]) clamps to even[i] at the bottom edge. */
    for (i = 0u; i < hh; ++i) {
        const int64_t *e0 = data + (2u * i) * width;
        const int64_t *e1 = data + (2u * (i + 1u < lh ? i + 1u : i)) * width;
        const int64_t *od = data + (2u * i + 1u) * width;
        int64_t *hd = temp + (lh + i) * rw;
        for (c = 0u; c < rw; ++c)
            hd[c] = od[c] - jph_floor_div_pow2(e0[c] + e1[c], 1);
    }
    /* Phase 2: low rows -> temp[i*rw]. low[i] = even[i] + floor((dl+dr+2)/4),
     * dl=high[i-1] (clamp 0), dr=high[i] (clamp hh-1). hh==0 -> low[i]=even[i]. */
    for (i = 0u; i < lh; ++i) {
        const int64_t *e0 = data + (2u * i) * width;
        int64_t *ld = temp + i * rw;
        if (hh == 0u) {
            for (c = 0u; c < rw; ++c) ld[c] = e0[c];
        } else {
            const int64_t *hl = temp + (lh + (i > 0u ? i - 1u : 0u)) * rw;
            const int64_t *hr = temp + (lh + (i < hh ? i : hh - 1u)) * rw;
            for (c = 0u; c < rw; ++c)
                ld[c] = e0[c] + jph_floor_div_pow2(hl[c] + hr[c] + 2, 2);
        }
    }
    return EXR_SUCCESS;
}

static exr_result jph_forward_53_2d_i64(const exr_allocator *a, int64_t *data,
                                        size_t width, size_t height,
                                        unsigned levels) {
    unsigned level;
    int use_simd = 0;
    if (!a) a = exr_default_allocator();
    if (!data && width && height) return EXR_ERROR_INVALID_ARGUMENT;
    if (levels > 32) return EXR_ERROR_INVALID_ARGUMENT;
    if (width == 0 || height == 0 || levels == 0) return EXR_SUCCESS;
#if defined(EXR_X86)
    use_simd = (exr_cpu_caps() & EXR_SIMD_AVX2) != 0;
#elif defined(EXR_NEON)
    use_simd = 1;
#endif

    for (level = 1; level <= levels; ++level) {
        size_t rw = jph_ceil_div_pow2_size(width, level - 1u);
        size_t rh = jph_ceil_div_pow2_size(height, level - 1u);
        size_t lw = (rw + 1u) / 2u, hw = rw / 2u;
        size_t lh = (rh + 1u) / 2u, hh = rh / 2u;
        size_t temp_count, temp_bytes, scratch_len, scratch_bytes;
        int64_t *temp = NULL, *col_low = NULL, *col_high = NULL;
        int64_t *ev = NULL, *od = NULL;
        size_t y, x;
        exr_result rc = EXR_SUCCESS;

        if (rw == 0 || rh == 0) return EXR_ERROR_CORRUPT;
        if (exr_mul_ovf(rw, rh, &temp_count) ||
            exr_mul_ovf(temp_count, sizeof(int64_t), &temp_bytes))
            return EXR_ERROR_CORRUPT;
        scratch_len = rw > rh ? rw : rh;
        if (exr_mul_ovf(scratch_len, sizeof(int64_t), &scratch_bytes))
            return EXR_ERROR_CORRUPT;

        temp = (int64_t *)exr_malloc(a, temp_bytes);
        col_low = (int64_t *)exr_malloc(a, scratch_bytes);
        col_high = (int64_t *)exr_malloc(a, scratch_bytes);
        if (use_simd) {
            ev = (int64_t *)exr_malloc(a, scratch_bytes);
            od = (int64_t *)exr_malloc(a, scratch_bytes);
        }
        if (!temp || !col_low || !col_high ||
            (use_simd && (!ev || !od))) {
            exr_free(a, temp); exr_free(a, col_low);
            exr_free(a, col_high); exr_free(a, ev); exr_free(a, od);
            return EXR_ERROR_OUT_OF_MEMORY;
        }

        /* Vertical (column) analysis, row-wise across all columns -- no gather/
         * scatter. data's interleaved rows -> temp's lh low-rows / hh high-rows. */
#if defined(EXR_X86)
        if (use_simd)
            rc = jph_forward_53_vert_i64_avx2(data, width, rw, lh, hh, temp);
        else
#elif defined(EXR_NEON)
        if (use_simd)
            rc = jph_forward_53_vert_i64_neon(data, width, rw, lh, hh, temp);
        else
#endif
            rc = jph_forward_53_vert_i64(data, width, rw, lh, hh, temp);
        if (rc != EXR_SUCCESS) goto done_fwd64;
        /* Horizontal (row) analysis: each temp row is contiguous. */
        for (y = 0; y < rh; ++y) {
            rc = jph_forward_53_1d_i64(temp + y * rw, rw, col_low, lw, col_high,
                                       hw, use_simd, ev, od);
            if (rc != EXR_SUCCESS) goto done_fwd64;
            for (x = 0; x < lw; ++x) data[y * width + x] = col_low[x];
            for (x = 0; x < hw; ++x) data[y * width + lw + x] = col_high[x];
        }

done_fwd64:
        exr_free(a, temp); exr_free(a, col_low);
        exr_free(a, col_high); exr_free(a, ev); exr_free(a, od);
        if (rc != EXR_SUCCESS) return rc;
    }
    return EXR_SUCCESS;
}

/* Forward NLT type-3 is the same involution as the inverse; share the
 * dispatched implementation. */
static exr_result jph_forward_nlt_type3_i64(int64_t *data, size_t count,
                                            uint32_t bit_depth) {
    return jph_apply_nlt_type3_i64(data, count, bit_depth);
}

/* Deinterleave a packed EXR block into component planes. */
static exr_result jph_deinterleave_block(const exr_codec_ctx *ctx,
                                         const uint8_t *block, size_t n,
                                         JphPlane64 *planes) {
    uint32_t c, x, y, pixel_type, ps;
    size_t off = 0;
    if (!ctx || !block || !planes) return EXR_ERROR_INVALID_ARGUMENT;
    for (y = 0; y < (uint32_t)ctx->num_lines; ++y) {
        int32_t yy = ctx->y + (int32_t)y;
        for (c = 0; c < (uint32_t)ctx->num_channels; ++c) {
            const exr_channel *ch = &ctx->channels[c];
            int32_t xs = ch->x_sampling > 0 ? ch->x_sampling : 1;
            int32_t ys = ch->y_sampling > 0 ? ch->y_sampling : 1;
            int32_t nx, row_i;
            if ((yy % ys) != 0) continue;
            nx = exr_num_samples(ctx->x, ctx->x + ctx->width - 1, xs);
            if (nx < 0) nx = 0;
            row_i = exr_num_samples(ctx->y, yy, ys) - 1;
            if (row_i < 0) return EXR_ERROR_CORRUPT;
            for (x = 0; x < (uint32_t)nx; ++x) {
                size_t idx = (size_t)row_i * planes[c].w + x;
                pixel_type = ch->pixel_type;
                ps = (pixel_type == EXR_PIXEL_HALF) ? 2 : 4;
                if (off + ps > n) return EXR_ERROR_CORRUPT;
                if (pixel_type == EXR_PIXEL_HALF) {
                    int16_t v = (int16_t)(block[off] | ((uint16_t)block[off+1] << 8));
                    planes[c].data[idx] = (int64_t)v;
                    off += 2;
                } else {
                    uint32_t v = (uint32_t)block[off] |
                                 ((uint32_t)block[off+1] << 8) |
                                 ((uint32_t)block[off+2] << 16) |
                                 ((uint32_t)block[off+3] << 24);
                    if (pixel_type == EXR_PIXEL_UINT)
                        planes[c].data[idx] = (int64_t)(uint32_t)v;
                    else
                        planes[c].data[idx] = (int64_t)(int32_t)v;
                    off += 4;
                }
            }
        }
    }
    if (off != n) return EXR_ERROR_CORRUPT;
    return EXR_SUCCESS;
}

/* ----------------------------------------------------------------------------
 * VLC/UVLC encode tables (from OpenJPH block encoder, BSD-2-Clause).
 * ------------------------------------------------------------------------- */

/* UVLC encode table: 75 entries as defined in the HTJ2K spec. */
typedef struct {
    uint8_t pre, pre_len, suf, suf_len, ext, ext_len;
} JphUvlcEnc;

static JphUvlcEnc g_uvlc_enc_tbl[75];
static int g_uvlc_enc_init = 0;

static void JPH_MAYBE_UNUSED jph_ensure_uvlc_enc_tables(void) {
    if (g_uvlc_enc_init) return;
    g_uvlc_enc_init = 1;
    memset(g_uvlc_enc_tbl, 0, sizeof(g_uvlc_enc_tbl));
    g_uvlc_enc_tbl[0] = (JphUvlcEnc){0,0,0,0,0,0};
    g_uvlc_enc_tbl[1] = (JphUvlcEnc){1,1,0,0,0,0};
    g_uvlc_enc_tbl[2] = (JphUvlcEnc){2,2,0,0,0,0};
    g_uvlc_enc_tbl[3] = (JphUvlcEnc){4,3,0,1,0,0};
    g_uvlc_enc_tbl[4] = (JphUvlcEnc){4,3,1,1,0,0};
    for (int i = 5; i < 33; ++i) {
        g_uvlc_enc_tbl[i] = (JphUvlcEnc){0,3,(uint8_t)(i-5),5,0,0};
    }
    for (int i = 33; i < 75; ++i) {
        g_uvlc_enc_tbl[i] = (JphUvlcEnc){0,3,(uint8_t)(28+(i-33)%4),5,(uint8_t)((i-33)/4),4};
    }
}

/* VLC encode tables: 2048 entries (c_q<<8 | rho<<4 | emb) → value (cwd<<8 | cwd_len<<4 | e_k) */
static uint16_t g_vlc_enc_tbl0[2048];
static uint16_t g_vlc_enc_tbl1[2048];
static int g_vlc_enc_init = 0;

static int jph_popcount(int v) {
    v = v - ((v >> 1) & 0x55555555);
    v = (v & 0x33333333) + ((v >> 2) & 0x33333333);
    v = (v + (v >> 4)) & 0x0F0F0F0F;
    v = v + (v >> 8);
    v = v + (v >> 16);
    return v & 0x3F;
}

static void JPH_MAYBE_UNUSED jph_ensure_vlc_enc_tables(void) {
    if (g_vlc_enc_init) return;
    g_vlc_enc_init = 1;
    memset(g_vlc_enc_tbl0, 0, sizeof(g_vlc_enc_tbl0));
    memset(g_vlc_enc_tbl1, 0, sizeof(g_vlc_enc_tbl1));

    /* Build table 0 (initial row) */
    {
        size_t tbl_size = sizeof(jph_vlc_src_table0) / sizeof(jph_vlc_src_table0[0]);
        for (int i = 0; i < 2048; ++i) {
            int c_q = i >> 8, rho = (i >> 4) & 0xF, emb = i & 0xF;
            if (((emb & rho) != emb) || (rho == 0 && c_q == 0))
                continue;
            const uint8_t *best = NULL;
            if (emb) { /* u_off = 1 */
                int best_ones = -1;
                for (size_t j = 0; j < tbl_size; ++j) {
                    const uint8_t *e = jph_vlc_src_table0[j];
                    if (e[0] == (uint8_t)c_q && e[1] == (uint8_t)rho && e[2] == 1) {
                        if ((emb & e[3]) == e[4]) {
                            int ones = jph_popcount(e[3]);
                            if (ones >= best_ones) { best = e; best_ones = ones; }
                        }
                    }
                }
            } else { /* u_off = 0 */
                for (size_t j = 0; j < tbl_size; ++j) {
                    const uint8_t *e = jph_vlc_src_table0[j];
                    if (e[0] == (uint8_t)c_q && e[1] == (uint8_t)rho && e[2] == 0) {
                        best = e; break;
                    }
                }
            }
            if (best) {
                int cwd = best[5], cwd_len = best[6], e_k = best[3];
                g_vlc_enc_tbl0[i] = (uint16_t)((cwd << 8) | (cwd_len << 4) | e_k);
            }
        }
    }

    /* Build table 1 (non-initial row) */
    {
        size_t tbl_size = sizeof(jph_vlc_src_table1) / sizeof(jph_vlc_src_table1[0]);
        for (int i = 0; i < 2048; ++i) {
            int c_q = i >> 8, rho = (i >> 4) & 0xF, emb = i & 0xF;
            if (((emb & rho) != emb) || (rho == 0 && c_q == 0))
                continue;
            const uint8_t *best = NULL;
            if (emb) { /* u_off = 1 */
                int best_ones = -1;
                for (size_t j = 0; j < tbl_size; ++j) {
                    const uint8_t *e = jph_vlc_src_table1[j];
                    if (e[0] == (uint8_t)c_q && e[1] == (uint8_t)rho && e[2] == 1) {
                        if ((emb & e[3]) == e[4]) {
                            int ones = jph_popcount(e[3]);
                            if (ones >= best_ones) { best = e; best_ones = ones; }
                        }
                    }
                }
            } else {
                for (size_t j = 0; j < tbl_size; ++j) {
                    const uint8_t *e = jph_vlc_src_table1[j];
                    if (e[0] == (uint8_t)c_q && e[1] == (uint8_t)rho && e[2] == 0) {
                        best = e; break;
                    }
                }
            }
            if (best) {
                int cwd = best[5], cwd_len = best[6], e_k = best[3];
                g_vlc_enc_tbl1[i] = (uint16_t)((cwd << 8) | (cwd_len << 4) | e_k);
            }
        }
    }
}

void exr_jph_warmup_encode_tables(void) {
    jph_ensure_uvlc_enc_tables();
    jph_ensure_vlc_enc_tables();
}

/* ----------------------------------------------------------------------------
 * HT codeblock encoder (port of OpenJPH ojph_encode_codeblock32).
 * ------------------------------------------------------------------------- */

/* Forward-growing MagSgn bit writer with a 64-bit accumulator and bulk byte
 * flush. Bits accumulate LSB-first in `acc`; bytes are sliced off `max_bits`
 * wide (8 normally, 7 right after a 0xFF byte — the JPEG2000 bit-stuffing that
 * keeps the segment free of false marker codes). The byte-exact equivalent of
 * the previous byte-at-a-time writer; the only change is that runs of
 * non-0xFF bytes are emitted in one store via a SWAR 0xFF scan. */
typedef struct {
    uint8_t *buf; uint32_t pos; uint32_t cap;
    uint64_t acc;     /* pending bits, LSB-first (bits >= nbits are 0) */
    int nbits;        /* valid bits in acc (< max_bits once flushed) */
    int max_bits;     /* width of the next byte: 8, or 7 right after a 0xFF */
} JphMsEnc;

static inline void jph_ms_init(JphMsEnc *m, uint8_t *buf, uint32_t cap) {
    m->buf = buf; m->pos = 0; m->cap = cap;
    m->acc = 0; m->nbits = 0; m->max_bits = 8;
}

/* Emit every complete byte currently in acc. When max_bits==8 (no pending
 * stuffing) the low bytes of acc ARE the output bytes, so a run with no 0xFF is
 * stored in bulk; the first 0xFF byte ends the run and forces the next byte to
 * 7-bit (which can never be 0xFF, re-aligning the accumulator to 8-bit). */
static inline exr_result jph_ms_flush(JphMsEnc *m) {
    while (m->nbits >= m->max_bits) {
        if (m->max_bits == 8) {
            int avail = m->nbits >> 3;            /* complete bytes available */
            uint64_t t = m->acc ^ ~UINT64_C(0);   /* 0xFF bytes -> 0x00 */
            uint64_t hasff;
            int run;
            if (avail < 8)
                t |= ~((UINT64_C(1) << (avail * 8)) - 1u); /* ignore high bytes */
            hasff = (t - 0x0101010101010101ULL) & ~t & 0x8080808080808080ULL;
            if (hasff == 0) {
                run = avail;                       /* no 0xFF in the run */
            } else {
                run = 1;                           /* find first 0xFF byte */
                while (((m->acc >> ((run - 1) * 8)) & 0xFFu) != 0xFFu) ++run;
            }
            if (m->pos + (uint32_t)run > m->cap) return EXR_ERROR_CORRUPT;
            if (m->pos + 8u <= m->cap) {
                uint64_t le = m->acc;              /* little-endian == stream order */
                memcpy(m->buf + m->pos, &le, 8);   /* slack bytes get overwritten */
            } else {
                int i;
                for (i = 0; i < run; ++i)
                    m->buf[m->pos + i] = (uint8_t)(m->acc >> (i * 8));
            }
            m->pos += (uint32_t)run;
            m->acc = (run >= 8) ? 0 : (m->acc >> (run * 8)); /* >>64 is UB */
            m->nbits -= run * 8;
            if (hasff != 0) m->max_bits = 7;       /* last emitted byte was 0xFF */
        } else {
            uint8_t byte;                          /* max_bits == 7 */
            if (m->pos >= m->cap) return EXR_ERROR_CORRUPT;
            byte = (uint8_t)(m->acc & 0x7Fu);
            m->buf[m->pos++] = byte;
            m->acc >>= 7; m->nbits -= 7;
            m->max_bits = 8;                       /* a 7-bit byte is never 0xFF */
        }
    }
    return EXR_SUCCESS;
}

static inline exr_result jph_ms_encode64(JphMsEnc *m, uint64_t cwd, int cwd_len) {
    while (cwd_len > 0) {
        int take = 64 - m->nbits;
        exr_result rc;
        if (take > cwd_len) take = cwd_len;
        m->acc |= (cwd & (take < 64 ? ((UINT64_C(1) << take) - 1u) : ~UINT64_C(0)))
                  << m->nbits;
        m->nbits += take;
        cwd >>= take; cwd_len -= take;
        rc = jph_ms_flush(m);
        if (rc != EXR_SUCCESS) return rc;
    }
    return EXR_SUCCESS;
}

static inline exr_result jph_ms_terminate(JphMsEnc *m) {
    if (m->nbits) {
        int t = m->max_bits - m->nbits;
        uint32_t tmp = (uint32_t)(m->acc & 0xFFu);
        tmp |= (0xFFu & ((1U << t) - 1u)) << m->nbits;
        if (tmp != 0xFFu) {
            if (m->pos >= m->cap) return EXR_ERROR_CORRUPT;
            m->buf[m->pos++] = (uint8_t)tmp;
        }
    } else if (m->max_bits == 7 && m->pos > 0u) {
        m->pos--;
    }
    return EXR_SUCCESS;
}

/* MEL encoder structure */
typedef struct {
    uint8_t *buf; uint32_t pos; uint32_t cap;
    int remaining_bits; int tmp; int run; int k; int threshold;
} JphMelEnc;

static inline void jph_mel_enc_init(JphMelEnc *m, uint8_t *buf, uint32_t cap) {
    m->buf = buf; m->pos = 0; m->cap = cap;
    m->remaining_bits = 8; m->tmp = 0; m->run = 0; m->k = 0;
    m->threshold = 1;
}

static exr_result jph_mel_enc_emit_bit(JphMelEnc *m, int v) {
    m->tmp = (m->tmp << 1) + v;
    m->remaining_bits--;
    if (m->remaining_bits == 0) {
        if (m->pos >= m->cap) return EXR_ERROR_CORRUPT;
        m->buf[m->pos++] = (uint8_t)m->tmp;
        m->remaining_bits = (m->tmp == 0xFF) ? 7 : 8;
        m->tmp = 0;
    }
    return EXR_SUCCESS;
}

static inline exr_result jph_mel_enc_encode(JphMelEnc *m, int bit) {
    static const int mel_exp[13] = {0,0,0,1,1,1,2,2,2,3,3,4,5};
    exr_result rc;
    if (bit == 0) {
        ++m->run;
        if (m->run >= m->threshold) {
            rc = jph_mel_enc_emit_bit(m, 1);
            if (rc != EXR_SUCCESS) return rc;
            m->run = 0;
            m->k = (m->k < 12) ? m->k + 1 : 12;
            m->threshold = 1 << mel_exp[m->k];
        }
    } else {
        rc = jph_mel_enc_emit_bit(m, 0);
        if (rc != EXR_SUCCESS) return rc;
        int t = mel_exp[m->k];
        while (t > 0) {
            t--;
            rc = jph_mel_enc_emit_bit(m, (m->run >> t) & 1);
            if (rc != EXR_SUCCESS) return rc;
        }
        m->run = 0;
        m->k = (m->k > 0) ? m->k - 1 : 0;
        m->threshold = 1 << mel_exp[m->k];
    }
    return EXR_SUCCESS;
}

/* VLC encoder (backward-growing) */
typedef struct {
    uint8_t *buf; uint32_t pos; uint32_t cap;
    int used_bits; uint64_t tmp; int last_greater_than_8F;
} JphVlcEnc;

static inline void jph_vlc_enc_init(JphVlcEnc *v, uint8_t *buf, uint32_t cap) {
    v->buf = buf + cap - 1; v->pos = 1; v->cap = cap;
    v->buf[0] = 0xFF;
    v->used_bits = 4; v->tmp = 0xF;
    v->last_greater_than_8F = 1;
}

static inline exr_result jph_vlc_enc_encode(JphVlcEnc *v, int cwd, int cwd_len) {
    if (cwd_len <= 0) return EXR_SUCCESS;
    v->tmp |= (uint64_t)(uint32_t)cwd << v->used_bits;
    v->used_bits += cwd_len;
    while (v->used_bits >= 8) {
        uint8_t b;
        if (v->pos >= v->cap) return EXR_ERROR_CORRUPT;
        if (v->last_greater_than_8F) {
            b = (uint8_t)(v->tmp & 0x7Fu);
            if (b != 0x7Fu) {
                b = (uint8_t)(v->tmp & 0xFFu);
                *(v->buf - v->pos) = b;
                v->pos++;
                v->last_greater_than_8F = (b > 0x8Fu) ? 1 : 0;
                v->tmp >>= 8;
                v->used_bits -= 8;
            } else {
                *(v->buf - v->pos) = b;
                v->pos++;
                v->last_greater_than_8F = 0;
                v->tmp >>= 7;
                v->used_bits -= 7;
            }
        } else {
            b = (uint8_t)(v->tmp & 0xFFu);
            *(v->buf - v->pos) = b;
            v->pos++;
            v->last_greater_than_8F = (b > 0x8Fu) ? 1 : 0;
            v->tmp >>= 8;
            v->used_bits -= 8;
        }
    }
    return EXR_SUCCESS;
}

static inline exr_result jph_mel_vlc_terminate(JphMelEnc *m, JphVlcEnc *v) {
    exr_result rc;
    if (m->run > 0) { rc = jph_mel_enc_emit_bit(m, 1); if (rc != EXR_SUCCESS) return rc; }
    m->tmp = m->tmp << m->remaining_bits;
    int mel_mask = (0xFF << m->remaining_bits) & 0xFF;
    int vlc_mask = 0xFF >> (8 - v->used_bits);
    if ((mel_mask | vlc_mask) == 0) return EXR_SUCCESS;
    if (m->pos >= m->cap) return EXR_ERROR_CORRUPT;
    int vtmp = (int)(v->tmp & 0xFFu);
    int fuse = m->tmp | vtmp;
    if (((((fuse ^ m->tmp) & mel_mask) | ((fuse ^ vtmp) & vlc_mask)) == 0)
        && (fuse != 0xFF) && v->pos > 1) {
        m->buf[m->pos++] = (uint8_t)fuse;
    } else {
        if (v->pos >= v->cap) return EXR_ERROR_CORRUPT;
        m->buf[m->pos++] = (uint8_t)m->tmp;
        *(v->buf - v->pos) = (uint8_t)vtmp;
        v->pos++;
    }
    return EXR_SUCCESS;
}

static inline int jph_clz64(uint64_t v) {
    if (!v) return 64;
#if defined(__GNUC__) || defined(__clang__)
    return __builtin_clzll(v); /* single lzcnt/bsr on x86; v != 0 guaranteed */
#else
    {
        int n = 0;
        if (!(v & UINT64_C(0xFFFFFFFF00000000))) { n += 32; v <<= 32; }
        if (!(v & UINT64_C(0xFFFF000000000000))) { n += 16; v <<= 16; }
        if (!(v & UINT64_C(0xFF00000000000000))) { n += 8;  v <<= 8;  }
        if (!(v & UINT64_C(0xF000000000000000))) { n += 4;  v <<= 4;  }
        if (!(v & UINT64_C(0xC000000000000000))) { n += 2;  v <<= 2;  }
        if (!(v & UINT64_C(0x8000000000000000))) { n += 1;  }
        return n;
    }
#endif
}

static inline int jph_clz32(uint32_t v) {
    if (!v) return 32;
#if defined(__GNUC__) || defined(__clang__)
    return __builtin_clz(v);
#else
    {
        int n = 0;
        if (!(v & UINT32_C(0xFFFF0000))) { n += 16; v <<= 16; }
        if (!(v & UINT32_C(0xFF000000))) { n += 8;  v <<= 8;  }
        if (!(v & UINT32_C(0xF0000000))) { n += 4;  v <<= 4;  }
        if (!(v & UINT32_C(0xC0000000))) { n += 2;  v <<= 2;  }
        if (!(v & UINT32_C(0x80000000))) { n += 1;  }
        return n;
    }
#endif
}

/* abs() of a coefficient that may exceed int32 (32-bit-precision components). */
static inline uint64_t jph_abs_i64_to_u64(int64_t v) {
    uint64_t mask = (uint64_t)(v >> 63); /* 0 for v>=0, ~0 for v<0 */
    return ((uint64_t)v ^ mask) - mask;
}

static inline uint64_t jph_encode_block_sample(const int64_t *plane_data,
                                        uint32_t plane_stride,
                                        uint32_t cb_x0, uint32_t cb_y0,
                                        uint32_t x, uint32_t y,
                                        uint32_t shift) {
    int64_t sv = plane_data[(cb_y0 + y) * plane_stride + (cb_x0 + x)];
    uint64_t sign = (uint64_t)(sv >> 63) & UINT64_C(0x8000000000000000);
    uint64_t mag = jph_abs_i64_to_u64(sv);
    return sign | (mag << shift);
}

static inline void jph_encode_block_prepare_sample(const int64_t *plane_data,
                                                    uint32_t plane_stride,
                                                    uint32_t cb_x0,
                                                    uint32_t cb_y0, uint32_t x,
                                                    uint32_t y, uint32_t shift,
                                                    uint32_t p, int *rho,
                                                    int *e_qmax, int *e_q,
                                                    uint64_t *s, int bit,
                                                    uint64_t *max_val) {
    uint64_t t, val;
    t = jph_encode_block_sample(plane_data, plane_stride, cb_x0, cb_y0,
                                x, y, shift);
    if (max_val) {
        uint64_t absv = (t << 1) >> (shift + 1u);
        if (absv > *max_val) *max_val = absv;
    }
    val = t + t;
    val >>= p;
    val &= ~UINT64_C(1);
    if (val) {
        int eq;
        *rho |= bit;
        eq = 64 - jph_clz64(--val);
        *e_q = eq;
        if (eq > *e_qmax) *e_qmax = eq;
        *s = --val + (t >> 63);
    } else {
        *e_q = 0;
        *s = 0;
    }
}

static inline void jph_encode_block_prepare_sample32(const int64_t *plane_data,
                                                      uint32_t plane_stride,
                                                      uint32_t cb_x0,
                                                      uint32_t cb_y0,
                                                      uint32_t x, uint32_t y,
                                                      uint32_t shift, uint32_t p,
                                                      int *rho, int *e_qmax,
                                                      int *e_q, uint64_t *s,
                                                      int bit,
                                                      uint64_t *max_val) {
    int64_t sv = plane_data[(cb_y0 + y) * plane_stride + (cb_x0 + x)];
    uint32_t mag = (uint32_t)jph_abs_i64_to_u64(sv);
    if (max_val) {
        uint64_t absv = (uint64_t)mag;
        if (absv > *max_val) *max_val = absv;
    }
    uint32_t t = (sv < 0 ? UINT32_C(0x80000000) : 0u) | (mag << shift);
    uint32_t val = t + t;
    val >>= p;
    val &= ~UINT32_C(1);
    if (val) {
        int eq;
        *rho |= bit;
        eq = 32 - jph_clz32(--val);
        *e_q = eq;
        if (eq > *e_qmax) *e_qmax = eq;
        *s = (uint64_t)(--val + (t >> 31u));
    } else {
        *e_q = 0;
        *s = 0;
    }
}

/* int32-native variant of jph_encode_block_prepare_sample32: reads the sample
 * straight from an int32 plane (all-HALF path) instead of an int64 plane. The
 * arithmetic is identical to the int64 version's low-32-bit result, so the
 * encoded output is byte-for-byte the same. */
static inline void jph_encode_block_prepare_sample_from32(
    const int32_t *plane_data, uint32_t plane_stride, uint32_t cb_x0,
    uint32_t cb_y0, uint32_t x, uint32_t y, uint32_t shift, uint32_t p,
    int *rho, int *e_qmax, int *e_q, uint64_t *s, int bit, uint64_t *max_val) {
    int32_t sv = plane_data[(cb_y0 + y) * plane_stride + (cb_x0 + x)];
    uint32_t mag = (uint32_t)jph_abs_i64_to_u64((int64_t)sv);
    if (max_val) {
        uint64_t absv = (uint64_t)mag;
        if (absv > *max_val) *max_val = absv;
    }
    uint32_t t = (sv < 0 ? UINT32_C(0x80000000) : 0u) | (mag << shift);
    uint32_t val = t + t;
    val >>= p;
    val &= ~UINT32_C(1);
    if (val) {
        int eq;
        *rho |= bit;
        eq = 32 - jph_clz32(--val);
        *e_q = eq;
        if (eq > *e_qmax) *e_qmax = eq;
        *s = (uint64_t)(--val + (t >> 31u));
    } else {
        *e_q = 0;
        *s = 0;
    }
}

#if defined(EXR_X86)
/* Dispatch the SSE2 quad prepare to the int32-native or int64 loader. */
static inline void jph_encode_prepare_quad_dispatch(
    const int64_t *plane_data, const int32_t *plane_data32, uint32_t stride,
    uint32_t x0, uint32_t y0, uint32_t x, uint32_t y, uint32_t shift,
    uint32_t p, int *rho, int *e_qmax, int e_q[4], uint64_t s[4],
    uint64_t *max_val) {
    if (plane_data32)
        jph_encode_prepare_quad_from32_sse2(plane_data32, stride, x0, y0, x, y,
                                            shift, p, rho, e_qmax, e_q, s,
                                            max_val);
    else
        jph_encode_prepare_quad_i32_sse2(plane_data, stride, x0, y0, x, y,
                                         shift, p, rho, e_qmax, e_q, s,
                                         max_val);
}
#endif

static inline void jph_encode_block_prepare_sample_fast(
    const int64_t *plane_data, const int32_t *plane_data32,
    uint32_t plane_stride, uint32_t cb_x0,
    uint32_t cb_y0, uint32_t x, uint32_t y, uint32_t shift, uint32_t p,
    int use_i32, int *rho, int *e_qmax, int *e_q, uint64_t *s, int bit,
    uint64_t *max_val) {
    if (plane_data32) {
        jph_encode_block_prepare_sample_from32(plane_data32, plane_stride,
                                               cb_x0, cb_y0, x, y, shift, p,
                                               rho, e_qmax, e_q, s, bit,
                                               max_val);
    } else if (use_i32) {
        jph_encode_block_prepare_sample32(plane_data, plane_stride, cb_x0,
                                          cb_y0, x, y, shift, p, rho, e_qmax,
                                          e_q, s, bit, max_val);
    } else {
        jph_encode_block_prepare_sample(plane_data, plane_stride, cb_x0, cb_y0,
                                        x, y, shift, p, rho, e_qmax, e_q, s,
                                        bit, max_val);
    }
}

static inline exr_result jph_encode_uvlc_pair(JphVlcEnc *vlc, int u_q0,
                                       int u_q1, int initial_line) {
    exr_result rc;
    if (u_q0 < 0 || u_q1 < 0 || u_q0 >= 75 || u_q1 >= 75)
        return EXR_ERROR_CORRUPT;
    if (initial_line && u_q0 > 0 && u_q1 > 0) {
        if (u_q0 > 2 && u_q1 > 2) {
            if (u_q0 - 2 >= 75 || u_q1 - 2 >= 75) return EXR_ERROR_CORRUPT;
            rc = jph_vlc_enc_encode(vlc, g_uvlc_enc_tbl[u_q0 - 2].pre,
                                    g_uvlc_enc_tbl[u_q0 - 2].pre_len);
            if (rc != EXR_SUCCESS) return rc;
            rc = jph_vlc_enc_encode(vlc, g_uvlc_enc_tbl[u_q1 - 2].pre,
                                    g_uvlc_enc_tbl[u_q1 - 2].pre_len);
            if (rc != EXR_SUCCESS) return rc;
            rc = jph_vlc_enc_encode(vlc, g_uvlc_enc_tbl[u_q0 - 2].suf,
                                    g_uvlc_enc_tbl[u_q0 - 2].suf_len);
            if (rc != EXR_SUCCESS) return rc;
            rc = jph_vlc_enc_encode(vlc, g_uvlc_enc_tbl[u_q1 - 2].suf,
                                    g_uvlc_enc_tbl[u_q1 - 2].suf_len);
            if (rc != EXR_SUCCESS) return rc;
            rc = jph_vlc_enc_encode(vlc, g_uvlc_enc_tbl[u_q0 - 2].ext,
                                    g_uvlc_enc_tbl[u_q0 - 2].ext_len);
            if (rc != EXR_SUCCESS) return rc;
            return jph_vlc_enc_encode(vlc, g_uvlc_enc_tbl[u_q1 - 2].ext,
                                      g_uvlc_enc_tbl[u_q1 - 2].ext_len);
        }
        if (u_q0 > 2) {
            rc = jph_vlc_enc_encode(vlc, g_uvlc_enc_tbl[u_q0].pre,
                                    g_uvlc_enc_tbl[u_q0].pre_len);
            if (rc != EXR_SUCCESS) return rc;
            rc = jph_vlc_enc_encode(vlc, (uint32_t)(u_q1 - 1), 1);
            if (rc != EXR_SUCCESS) return rc;
            rc = jph_vlc_enc_encode(vlc, g_uvlc_enc_tbl[u_q0].suf,
                                    g_uvlc_enc_tbl[u_q0].suf_len);
            if (rc != EXR_SUCCESS) return rc;
            return jph_vlc_enc_encode(vlc, g_uvlc_enc_tbl[u_q0].ext,
                                      g_uvlc_enc_tbl[u_q0].ext_len);
        }
    }
    rc = jph_vlc_enc_encode(vlc, g_uvlc_enc_tbl[u_q0].pre,
                            g_uvlc_enc_tbl[u_q0].pre_len);
    if (rc != EXR_SUCCESS) return rc;
    rc = jph_vlc_enc_encode(vlc, g_uvlc_enc_tbl[u_q1].pre,
                            g_uvlc_enc_tbl[u_q1].pre_len);
    if (rc != EXR_SUCCESS) return rc;
    rc = jph_vlc_enc_encode(vlc, g_uvlc_enc_tbl[u_q0].suf,
                            g_uvlc_enc_tbl[u_q0].suf_len);
    if (rc != EXR_SUCCESS) return rc;
    rc = jph_vlc_enc_encode(vlc, g_uvlc_enc_tbl[u_q1].suf,
                            g_uvlc_enc_tbl[u_q1].suf_len);
    if (rc != EXR_SUCCESS) return rc;
    rc = jph_vlc_enc_encode(vlc, g_uvlc_enc_tbl[u_q0].ext,
                            g_uvlc_enc_tbl[u_q0].ext_len);
    if (rc != EXR_SUCCESS) return rc;
    return jph_vlc_enc_encode(vlc, g_uvlc_enc_tbl[u_q1].ext,
                              g_uvlc_enc_tbl[u_q1].ext_len);
}

static inline exr_result jph_encode_mag_bits_pair(JphMsEnc *ms, uint64_t s0,
                                                  uint64_t s1, int rho,
                                                  int bit0, int bit1, int Uq,
                                                  uint16_t tuple) {
    int m0 = (rho & bit0) ? Uq - ((tuple & bit0) ? 1 : 0) : 0;
    int m1 = (rho & bit1) ? Uq - ((tuple & bit1) ? 1 : 0) : 0;
    uint64_t cwd;
    int cwd_len;

    if (m0 < 0 || m1 < 0 || m0 >= 64 || m1 >= 64) return EXR_ERROR_CORRUPT;
    if ((m0 | m1) == 0) return EXR_SUCCESS;
    if (m0 == 0) return jph_ms_encode64(ms, s1 & jph_mask64((uint32_t)m1), m1);
    if (m1 == 0) return jph_ms_encode64(ms, s0 & jph_mask64((uint32_t)m0), m0);

    cwd_len = m0 + m1;
    if (cwd_len < 64) {
        cwd = s0 & jph_mask64((uint32_t)m0);
        cwd |= (s1 & jph_mask64((uint32_t)m1)) << (uint32_t)m0;
        return jph_ms_encode64(ms, cwd, cwd_len);
    }

    {
        exr_result rc =
            jph_ms_encode64(ms, s0 & jph_mask64((uint32_t)m0), m0);
        if (rc != EXR_SUCCESS) return rc;
        return jph_ms_encode64(ms, s1 & jph_mask64((uint32_t)m1), m1);
    }
}

static inline exr_result jph_encode_mag_bits_quad(JphMsEnc *ms,
                                                  const uint64_t *s, int rho,
                                                  int Uq, uint16_t tuple) {
    exr_result rc;
    rc = jph_encode_mag_bits_pair(ms, s[0], s[1], rho, 1, 2, Uq, tuple);
    if (rc != EXR_SUCCESS) return rc;
    return jph_encode_mag_bits_pair(ms, s[2], s[3], rho, 4, 8, Uq, tuple);
}

#if defined(EXR_X86)
/* Emit one quad's MagSgn bits from the Stage-C precomputed m_n / masked cwd_s
 * (sample-major [k*8+q]); the pair assembly + bit-append match the scalar
 * jph_encode_mag_bits_pair exactly but skip the now-vectorized m_n + masking. */
static inline exr_result jph_emit_mag_quad_pre(JphMsEnc *ms, const int32_t *m_n,
                                               const int32_t *cwd_s, int q) {
    /* The MagSgn writer is a continuous bitstream, so emitting the quad's four
     * samples [s0|s1|s2|s3] as one codeword produces the exact same bytes as the
     * per-pair emit but with one ms_encode64 call instead of two. cwd_s is
     * pre-masked to m_n bits, so zero-magnitude samples contribute nothing. */
    int m0 = m_n[q], m1 = m_n[8 + q], m2 = m_n[16 + q], m3 = m_n[24 + q];
    int L;
    if ((unsigned)m0 >= 64u || (unsigned)m1 >= 64u ||
        (unsigned)m2 >= 64u || (unsigned)m3 >= 64u)
        return EXR_ERROR_CORRUPT;       /* also catches negative (wraps high) */
    L = m0 + m1 + m2 + m3;
    if (L == 0) return EXR_SUCCESS;
    if (L < 64) {
        uint64_t cwd = (uint32_t)cwd_s[q];
        cwd |= (uint64_t)(uint32_t)cwd_s[8 + q] << m0;
        cwd |= (uint64_t)(uint32_t)cwd_s[16 + q] << (m0 + m1);
        cwd |= (uint64_t)(uint32_t)cwd_s[24 + q] << (m0 + m1 + m2);
        return jph_ms_encode64(ms, cwd, L);
    }
    /* Rare L>=64: emit per pair (continuous-bitstream equivalent). */
    {
        int pr;
        for (pr = 0; pr < 2; ++pr) {
            int k0 = pr * 2;
            int a0 = m_n[k0 * 8 + q], a1 = m_n[(k0 + 1) * 8 + q];
            uint64_t d0 = (uint32_t)cwd_s[k0 * 8 + q];
            uint64_t d1 = (uint32_t)cwd_s[(k0 + 1) * 8 + q];
            exr_result rc;
            if ((a0 | a1) == 0) continue;
            if (a0 == 0) rc = jph_ms_encode64(ms, d1, a1);
            else if (a1 == 0) rc = jph_ms_encode64(ms, d0, a0);
            else if (a0 + a1 < 64)
                rc = jph_ms_encode64(ms, d0 | (d1 << (uint32_t)a0), a0 + a1);
            else {
                rc = jph_ms_encode64(ms, d0, a0);
                if (rc != EXR_SUCCESS) return rc;
                rc = jph_ms_encode64(ms, d1, a1);
            }
            if (rc != EXR_SUCCESS) return rc;
        }
    }
    return EXR_SUCCESS;
}

/* Build a zero-padded 16x2 int32 tile for the 8-quad AVX2 prepare kernel: tile
 * columns [0,16) map to codeblock columns [xg, xg+16); out-of-range columns and
 * (for odd height) the absent bottom row are zeroed, matching the scalar
 * prepare's treatment of edge samples. tile[0..15]=row y, tile[16..31]=row y+1. */
static inline void jph_enc_build_tile_i32(int32_t tile[32],
                                          const int64_t *p64, const int32_t *p32,
                                          uint32_t stride, uint32_t x0,
                                          uint32_t y0, uint32_t xg, uint32_t y,
                                          uint32_t width, uint32_t height) {
    uint32_t cmax = width - xg;          /* real columns in this group (>=1) */
    int has_row1 = (y + 1u < height);
    uint32_t c;
    if (cmax > 16u) cmax = 16u;
    memset(tile, 0, 32 * sizeof(int32_t));
    if (p32) {
        const int32_t *r0 = p32 + (size_t)(y0 + y) * stride + (x0 + xg);
        memcpy(tile, r0, cmax * sizeof(int32_t));
        if (has_row1)
            memcpy(tile + 16,
                   p32 + (size_t)(y0 + y + 1u) * stride + (x0 + xg),
                   cmax * sizeof(int32_t));
    } else {
        const int64_t *r0 = p64 + (size_t)(y0 + y) * stride + (x0 + xg);
        const int64_t *r1 = has_row1
            ? p64 + (size_t)(y0 + y + 1u) * stride + (x0 + xg) : NULL;
        for (c = 0; c < cmax; ++c) {
            tile[c] = (int32_t)r0[c];
            if (has_row1) tile[16 + c] = (int32_t)r1[c];
        }
    }
}

/* AVX2 i32 codeblock encoder (kmax<=30). Functionally identical to the scalar
 * jph_encode_block below but processes the cleanup pass in 8-quad (16-column)
 * batches: one jph_enc_proc_pixel_8q_avx2 call prepares e_q/s/rho/e_qmax for 8
 * quads, then tinyexr's exact scalar bookkeeping + MEL/VLC/MagSgn emit run per
 * quad-pair (byte-identical output). The remaining per-quad bookkeeping is the
 * vectorization target of later stages; here it stays scalar. */
static exr_result jph_encode_block_i32_avx2(
    const int64_t *plane64, const int32_t *plane32, uint32_t plane_stride,
    uint32_t cb_x0, uint32_t cb_y0, uint32_t cb_w, uint32_t cb_h, uint32_t kmax,
    uint32_t *out_missing_msbs, uint32_t out_lengths[2], uint8_t *out_buf,
    size_t out_cap, size_t *out_size) {
    uint8_t ms_buf[65536u];
    uint8_t mel_vlc_buf[3072];
    uint8_t *mel_buf = mel_vlc_buf;
    uint8_t *vlc_buf = mel_vlc_buf + 192;
    uint32_t ms_cap = (uint32_t)sizeof(ms_buf), mel_cap = 192;
    uint32_t vlc_cap = 3072u - 192u;
    uint32_t shift, p, width = cb_w, height = cb_h;
    uint32_t n_vec = (width + 15u) / 16u;
    uint64_t max_val = 0;
    exr_result rc;
    int32_t geq[32], gsa[32], grho[8], gem[8], tile[32];
    int32_t cq8[8], eps8[8], uq8[8], Uq8[8];
    int32_t tuple8[8], m_n[32], cwd_s[32];
    /* Flat E/CX line-state indexed by quad position; the slack covers the
     * unaligned neighbour reads (max_e, proc_cq2) one+ vectors past the end.
     * A 128-wide codeblock yields n_vec<=8 (<=64 quad positions). */
    int32_t e_line[528] = {0};
    int32_t cx_line[528] = {0};
    int prev_cq, prev_e, prev_cx;
    uint32_t xv, xi, y;

    *out_missing_msbs = kmax - 1u;
    p = 31u - kmax;          /* use_i32 path (kmax<=30) guaranteed by caller */
    shift = p;

    JphMelEnc mel; jph_mel_enc_init(&mel, mel_buf, mel_cap);
    JphVlcEnc vlc; jph_vlc_enc_init(&vlc, vlc_buf, vlc_cap);
    JphMsEnc ms; jph_ms_init(&ms, ms_buf, ms_cap);

    /* Initial row pair (y=0): tbl0, kappa==1, the min(u_q)>2 MEL, uvlc initial.
     * The context kernel computes c_q/eps/u_q/U_q + line-state for all 8 quads;
     * the serial emit then drives MEL/VLC/MagSgn per quad-pair in scalar order. */
    prev_cq = 0; prev_e = 0; prev_cx = 0;
    for (xv = 0u; xv < n_vec; ++xv) {
        uint32_t xg = xv * 16u;
        jph_enc_build_tile_i32(tile, plane64, plane32, plane_stride, cb_x0,
                               cb_y0, xg, 0u, width, height);
        jph_enc_proc_pixel_8q_avx2(tile, shift, p, geq, gsa, grho, gem,
                                   &max_val);
        jph_enc_context_8q_avx2(1, geq, grho, gem, e_line, cx_line, xv,
                                &prev_cq, &prev_e, &prev_cx, cq8, eps8, uq8, Uq8);
        for (int q = 0; q < 8; ++q)
            tuple8[q] = g_vlc_enc_tbl0[(cq8[q] << 8) + (grho[q] << 4) + eps8[q]];
        jph_enc_ms_prep_8q_avx2(grho, Uq8, tuple8, gsa, m_n, cwd_s);
        for (xi = 0u; xi < 16u && xg + xi < width; xi += 4u) {
            uint32_t x = xg + xi;
            int q0 = (int)(xi >> 1), q1 = q0 + 1;
            int uq0 = uq8[q0], uq1 = 0;
            uint16_t tuple0 = (uint16_t)tuple8[q0];

            rc = jph_vlc_enc_encode(&vlc, tuple0 >> 8, (tuple0 >> 4) & 7);
            if (rc != EXR_SUCCESS) return rc;
            if (cq8[q0] == 0) {
                rc = jph_mel_enc_encode(&mel, grho[q0] != 0);
                if (rc != EXR_SUCCESS) return rc;
            }
            rc = jph_emit_mag_quad_pre(&ms, m_n, cwd_s, q0);
            if (rc != EXR_SUCCESS) return rc;

            if (x + 2u < width) {
                uint16_t tuple1 = (uint16_t)tuple8[q1];
                uq1 = uq8[q1];
                rc = jph_vlc_enc_encode(&vlc, tuple1 >> 8, (tuple1 >> 4) & 7);
                if (rc != EXR_SUCCESS) return rc;
                if (cq8[q1] == 0) {
                    rc = jph_mel_enc_encode(&mel, grho[q1] != 0);
                    if (rc != EXR_SUCCESS) return rc;
                }
                rc = jph_emit_mag_quad_pre(&ms, m_n, cwd_s, q1);
                if (rc != EXR_SUCCESS) return rc;
            }

            if (uq0 > 0 && uq1 > 0) {
                int min_uq = uq0 < uq1 ? uq0 : uq1;
                rc = jph_mel_enc_encode(&mel, min_uq > 2);
                if (rc != EXR_SUCCESS) return rc;
            }
            rc = jph_encode_uvlc_pair(&vlc, uq0, uq1, 1);
            if (rc != EXR_SUCCESS) return rc;
        }
    }
    /* Emulate the absent xv=n_vec vector's lane-0 line-state write (the carry-out
     * of the last quad): the last real quad's max_e / proc_cq2 reads position
     * n_vec*8, which the scalar encoder fills via its final lep[0]=e_q3 /
     * lcxp[0]=(rho&8)>>3 store after the last lep/lcxp++. */
    e_line[n_vec * 8] = prev_e;
    cx_line[n_vec * 8] = (prev_cx & 8) >> 3;

    for (y = 2u; y < height; y += 2u) {
        /* Reseed c_q for this row from the previous row's first-vector CX
         * line-state (matches the scalar row-start c_q0 = lcxp[0]+(lcxp[1]<<2));
         * the E/CX line-state itself carries (not reset) as the previous row. */
        prev_cq = cx_line[0] + (cx_line[1] << 2);
        prev_e = 0; prev_cx = 0;
        for (xv = 0u; xv < n_vec; ++xv) {
            uint32_t xg = xv * 16u;
            jph_enc_build_tile_i32(tile, plane64, plane32, plane_stride, cb_x0,
                                   cb_y0, xg, y, width, height);
            jph_enc_proc_pixel_8q_avx2(tile, shift, p, geq, gsa, grho, gem,
                                       &max_val);
            jph_enc_context_8q_avx2(0, geq, grho, gem, e_line, cx_line, xv,
                                    &prev_cq, &prev_e, &prev_cx, cq8, eps8, uq8,
                                    Uq8);
            for (int q = 0; q < 8; ++q)
                tuple8[q] = g_vlc_enc_tbl1[(cq8[q] << 8) + (grho[q] << 4) +
                                          eps8[q]];
            jph_enc_ms_prep_8q_avx2(grho, Uq8, tuple8, gsa, m_n, cwd_s);
            for (xi = 0u; xi < 16u && xg + xi < width; xi += 4u) {
                uint32_t x = xg + xi;
                int q0 = (int)(xi >> 1), q1 = q0 + 1;
                int uq0 = uq8[q0], uq1 = 0;
                uint16_t tuple0 = (uint16_t)tuple8[q0];

                rc = jph_vlc_enc_encode(&vlc, tuple0 >> 8, (tuple0 >> 4) & 7);
                if (rc != EXR_SUCCESS) return rc;
                if (cq8[q0] == 0) {
                    rc = jph_mel_enc_encode(&mel, grho[q0] != 0);
                    if (rc != EXR_SUCCESS) return rc;
                }
                rc = jph_emit_mag_quad_pre(&ms, m_n, cwd_s, q0);
                if (rc != EXR_SUCCESS) return rc;

                if (x + 2u < width) {
                    uint16_t tuple1 = (uint16_t)tuple8[q1];
                    uq1 = uq8[q1];
                    rc = jph_vlc_enc_encode(&vlc, tuple1 >> 8, (tuple1 >> 4) & 7);
                    if (rc != EXR_SUCCESS) return rc;
                    if (cq8[q1] == 0) {
                        rc = jph_mel_enc_encode(&mel, grho[q1] != 0);
                        if (rc != EXR_SUCCESS) return rc;
                    }
                    rc = jph_emit_mag_quad_pre(&ms, m_n, cwd_s, q1);
                    if (rc != EXR_SUCCESS) return rc;
                }

                rc = jph_encode_uvlc_pair(&vlc, uq0, uq1, 0);
                if (rc != EXR_SUCCESS) return rc;
            }
        }
        e_line[n_vec * 8] = prev_e;          /* carry-out (see initial row) */
        cx_line[n_vec * 8] = (prev_cx & 8) >> 3;
    }

    if (max_val == 0) {
        *out_missing_msbs = kmax;
        out_lengths[0] = 0u; out_lengths[1] = 0u; *out_size = 0u;
        return EXR_SUCCESS;
    }
    if (max_val >= (UINT64_C(1) << kmax)) return EXR_ERROR_CORRUPT;

    rc = jph_ms_terminate(&ms);
    if (rc != EXR_SUCCESS) return rc;
    rc = jph_mel_vlc_terminate(&mel, &vlc);
    if (rc != EXR_SUCCESS) return rc;

    {
        int lcup = (int)ms.pos + (int)mel.pos + (int)vlc.pos;
        int scup = (int)mel.pos + (int)vlc.pos;
        if (lcup < 2) { lcup = 2; scup = 2; }
        if (lcup > (int)out_cap) return EXR_ERROR_CORRUPT;
        if (ms.pos) memcpy(out_buf, ms_buf, ms.pos);
        if (mel.pos) memcpy(out_buf + ms.pos, mel_buf, mel.pos);
        if (vlc.pos)
            memcpy(out_buf + ms.pos + mel.pos, vlc_buf + vlc.cap - vlc.pos,
                   vlc.pos);
        out_buf[lcup - 2] = (uint8_t)((out_buf[lcup - 2] & 0xF0) | (scup & 0x0F));
        out_buf[lcup - 1] = (uint8_t)((scup >> 4) & 0xFF);
        out_lengths[0] = (uint32_t)lcup;
        out_lengths[1] = 0;
        *out_size = (size_t)lcup;
    }
    return EXR_SUCCESS;
}
#endif /* EXR_X86 */

/* Encode one HT codeblock. When plane32 is non-NULL, read samples from the
 * int32 plane (widening to int64 on the fly via a local per-codeblock buffer). */
static exr_result JPH_MAYBE_UNUSED jph_encode_block(const int64_t *plane64,
                                    const int32_t *plane32,
                                    uint32_t plane_stride,
                                    uint32_t cb_x0, uint32_t cb_y0,
                                    uint32_t cb_w, uint32_t cb_h,
                                    uint32_t kmax,
                                    uint32_t *out_missing_msbs,
                                    uint32_t out_lengths[2],
                                    uint8_t *out_buf,
                                    size_t out_cap,
                                    size_t *out_size) {
    /* Buffer allocation for the three segments */
    uint8_t ms_buf[65536u];
    uint8_t mel_vlc_buf[3072];
    uint8_t *mel_buf = mel_vlc_buf;
    uint8_t *vlc_buf = mel_vlc_buf + 192;
    uint32_t ms_cap = (uint32_t)sizeof(ms_buf), mel_cap = 192;
    uint32_t vlc_cap = 3072u - 192u;
    uint32_t shift, p;
    int use_i32;
    uint64_t max_val = 0;
    exr_result rc;

    if (cb_w == 0 || cb_h == 0) { *out_size = 0; return EXR_SUCCESS; }
    if ((!plane64 && !plane32) || !out_missing_msbs || !out_lengths ||
        !out_buf || !out_size || kmax == 0u || kmax > 36u)
        return EXR_ERROR_INVALID_ARGUMENT;

    /* int32 codeblock data is read natively (no int32->int64 widening) when
     * plane32 is provided; otherwise the int64 plane is the source. */
    const int64_t *plane_data = plane64;
    const int32_t *plane_data32 = plane32;

    /* Missing MSBs, shift and precision are directly computed from Kmax.
     * The max-value scan is folded into the main encode loop below. */
    *out_missing_msbs = kmax - 1u;
    use_i32 = kmax <= 30u;
    p = (use_i32 ? 31u : 63u) - kmax;
    shift = p;
#if defined(EXR_X86)
    /* AVX2 i32 fast path: 8-quad-batched cleanup pass, byte-identical output. */
    if (use_i32 && (exr_cpu_caps() & EXR_SIMD_AVX2))
        return jph_encode_block_i32_avx2(plane64, plane32, plane_stride, cb_x0,
                                         cb_y0, cb_w, cb_h, kmax,
                                         out_missing_msbs, out_lengths, out_buf,
                                         out_cap, out_size);
    int use_sse2 = use_i32 && (exr_cpu_caps() & EXR_SIMD_SSE2);
#else
    int use_sse2 = 0;
#endif
    (void)use_sse2;

    /* Initialize encoders */
    JphMelEnc mel; jph_mel_enc_init(&mel, mel_buf, mel_cap);
    JphVlcEnc vlc; jph_vlc_enc_init(&vlc, vlc_buf, vlc_cap);
    JphMsEnc ms; jph_ms_init(&ms, ms_buf, ms_cap);

    uint8_t e_val[513] = {0};
    uint8_t cx_val[513] = {0};
    uint8_t *lep = e_val; lep[0] = 0;
    uint8_t *lcxp = cx_val; lcxp[0] = 0;

    uint32_t width = cb_w, height = cb_h;
    int c_q0 = 0;

    {
        uint32_t x;
        int e_qmax[2] = {0, 0};
        int e_q[8] = {0, 0, 0, 0, 0, 0, 0, 0};
        int rho[2] = {0, 0};
        uint64_t s[8] = {0, 0, 0, 0, 0, 0, 0, 0};
        for (x = 0u; x < width; x += 4u) {
            int Uq0, Uq1 = 1, u_q0, u_q1 = 0, eps0 = 0, eps1 = 0;
            int c_q1;
            uint16_t tuple0, tuple1 = 0;
            rho[0] = rho[1] = 0;
            e_qmax[0] = e_qmax[1] = 0;

#if defined(EXR_X86)
            if (use_sse2 && height > 1u && x + 1u < width) {
                jph_encode_prepare_quad_dispatch(plane_data, plane_data32, plane_stride,
                                                  cb_x0, cb_y0, x, 0u,
                                                  shift, p, &rho[0],
                                                  &e_qmax[0], e_q, s,
                                                  &max_val);
            } else
#endif
            {
            jph_encode_block_prepare_sample_fast(plane_data, plane_data32, plane_stride,
                                                  cb_x0, cb_y0, x, 0u, shift,
                                                  p, use_i32, &rho[0],
                                                  &e_qmax[0], &e_q[0], &s[0],
                                                   1, &max_val);
            if (height > 1u)
                jph_encode_block_prepare_sample_fast(
                    plane_data, plane_data32, plane_stride, cb_x0, cb_y0, x, 1u, shift, p,
                    use_i32, &rho[0], &e_qmax[0], &e_q[1], &s[1], 2, &max_val);
            else { e_q[1] = 0; s[1] = 0; }
            if (x + 1u < width) {
                jph_encode_block_prepare_sample_fast(
                    plane_data, plane_data32, plane_stride, cb_x0, cb_y0, x + 1u, 0u, shift,
                    p, use_i32, &rho[0], &e_qmax[0], &e_q[2], &s[2], 4, &max_val);
                if (height > 1u)
                    jph_encode_block_prepare_sample_fast(
                        plane_data, plane_data32, plane_stride, cb_x0, cb_y0, x + 1u, 1u,
                        shift, p, use_i32, &rho[0], &e_qmax[0], &e_q[3],
                        &s[3], 8, &max_val);
                else { e_q[3] = 0; s[3] = 0; }
            } else { e_q[2] = 0; s[2] = 0; e_q[3] = 0; s[3] = 0; }
            }

            Uq0 = e_qmax[0] > 1 ? e_qmax[0] : 1;
            u_q0 = Uq0 - 1;
            if (u_q0 > 0) {
                eps0 |= (e_q[0] == e_qmax[0]);
                eps0 |= (e_q[1] == e_qmax[0]) << 1;
                eps0 |= (e_q[2] == e_qmax[0]) << 2;
                eps0 |= (e_q[3] == e_qmax[0]) << 3;
            }
            lep[0] = lep[0] > (uint8_t)e_q[1] ? lep[0] : (uint8_t)e_q[1];
            lep++;
            lep[0] = (uint8_t)e_q[3];
            lcxp[0] = (uint8_t)(lcxp[0] | (uint8_t)((rho[0] & 2) >> 1));
            lcxp++;
            lcxp[0] = (uint8_t)((rho[0] & 8) >> 3);

            tuple0 = g_vlc_enc_tbl0[(c_q0 << 8) + (rho[0] << 4) + eps0];
            rc = jph_vlc_enc_encode(&vlc, tuple0 >> 8, (tuple0 >> 4) & 7);
            if (rc != EXR_SUCCESS) return rc;
            if (c_q0 == 0) {
                rc = jph_mel_enc_encode(&mel, rho[0] != 0);
                if (rc != EXR_SUCCESS) return rc;
            }
            rc = jph_encode_mag_bits_quad(&ms, &s[0], rho[0], Uq0, tuple0);
            if (rc != EXR_SUCCESS) return rc;

            if (x + 2u < width) {
#if defined(EXR_X86)
                if (use_sse2 && height > 1u && x + 3u < width) {
                    jph_encode_prepare_quad_dispatch(plane_data, plane_data32, plane_stride,
                                                      cb_x0, cb_y0, x + 2u, 0u,
                                                      shift, p, &rho[1],
                                                      &e_qmax[1], e_q + 4, s + 4,
                                                      &max_val);
                } else
#endif
                {
                jph_encode_block_prepare_sample_fast(
                    plane_data, plane_data32, plane_stride, cb_x0, cb_y0, x + 2u, 0u, shift,
                    p, use_i32, &rho[1], &e_qmax[1], &e_q[4], &s[4], 1, &max_val);
                if (height > 1u)
                    jph_encode_block_prepare_sample_fast(
                        plane_data, plane_data32, plane_stride, cb_x0, cb_y0, x + 2u, 1u,
                        shift, p, use_i32, &rho[1], &e_qmax[1], &e_q[5],
                        &s[5], 2, &max_val);
                else { e_q[5] = 0; s[5] = 0; }
                if (x + 3u < width) {
                    jph_encode_block_prepare_sample_fast(
                        plane_data, plane_data32, plane_stride, cb_x0, cb_y0, x + 3u, 0u,
                        shift, p, use_i32, &rho[1], &e_qmax[1], &e_q[6],
                        &s[6], 4, &max_val);
                    if (height > 1u)
                        jph_encode_block_prepare_sample_fast(
                            plane_data, plane_data32, plane_stride, cb_x0, cb_y0, x + 3u,
                            1u, shift, p, use_i32, &rho[1], &e_qmax[1],
                            &e_q[7], &s[7], 8, &max_val);
                    else { e_q[7] = 0; s[7] = 0; }
                } else { e_q[6] = 0; s[6] = 0; e_q[7] = 0; s[7] = 0; }
                }

                c_q1 = (rho[0] >> 1) | (rho[0] & 1);
                Uq1 = e_qmax[1] > 1 ? e_qmax[1] : 1;
                u_q1 = Uq1 - 1;
                if (u_q1 > 0) {
                    eps1 |= (e_q[4] == e_qmax[1]);
                    eps1 |= (e_q[5] == e_qmax[1]) << 1;
                    eps1 |= (e_q[6] == e_qmax[1]) << 2;
                    eps1 |= (e_q[7] == e_qmax[1]) << 3;
                }
                lep[0] = lep[0] > (uint8_t)e_q[5] ? lep[0] : (uint8_t)e_q[5];
                lep++;
                lep[0] = (uint8_t)e_q[7];
                lcxp[0] = (uint8_t)(lcxp[0] |
                                    (uint8_t)((rho[1] & 2) >> 1));
                lcxp++;
                lcxp[0] = (uint8_t)((rho[1] & 8) >> 3);

                tuple1 = g_vlc_enc_tbl0[(c_q1 << 8) + (rho[1] << 4) + eps1];
                rc = jph_vlc_enc_encode(&vlc, tuple1 >> 8,
                                        (tuple1 >> 4) & 7);
                if (rc != EXR_SUCCESS) return rc;
                if (c_q1 == 0) {
                    rc = jph_mel_enc_encode(&mel, rho[1] != 0);
                    if (rc != EXR_SUCCESS) return rc;
                }
                rc = jph_encode_mag_bits_quad(&ms, &s[4], rho[1], Uq1, tuple1);
                if (rc != EXR_SUCCESS) return rc;
            }

            if (u_q0 > 0 && u_q1 > 0) {
                int min_uq = u_q0 < u_q1 ? u_q0 : u_q1;
                rc = jph_mel_enc_encode(&mel, min_uq > 2);
                if (rc != EXR_SUCCESS) return rc;
            }
            rc = jph_encode_uvlc_pair(&vlc, u_q0, u_q1, 1);
            if (rc != EXR_SUCCESS) return rc;
            c_q0 = (rho[1] >> 1) | (rho[1] & 1);
        }
    }

    lep[1] = 0;

    for (uint32_t y = 2u; y < height; y += 2u) {
        int max_e;
        lep = e_val;
        max_e = (lep[0] > lep[1] ? lep[0] : lep[1]) - 1;
        lep[0] = 0;
        lcxp = cx_val;
        c_q0 = lcxp[0] + (lcxp[1] << 2);
        lcxp[0] = 0;

        for (uint32_t x = 0u; x < width; x += 4u) {
            int e_qmax[2] = {0, 0};
            int e_q[8] = {0, 0, 0, 0, 0, 0, 0, 0};
            int rho[2] = {0, 0};
            uint64_t s[8] = {0, 0, 0, 0, 0, 0, 0, 0};
            int kappa, Uq0, Uq1 = 1, u_q0, u_q1 = 0;
            int eps0 = 0, eps1 = 0, c_q1;
            uint16_t tuple0, tuple1 = 0;

#if defined(EXR_X86)
            if (use_sse2 && y + 1u < height && x + 1u < width) {
                jph_encode_prepare_quad_dispatch(plane_data, plane_data32, plane_stride,
                                                  cb_x0, cb_y0, x, y,
                                                  shift, p, &rho[0],
                                                  &e_qmax[0], e_q, s,
                                                  &max_val);
            } else
#endif
            {
            jph_encode_block_prepare_sample_fast(plane_data, plane_data32, plane_stride,
                                                  cb_x0, cb_y0, x, y, shift, p,
                                                  use_i32, &rho[0],
                                                  &e_qmax[0], &e_q[0], &s[0],
                                                   1, &max_val);
            if (y + 1u < height)
                jph_encode_block_prepare_sample_fast(
                    plane_data, plane_data32, plane_stride, cb_x0, cb_y0, x, y + 1u, shift,
                    p, use_i32, &rho[0], &e_qmax[0], &e_q[1], &s[1], 2, &max_val);
            else { e_q[1] = 0; s[1] = 0; }
            if (x + 1u < width) {
                jph_encode_block_prepare_sample_fast(
                    plane_data, plane_data32, plane_stride, cb_x0, cb_y0, x + 1u, y, shift,
                    p, use_i32, &rho[0], &e_qmax[0], &e_q[2], &s[2], 4, &max_val);
                if (y + 1u < height)
                    jph_encode_block_prepare_sample_fast(
                        plane_data, plane_data32, plane_stride, cb_x0, cb_y0, x + 1u,
                        y + 1u, shift, p, use_i32, &rho[0], &e_qmax[0],
                        &e_q[3], &s[3], 8, &max_val);
                else { e_q[3] = 0; s[3] = 0; }
            } else { e_q[2] = 0; s[2] = 0; e_q[3] = 0; s[3] = 0; }
            }

            kappa = (rho[0] & (rho[0] - 1)) ? (max_e > 1 ? max_e : 1) : 1;
            Uq0 = e_qmax[0] > kappa ? e_qmax[0] : kappa;
            u_q0 = Uq0 - kappa;
            if (u_q0 > 0) {
                eps0 |= (e_q[0] == e_qmax[0]);
                eps0 |= (e_q[1] == e_qmax[0]) << 1;
                eps0 |= (e_q[2] == e_qmax[0]) << 2;
                eps0 |= (e_q[3] == e_qmax[0]) << 3;
            }
            lep[0] = lep[0] > (uint8_t)e_q[1] ? lep[0] : (uint8_t)e_q[1];
            lep++;
            max_e = (lep[0] > lep[1] ? lep[0] : lep[1]) - 1;
            lep[0] = (uint8_t)e_q[3];
            lcxp[0] = (uint8_t)(lcxp[0] | (uint8_t)((rho[0] & 2) >> 1));
            lcxp++;
            c_q1 = lcxp[0] + (lcxp[1] << 2);
            lcxp[0] = (uint8_t)((rho[0] & 8) >> 3);

            tuple0 = g_vlc_enc_tbl1[(c_q0 << 8) + (rho[0] << 4) + eps0];
            rc = jph_vlc_enc_encode(&vlc, tuple0 >> 8, (tuple0 >> 4) & 7);
            if (rc != EXR_SUCCESS) return rc;
            if (c_q0 == 0) {
                rc = jph_mel_enc_encode(&mel, rho[0] != 0);
                if (rc != EXR_SUCCESS) return rc;
            }
            rc = jph_encode_mag_bits_quad(&ms, &s[0], rho[0], Uq0, tuple0);
            if (rc != EXR_SUCCESS) return rc;

            if (x + 2u < width) {
#if defined(EXR_X86)
                if (use_sse2 && y + 1u < height && x + 3u < width) {
                    jph_encode_prepare_quad_dispatch(plane_data, plane_data32, plane_stride,
                                                      cb_x0, cb_y0, x + 2u, y,
                                                      shift, p, &rho[1],
                                                      &e_qmax[1], e_q + 4,
                                                      s + 4, &max_val);
                } else
#endif
                {
                jph_encode_block_prepare_sample_fast(
                    plane_data, plane_data32, plane_stride, cb_x0, cb_y0, x + 2u, y, shift,
                    p, use_i32, &rho[1], &e_qmax[1], &e_q[4], &s[4], 1, &max_val);
                if (y + 1u < height)
                    jph_encode_block_prepare_sample_fast(
                        plane_data, plane_data32, plane_stride, cb_x0, cb_y0, x + 2u,
                        y + 1u, shift, p, use_i32, &rho[1], &e_qmax[1],
                        &e_q[5], &s[5], 2, &max_val);
                else { e_q[5] = 0; s[5] = 0; }
                if (x + 3u < width) {
                    jph_encode_block_prepare_sample_fast(
                        plane_data, plane_data32, plane_stride, cb_x0, cb_y0, x + 3u, y,
                        shift, p, use_i32, &rho[1], &e_qmax[1], &e_q[6],
                        &s[6], 4, &max_val);
                    if (y + 1u < height)
                        jph_encode_block_prepare_sample_fast(
                            plane_data, plane_data32, plane_stride, cb_x0, cb_y0, x + 3u,
                            y + 1u, shift, p, use_i32, &rho[1], &e_qmax[1],
                            &e_q[7], &s[7], 8, &max_val);
                    else { e_q[7] = 0; s[7] = 0; }
                } else { e_q[6] = 0; s[6] = 0; e_q[7] = 0; s[7] = 0; }
                }

                kappa = (rho[1] & (rho[1] - 1)) ? (max_e > 1 ? max_e : 1) : 1;
                c_q1 |= ((rho[0] & 4) >> 1) | ((rho[0] & 8) >> 2);
                Uq1 = e_qmax[1] > kappa ? e_qmax[1] : kappa;
                u_q1 = Uq1 - kappa;
                if (u_q1 > 0) {
                    eps1 |= (e_q[4] == e_qmax[1]);
                    eps1 |= (e_q[5] == e_qmax[1]) << 1;
                    eps1 |= (e_q[6] == e_qmax[1]) << 2;
                    eps1 |= (e_q[7] == e_qmax[1]) << 3;
                }
                lep[0] = lep[0] > (uint8_t)e_q[5] ? lep[0] : (uint8_t)e_q[5];
                lep++;
                max_e = (lep[0] > lep[1] ? lep[0] : lep[1]) - 1;
                lep[0] = (uint8_t)e_q[7];
                lcxp[0] = (uint8_t)(lcxp[0] |
                                    (uint8_t)((rho[1] & 2) >> 1));
                lcxp++;
                c_q0 = lcxp[0] + (lcxp[1] << 2);
                lcxp[0] = (uint8_t)((rho[1] & 8) >> 3);

                tuple1 = g_vlc_enc_tbl1[(c_q1 << 8) + (rho[1] << 4) + eps1];
                rc = jph_vlc_enc_encode(&vlc, tuple1 >> 8,
                                        (tuple1 >> 4) & 7);
                if (rc != EXR_SUCCESS) return rc;
                if (c_q1 == 0) {
                    rc = jph_mel_enc_encode(&mel, rho[1] != 0);
                    if (rc != EXR_SUCCESS) return rc;
                }
                rc = jph_encode_mag_bits_quad(&ms, &s[4], rho[1], Uq1, tuple1);
                if (rc != EXR_SUCCESS) return rc;
            }

            rc = jph_encode_uvlc_pair(&vlc, u_q0, u_q1, 0);
            if (rc != EXR_SUCCESS) return rc;
            c_q0 |= ((rho[1] & 4) >> 1) | ((rho[1] & 8) >> 2);
        }
    }

    /* After tracking max_val during the main loop, handle all-zero/corrupt */
    if (max_val == 0) {
        *out_missing_msbs = kmax;
        out_lengths[0] = 0u;
        out_lengths[1] = 0u;
        *out_size = 0u;
        return EXR_SUCCESS;
    }
    if (max_val >= (UINT64_C(1) << kmax)) return EXR_ERROR_CORRUPT;

    /* Terminate encoders and assemble cleanup pass */
    rc = jph_ms_terminate(&ms);
    if (rc != EXR_SUCCESS) return rc;
    rc = jph_mel_vlc_terminate(&mel, &vlc);
    if (rc != EXR_SUCCESS) return rc;

    /* Assemble output: MagSgn segment + MEL+VLC segment + Scup */
    int lcup = (int)ms.pos + (int)mel.pos + (int)vlc.pos;
    int scup = (int)mel.pos + (int)vlc.pos;
    if (lcup < 2) { lcup = 2; scup = 2; }
    if (lcup > (int)out_cap) return EXR_ERROR_CORRUPT;

    /* Copy MagSgn bytes */
    if (ms.pos) memcpy(out_buf, ms_buf, ms.pos);
    /* Copy MEL bytes */
    if (mel.pos) memcpy(out_buf + ms.pos, mel_buf, mel.pos);
    /* Copy VLC bytes (already in reverse position) */
    if (vlc.pos) memcpy(out_buf + ms.pos + mel.pos, vlc_buf + vlc.cap - vlc.pos, vlc.pos);
    /* Write Scup footer */
    out_buf[lcup - 2] = (uint8_t)((out_buf[lcup - 2] & 0xF0) | (scup & 0x0F));
    out_buf[lcup - 1] = (uint8_t)((scup >> 4) & 0xFF);

    out_lengths[0] = (uint32_t)lcup;
    out_lengths[1] = 0;
    *out_size = (size_t)lcup;
    return EXR_SUCCESS;
}

/* ----------------------------------------------------------------------------
 * Codestream & HT wrapper writer (encode path).
 * ------------------------------------------------------------------------- */

static exr_result jph_write_soc(uint8_t **p, uint8_t *end) {
    if (*p + 2 > end) return EXR_ERROR_CORRUPT;
    *(*p)++ = 0xFF; *(*p)++ = 0x4F;
    return EXR_SUCCESS;
}

static exr_result jph_write_eoc(uint8_t **p, uint8_t *end) {
    if (*p + 2 > end) return EXR_ERROR_CORRUPT;
    *(*p)++ = 0xFF; *(*p)++ = 0xD9;
    return EXR_SUCCESS;
}

/* Write big-endian uint32 */
#define PUT_BE32(v) do {                                                  \
    *(*p)++ = (uint8_t)((uint32_t)(v) >> 24);                             \
    *(*p)++ = (uint8_t)(((uint32_t)(v) >> 16) & 0xFF);                    \
    *(*p)++ = (uint8_t)(((uint32_t)(v) >> 8) & 0xFF);                     \
    *(*p)++ = (uint8_t)((uint32_t)(v) & 0xFF);                            \
} while(0)

/* Write big-endian uint16 */
#define PUT_BE16(v) do {                                                    \
    *(*p)++ = (uint8_t)((uint16_t)(v) >> 8);                               \
    *(*p)++ = (uint8_t)((uint16_t)(v) & 0xFF);                             \
} while(0)

/* Case-insensitive equality of two NUL-terminated strings. */
static int jph_ci_streq(const char *a, const char *b) {
    size_t i = 0;
    for (;;) {
        unsigned char ca = (unsigned char)a[i], cb = (unsigned char)b[i];
        if (ca >= 'A' && ca <= 'Z') ca = (unsigned char)(ca - 'A' + 'a');
        if (cb >= 'A' && cb <= 'Z') cb = (unsigned char)(cb - 'A' + 'a');
        if (ca != cb) return 0;
        if (ca == 0) return 1;
        ++i;
    }
}

typedef struct {
    const char *rs, *gs, *bs;
    int ri, gi, bi;
    const char *prefix;
    size_t prefix_len;
} JphRgbParam;

/* Build the codestream->file channel map. Returns 1 and fills cs2f with
 * [R, G, B, <remaining file channels in file order>] when an RGB triplet is
 * detected (so the reversible color transform can decorrelate them and shrink
 * the payload), else returns 0 and fills cs2f with the identity. Mirrors
 * OpenEXR's make_channel_map heuristic (case-insensitive r/g/b or red/green/blue
 * suffix after an optional common "prefix." with matching type and sampling) so
 * the RCT grouping and the chunk channel-map header match OpenEXR/OpenJPH. */
static int jph_make_channel_map(const exr_codec_ctx *ctx, uint16_t *cs2f) {
    int nch = ctx->num_channels;
    JphRgbParam params[2];
    int pj, i, found = -1, avail;
    int r_index = -1, g_index = -1, b_index = -1;

    params[0].rs = "r";   params[0].gs = "g";     params[0].bs = "b";
    params[1].rs = "red"; params[1].gs = "green"; params[1].bs = "blue";
    for (pj = 0; pj < 2; ++pj) {
        params[pj].ri = params[pj].gi = params[pj].bi = -1;
        params[pj].prefix = NULL; params[pj].prefix_len = 0;
    }

    for (i = 0; i < nch; ++i) {
        const char *name = ctx->channels[i].name;
        const char *suffix = name, *prefix = name;
        size_t prefix_len = 0, k;
        for (k = 0; name[k]; ++k)
            if (name[k] == '.') { suffix = name + k + 1; prefix_len = k; }
        for (pj = 0; pj < 2; ++pj) {
            JphRgbParam *pp = &params[pj];
            int match = 0;
            if (pp->prefix != NULL &&
                (pp->prefix_len != prefix_len ||
                 memcmp(pp->prefix, prefix, prefix_len) != 0))
                continue;
            if (jph_ci_streq(suffix, pp->rs) && pp->ri < 0) { pp->ri = i; match = 1; }
            else if (jph_ci_streq(suffix, pp->gs) && pp->gi < 0) { pp->gi = i; match = 1; }
            else if (jph_ci_streq(suffix, pp->bs) && pp->bi < 0) { pp->bi = i; match = 1; }
            if (match) { pp->prefix = prefix; pp->prefix_len = prefix_len; break; }
        }
    }

    for (pj = 0; pj < 2 && found < 0; ++pj) {
        JphRgbParam *pp = &params[pj];
        if (pp->ri >= 0 && pp->gi >= 0 && pp->bi >= 0) {
            const exr_channel *r = &ctx->channels[pp->ri];
            const exr_channel *g = &ctx->channels[pp->gi];
            const exr_channel *b = &ctx->channels[pp->bi];
            if (r->pixel_type == g->pixel_type && r->pixel_type == b->pixel_type &&
                r->x_sampling == g->x_sampling && r->x_sampling == b->x_sampling &&
                r->y_sampling == g->y_sampling && r->y_sampling == b->y_sampling) {
                r_index = pp->ri; g_index = pp->gi; b_index = pp->bi; found = pj;
            }
        }
    }

    if (found < 0) {
        for (i = 0; i < nch; ++i) cs2f[i] = (uint16_t)i;
        return 0;
    }
    cs2f[0] = (uint16_t)r_index;
    cs2f[1] = (uint16_t)g_index;
    cs2f[2] = (uint16_t)b_index;
    avail = 3;
    for (i = 0; i < nch; ++i) {
        if (i == r_index || i == g_index || i == b_index) continue;
        cs2f[avail++] = (uint16_t)i;
    }
    return 1;
}

static exr_result jph_write_siz(uint8_t **p, uint8_t *end,
                                uint32_t w, uint32_t h, uint16_t nch,
                                const exr_codec_ctx *ctx,
                                const uint16_t *cs2f) {
    size_t sz = 38u + (size_t)nch * 3u;
    uint16_t i;
    if (*p + 2 + 2 + sz > end) return EXR_ERROR_CORRUPT;
    *(*p)++ = 0xFF; *(*p)++ = 0x51;  /* SIZ marker */
    PUT_BE16((uint16_t)sz);
    PUT_BE16(0x4000);  /* rsiz = HT profile */
    PUT_BE32(w);
    PUT_BE32(h);
    PUT_BE32(0);   /* xosiz */
    PUT_BE32(0);   /* yosiz */
    PUT_BE32(w);   /* xtsiz */
    PUT_BE32(h);   /* ytsiz */
    PUT_BE32(0);   /* xtosiz */
    PUT_BE32(0);   /* ytosiz */
    PUT_BE16(nch);
    for (i = 0; i < nch; ++i) {
        const exr_channel *ch = &ctx->channels[cs2f[i]];
        int is_signed = (ch->pixel_type != EXR_PIXEL_UINT) ? 1 : 0;
        int bit_depth = (ch->pixel_type == EXR_PIXEL_HALF) ? 16 : 32;
        uint8_t ssiz = (uint8_t)((is_signed << 7) | ((bit_depth - 1) & 0x7F));
        *(*p)++ = ssiz;
        *(*p)++ = (uint8_t)(ch->x_sampling > 0 ? ch->x_sampling : 1);
        *(*p)++ = (uint8_t)(ch->y_sampling > 0 ? ch->y_sampling : 1);
    }
    return EXR_SUCCESS;
}

static exr_result jph_write_cap(uint8_t **p, uint8_t *end) {
    if (*p + 10 > end) return EXR_ERROR_CORRUPT;
    *(*p)++ = 0xFF; *(*p)++ = 0x50;  /* CAP */
    PUT_BE16(8);
    PUT_BE32(0x00020000);  /* HTJ2K */
    PUT_BE16(0x000b);
    return EXR_SUCCESS;
}

static exr_result jph_write_cod(uint8_t **p, uint8_t *end,
                                int mc_trans) {
    if (*p + 14 > end) return EXR_ERROR_CORRUPT;
    *(*p)++ = 0xFF; *(*p)++ = 0x52;  /* COD */
    PUT_BE16(12);
    *(*p)++ = 0;   /* scod */
    *(*p)++ = 2;   /* prog = RPCL */
    PUT_BE16(1);  /* layers */
    *(*p)++ = (uint8_t)mc_trans;  /* mc */
    *(*p)++ = 5;   /* ndecomp = 5 */
    *(*p)++ = 5;   /* cb_w_log2 = 5 → 128 */
    *(*p)++ = 3;   /* cb_h_log2 = 3 → 32 */
    *(*p)++ = 0x40;/* style */
    *(*p)++ = 1;   /* wavelet = 5/3 reversible */
    return EXR_SUCCESS;
}

static exr_result jph_write_qcd(uint8_t **p, uint8_t *end,
                                uint32_t kmax) {
    uint16_t i;
    uint32_t guard_bits = 1u;
    uint32_t expn = kmax;
    uint8_t sp;
    if (*p + 21 > end) return EXR_ERROR_CORRUPT;
    if (kmax == 0u || kmax > 36u) return EXR_ERROR_INVALID_ARGUMENT;
    if (kmax > 30u) {
        expn = 30u;
        guard_bits = kmax - 29u;
    }
    if (guard_bits > 7u || expn > 31u) return EXR_ERROR_INVALID_ARGUMENT;
    sp = (uint8_t)(expn << 3u);
    *(*p)++ = 0xFF; *(*p)++ = 0x5C;  /* QCD */
    PUT_BE16(19);
    *(*p)++ = (uint8_t)(guard_bits << 5u);
    for (i = 0; i < 16; ++i) *(*p)++ = sp;
    return EXR_SUCCESS;
}

static exr_result jph_write_nlt(uint8_t **p, uint8_t *end,
                                uint16_t comp, uint8_t bit_depth,
                                int is_signed, uint8_t type) {
    if (*p + 8 > end) return EXR_ERROR_CORRUPT;
    *(*p)++ = 0xFF; *(*p)++ = 0x76;  /* NLT */
    PUT_BE16(6);
    PUT_BE16(comp);
    *(*p)++ = (uint8_t)((is_signed << 7) | (bit_depth - 1));
    *(*p)++ = type;
    return EXR_SUCCESS;
}

static exr_result jph_write_sot_sod(uint8_t **p, uint8_t *end,
                                    uint8_t **out_psot) {
    if (*p + 14 > end) return EXR_ERROR_CORRUPT;
    *(*p)++ = 0xFF; *(*p)++ = 0x90;  /* SOT */
    PUT_BE16(10);
    PUT_BE16(0);   /* isot = 0 */
    if (out_psot) *out_psot = *p;
    PUT_BE32(0);   /* psot = 0 (unknown) */
    *(*p)++ = 0;   /* tpsot */
    *(*p)++ = 1;   /* tnsot */
    *(*p)++ = 0xFF; *(*p)++ = 0x93;  /* SOD */
    return EXR_SUCCESS;
}

static exr_result jph_write_codestream(uint8_t **p, uint8_t *end,
                                        const exr_codec_ctx *ctx,
                                        int mc_trans, uint32_t kmax,
                                        uint8_t **out_psot,
                                        const uint16_t *cs2f) {
    exr_result rc;
    int i;
    rc = jph_write_soc(p, end);
    if (rc != EXR_SUCCESS) return rc;
    rc = jph_write_siz(p, end, (uint32_t)ctx->width,
                       (uint32_t)ctx->num_lines,
                       (uint16_t)ctx->num_channels, ctx, cs2f);
    if (rc != EXR_SUCCESS) return rc;
    rc = jph_write_cap(p, end);
    if (rc != EXR_SUCCESS) return rc;
    rc = jph_write_cod(p, end, mc_trans);
    if (rc != EXR_SUCCESS) return rc;
    rc = jph_write_qcd(p, end, kmax);
    if (rc != EXR_SUCCESS) return rc;
    for (i = 0; i < ctx->num_channels; ++i) {
        const exr_channel *ch = &ctx->channels[cs2f[i]];
        int bit_depth = (ch->pixel_type == EXR_PIXEL_HALF) ? 16 : 32;
        int is_signed = (ch->pixel_type != EXR_PIXEL_UINT) ? 1 : 0;
        uint8_t nlt_type = is_signed ? 3u : 0u;
        rc = jph_write_nlt(p, end, (uint16_t)i, (uint8_t)bit_depth,
                           is_signed, nlt_type);
        if (rc != EXR_SUCCESS) return rc;
    }
    rc = jph_write_sot_sod(p, end, out_psot);
    if (rc != EXR_SUCCESS) return rc;
    return EXR_SUCCESS;
}

static exr_result jph_write_ht_header(uint8_t **p, uint8_t *end,
                                       const exr_codec_ctx *ctx,
                                       const uint16_t *cs2f) {
    uint16_t i;
    size_t payload_size = 2u + (size_t)ctx->num_channels * 2u;
    if (*p + 8 + payload_size > end) return EXR_ERROR_CORRUPT;
    /* magic */
    *(*p)++ = 0x48; *(*p)++ = 0x54;
    /* payload length (big endian) */
    PUT_BE32((uint32_t)payload_size);
    /* channel map: codestream component i -> file channel cs2f[i] */
    PUT_BE16((uint16_t)ctx->num_channels);
    for (i = 0; i < ctx->num_channels; ++i) {
        PUT_BE16(cs2f[i]);
    }
    return EXR_SUCCESS;
}

/* Reusable band geometry builder for encoder (takes JphSize directly). */
static void jph_build_band_geoms_from_size(JphSize comp_size,
                                            uint32_t num_decomps,
                                            uint32_t res,
                                            JphBandGeom bands[4],
                                            JphSize *out_res_size) {
    JphSize rs;
    uint32_t shift = num_decomps - res;
    rs.w = jph_ceil_div_pow2_u32(comp_size.w, shift);
    rs.h = jph_ceil_div_pow2_u32(comp_size.h, shift);
    memset(bands, 0, sizeof(JphBandGeom) * 4u);
    if (out_res_size) *out_res_size = rs;

    if (res == 0u) {
        bands[0].w = rs.w; bands[0].h = rs.h;
        bands[0].exists = (rs.w != 0u && rs.h != 0u);
        return;
    }
    bands[1].w = rs.w >> 1u;  bands[1].h = (rs.h + 1u) >> 1u;
    bands[2].w = (rs.w + 1u) >> 1u; bands[2].h = rs.h >> 1u;
    bands[3].w = rs.w >> 1u;  bands[3].h = rs.h >> 1u;
    for (uint32_t b = 1; b < 4; ++b)
        bands[b].exists = (bands[b].w != 0u && bands[b].h != 0u);
}

typedef struct {
    uint8_t *p;
    uint8_t *end;
    uint8_t cur;
    uint8_t used;
    uint8_t prev_ff;
} JphPacketWriter;

typedef struct {
    uint8_t *data;
    size_t size;
    uint32_t missing_msbs;
    uint32_t length0;
    uint32_t length1;
} JphEncodedCb;

static void jph_packet_writer_init(JphPacketWriter *bw, uint8_t *p,
                                   uint8_t *end) {
    bw->p = p;
    bw->end = end;
    bw->cur = 0;
    bw->used = 0;
    bw->prev_ff = 0;
}

static exr_result jph_packet_writer_flush_byte(JphPacketWriter *bw) {
    uint8_t limit;
    if (!bw || bw->p >= bw->end) return EXR_ERROR_CORRUPT;
    limit = bw->prev_ff ? 7u : 8u;
    if (bw->used < limit) bw->cur = (uint8_t)(bw->cur << (limit - bw->used));
    *bw->p++ = bw->cur;
    bw->prev_ff = (limit == 8u && bw->cur == 0xffu);
    bw->cur = 0u;
    bw->used = 0u;
    return EXR_SUCCESS;
}

static exr_result jph_packet_put_bit(JphPacketWriter *bw, uint32_t bit) {
    uint8_t limit;
    if (!bw) return EXR_ERROR_INVALID_ARGUMENT;
    limit = bw->prev_ff ? 7u : 8u;
    bw->cur = (uint8_t)((bw->cur << 1u) | (bit & 1u));
    bw->used++;
    if (bw->used == limit) return jph_packet_writer_flush_byte(bw);
    return EXR_SUCCESS;
}

static exr_result jph_packet_put_bits(JphPacketWriter *bw, uint32_t value,
                                      uint32_t nbits) {
    while (nbits) {
        exr_result rc;
        nbits--;
        rc = jph_packet_put_bit(bw, (value >> nbits) & 1u);
        if (rc != EXR_SUCCESS) return rc;
    }
    return EXR_SUCCESS;
}

static exr_result jph_packet_put_zeros(JphPacketWriter *bw, uint32_t nbits) {
    while (nbits) {
        exr_result rc = jph_packet_put_bit(bw, 0u);
        if (rc != EXR_SUCCESS) return rc;
        nbits--;
    }
    return EXR_SUCCESS;
}

static exr_result jph_packet_writer_finish(JphPacketWriter *bw,
                                           uint8_t **out_p) {
    exr_result rc = EXR_SUCCESS;
    if (!bw || !out_p) return EXR_ERROR_INVALID_ARGUMENT;
    if (bw->used) rc = jph_packet_writer_flush_byte(bw);
    /* If the packet header ends on a 0xFF byte, JPEG2000 bit-stuffing requires
     * a trailing 0x00: the decoder consumes a stuffing byte after every 0xFF,
     * including at packet termination (jph_bitreader_terminate_packet reads one
     * when prev_ff is set). Omitting it desyncs the following codeblock data by
     * one byte whenever a header happens to end in 0xFF (data/geometry
     * dependent). Matches OpenJPH's packet-header termination. */
    if (rc == EXR_SUCCESS && bw->prev_ff) {
        if (bw->p >= bw->end) return EXR_ERROR_CORRUPT;
        *bw->p++ = 0x00u;
        bw->prev_ff = 0u;
    }
    if (rc == EXR_SUCCESS) *out_p = bw->p;
    return rc;
}

static exr_result jph_tag_tree_build_values(exr_jph_tag_tree *tree,
                                            const uint32_t *leaf_values) {
    uint32_t level;
    if (!tree || !leaf_values || !tree->value) return EXR_ERROR_INVALID_ARGUMENT;
    for (uint32_t y = 0; y < tree->height[0]; ++y) {
        for (uint32_t x = 0; x < tree->width[0]; ++x) {
            size_t idx = tree->offset[0] + (size_t)y * tree->width[0] + x;
            tree->value[idx] = leaf_values[(size_t)y * tree->width[0] + x];
        }
    }
    for (level = 1u; level < tree->num_levels; ++level) {
        uint32_t w = tree->width[level];
        uint32_t h = tree->height[level];
        uint32_t cw = tree->width[level - 1u];
        uint32_t ch = tree->height[level - 1u];
        for (uint32_t y = 0; y < h; ++y) {
            for (uint32_t x = 0; x < w; ++x) {
                uint32_t v = UINT32_MAX;
                for (uint32_t yy = 0; yy < 2u; ++yy) {
                    uint32_t cy = (y << 1u) + yy;
                    if (cy >= ch) continue;
                    for (uint32_t xx = 0; xx < 2u; ++xx) {
                        uint32_t cx = (x << 1u) + xx;
                        size_t cidx;
                        if (cx >= cw) continue;
                        cidx = tree->offset[level - 1u] +
                               (size_t)cy * cw + cx;
                        if (tree->value[cidx] < v) v = tree->value[cidx];
                    }
                }
                tree->value[tree->offset[level] + (size_t)y * w + x] = v;
            }
        }
    }
    memset(tree->known, 0, tree->node_count);
    return EXR_SUCCESS;
}

static exr_result jph_packet_write_tag_tree_leaf(JphPacketWriter *bw,
                                                 exr_jph_tag_tree *tree,
                                                 uint32_t leaf_x,
                                                 uint32_t leaf_y,
                                                 int is_mmsb) {
    uint32_t cur_lev;
    if (!bw || !tree || leaf_x >= tree->width[0] ||
        leaf_y >= tree->height[0])
        return EXR_ERROR_INVALID_ARGUMENT;
    for (cur_lev = tree->num_levels; cur_lev > 0u; --cur_lev) {
        uint32_t levm1 = cur_lev - 1u;
        uint32_t cx = leaf_x >> levm1;
        uint32_t cy = leaf_y >> levm1;
        size_t idx = tree->offset[levm1] +
                     (size_t)cy * tree->width[levm1] + cx;
        uint32_t parent = 0u;
        uint32_t child;
        exr_result rc;
        if (idx >= tree->node_count) return EXR_ERROR_CORRUPT;
        child = tree->value[idx];
        if (cur_lev < tree->num_levels) {
            uint32_t px = leaf_x >> cur_lev;
            uint32_t py = leaf_y >> cur_lev;
            size_t pidx = tree->offset[cur_lev] +
                          (size_t)py * tree->width[cur_lev] + px;
            if (pidx >= tree->node_count) return EXR_ERROR_CORRUPT;
            parent = tree->value[pidx];
        }
        if (!tree->known[idx]) {
            if (child < parent) return EXR_ERROR_CORRUPT;
            if (is_mmsb) {
                rc = jph_packet_put_zeros(bw, child - parent);
                if (rc != EXR_SUCCESS) return rc;
                rc = jph_packet_put_bit(bw, 1u);
            } else {
                if (child - parent > 1u) return EXR_ERROR_CORRUPT;
                rc = jph_packet_put_bit(bw, 1u - (child - parent));
            }
            if (rc != EXR_SUCCESS) return rc;
            tree->known[idx] = 1u;
        }
        if (!is_mmsb && child > 0u) break;
    }
    return EXR_SUCCESS;
}

static exr_result jph_packet_write_pass_lengths(JphPacketWriter *bw,
                                                uint32_t length0,
                                                uint32_t length1,
                                                uint32_t active_passes) {
    uint32_t bits1, bits2 = 0u, extra_bit, bits, i;
    exr_result rc;
    if (!bw || active_passes == 0u || active_passes > 3u || length0 == 0u)
        return EXR_ERROR_INVALID_ARGUMENT;
    bits1 = jph_bit_length_u32(length0);
    extra_bit = active_passes > 2u ? 1u : 0u;
    if (active_passes > 1u) bits2 = jph_bit_length_u32(length1);
    bits = bits1;
    if (bits2 > extra_bit && bits2 - extra_bit > bits) bits = bits2 - extra_bit;
    bits = bits > 3u ? bits - 3u : 0u;
    for (i = 0u; i < bits; ++i) {
        rc = jph_packet_put_bit(bw, 1u);
        if (rc != EXR_SUCCESS) return rc;
    }
    rc = jph_packet_put_bit(bw, 0u);
    if (rc != EXR_SUCCESS) return rc;
    rc = jph_packet_put_bits(bw, length0, bits + 3u);
    if (rc != EXR_SUCCESS) return rc;
    if (active_passes > 1u)
        rc = jph_packet_put_bits(bw, length1, bits + 3u + extra_bit);
    return rc;
}

static void jph_band_offsets(JphSize comp_size, uint32_t num_decomps,
                             uint32_t res, uint32_t band,
                             uint32_t *row_off, uint32_t *col_off) {
    JphSize rs = jph_resolution_size(comp_size, num_decomps, res);
    *row_off = 0u;
    *col_off = 0u;
    if (res == 0u || band == 0u) return;
    if (band == 1u) {
        *col_off = (rs.w + 1u) >> 1u;
    } else if (band == 2u) {
        *row_off = (rs.h + 1u) >> 1u;
    } else if (band == 3u) {
        *row_off = (rs.h + 1u) >> 1u;
        *col_off = (rs.w + 1u) >> 1u;
    }
}

static exr_result jph_encoded_cb_append(const exr_allocator *a,
                                        JphEncodedCb **items,
                                        size_t *count, size_t *cap,
                                        const JphEncodedCb *cb) {
    JphEncodedCb *tmp;
    size_t new_cap, bytes;
    if (!items || !count || !cap || !cb) return EXR_ERROR_INVALID_ARGUMENT;
    if (*count == *cap) {
        new_cap = *cap ? (*cap * 2u) : 16u;
        if (exr_mul_ovf(new_cap, sizeof(**items), &bytes))
            return EXR_ERROR_CORRUPT;
        tmp = (JphEncodedCb *)exr_malloc(a, bytes);
        if (!tmp) return EXR_ERROR_OUT_OF_MEMORY;
        if (*items && *count) memcpy(tmp, *items, *count * sizeof(**items));
        exr_free(a, *items);
        *items = tmp;
        *cap = new_cap;
    }
    (*items)[(*count)++] = *cb;
    return EXR_SUCCESS;
}

static void jph_encoded_cb_free_all(const exr_allocator *a,
                                    JphEncodedCb *items, size_t count) {
    size_t i;
    for (i = 0; i < count; ++i) exr_free(a, items[i].data);
    exr_free(a, items);
}

/* ---- GPU encode seam: collect code-block coefficient tiles -------------- */
typedef struct {
    const exr_allocator *a;
    exr_jph_enc_record *records;
    size_t count, cap;
    int32_t *coeffs;
    size_t coeff_count, coeff_cap;
    int err;
    /* When gpu_enc_fn is set, jph_compress_impl collects the plan, encodes all
     * blocks on the GPU, then assembles the codestream from those outputs. */
    exr_jph_gpu_block_encode_fn gpu_enc_fn;
    void *gpu_enc_user;
} JphEncCollect;

/* Precomputed per-code-block GPU encode outputs, consumed in code-block order
 * (res->comp->band->y->x) by jph_write_packet_for_component_res. */
typedef struct {
    const uint8_t *bytes;
    uint32_t out_stride;
    const uint32_t *missing;
    const uint32_t *len0;
    const uint32_t *size;
    size_t cursor;
} JphGpuEncOutputs;

static int jph_enc_collect_block(JphEncCollect *ec, const JphPlane64 *pl,
                                 uint32_t cb_x0, uint32_t cb_y0,
                                 uint32_t cb_w, uint32_t cb_h, uint32_t kmax) {
    exr_jph_enc_record *rec;
    size_t tile = (size_t)cb_w * cb_h, y, x;
    if (ec->count == ec->cap) {
        size_t ncap = ec->cap ? ec->cap * 2u : 256u;
        exr_jph_enc_record *nr =
            (exr_jph_enc_record *)exr_malloc(ec->a, ncap * sizeof(*nr));
        if (!nr) { ec->err = 1; return 0; }
        if (ec->records) {
            memcpy(nr, ec->records, ec->count * sizeof(*nr));
            exr_free(ec->a, ec->records);
        }
        ec->records = nr;
        ec->cap = ncap;
    }
    if (ec->coeff_count + tile > ec->coeff_cap) {
        size_t ncap = ec->coeff_cap ? ec->coeff_cap * 2u : 65536u;
        int32_t *nc;
        while (ncap < ec->coeff_count + tile) ncap *= 2u;
        nc = (int32_t *)exr_malloc(ec->a, ncap * sizeof(*nc));
        if (!nc) { ec->err = 1; return 0; }
        if (ec->coeffs) {
            memcpy(nc, ec->coeffs, ec->coeff_count * sizeof(*nc));
            exr_free(ec->a, ec->coeffs);
        }
        ec->coeffs = nc;
        ec->coeff_cap = ncap;
    }
    rec = &ec->records[ec->count++];
    rec->width = cb_w;
    rec->height = cb_h;
    rec->kmax = kmax;
    rec->plane_is_i32 = pl->data32 ? 1 : 0;
    rec->coeff_offset = ec->coeff_count;
    /* Copy the tile (stride cb_w). The GPU path handles only i32 planes; for
     * int64 planes record a narrowed copy (used only as a CPU-side fallback). */
    for (y = 0; y < cb_h; ++y) {
        for (x = 0; x < cb_w; ++x) {
            int64_t v = pl->data32
                ? (int64_t)pl->data32[(size_t)(cb_y0 + y) * pl->w + (cb_x0 + x)]
                : pl->data[(size_t)(cb_y0 + y) * pl->w + (cb_x0 + x)];
            ec->coeffs[ec->coeff_count + y * cb_w + x] = (int32_t)v;
        }
    }
    ec->coeff_count += tile;
    return 1;
}

static exr_result jph_collect_packet_blocks(const JphPlane64 *pl,
                                            JphSize comp_size, uint32_t res,
                                            uint32_t num_decomps, uint32_t kmax,
                                            JphEncCollect *ec) {
    JphBandGeom bands[4];
    int first_band, last_band, b;
    jph_build_band_geoms_from_size(comp_size, num_decomps, res, bands, NULL);
    first_band = (res == 0u) ? 0 : 1;
    last_band = (res == 0u) ? 0 : 3;
    for (b = first_band; b <= last_band; ++b) {
        uint32_t cbw, cbh, row_off = 0u, col_off = 0u, y, x;
        if (!bands[b].exists) continue;
        cbw = jph_divceil_u32(bands[b].w, 128u);
        cbh = jph_divceil_u32(bands[b].h, 32u);
        if (cbw == 0u || cbh == 0u) continue;
        jph_band_offsets(comp_size, num_decomps, res, (uint32_t)b, &row_off,
                         &col_off);
        for (y = 0u; y < cbh; ++y) {
            for (x = 0u; x < cbw; ++x) {
                uint32_t bx0 = x * 128u, by0 = y * 32u;
                uint32_t bwid = bands[b].w - bx0, bhgt = bands[b].h - by0;
                if (bwid > 128u) bwid = 128u;
                if (bhgt > 32u) bhgt = 32u;
                if (!jph_enc_collect_block(ec, pl, col_off + bx0, row_off + by0,
                                           bwid, bhgt, kmax))
                    return EXR_ERROR_OUT_OF_MEMORY;
            }
        }
    }
    return EXR_SUCCESS;
}

static exr_result jph_write_packet_for_component_res(const exr_allocator *a,
                                                     uint8_t **p,
                                                     uint8_t *end,
                                                     const JphPlane64 *pl,
                                                     JphSize comp_size,
                                                     uint32_t res,
                                                     uint32_t num_decomps,
                                                     uint32_t kmax,
                                                     JphGpuEncOutputs *gpu_out) {
    JphPacketWriter bw;
    JphEncodedCb *body = NULL;
    size_t body_count = 0u, body_cap = 0u;
    int saw_nonempty = 0;
    uint32_t skipped_subbands = 0u;
    exr_result rc = EXR_SUCCESS;

    if (!a) a = exr_default_allocator();
    if (!p || !*p || !pl || (!pl->data && !pl->data32))
        return EXR_ERROR_INVALID_ARGUMENT;
    jph_packet_writer_init(&bw, *p, end);

    {
        JphBandGeom bands[4];
        int first_band, last_band;
        jph_build_band_geoms_from_size(comp_size, num_decomps, res, bands,
                                       NULL);
        first_band = (res == 0u) ? 0 : 1;
        last_band = (res == 0u) ? 0 : 3;
        for (int b = first_band; b <= last_band; ++b) {
            uint32_t cbw, cbh, ncb = 0u, row_off = 0u, col_off = 0u;
            uint32_t *inc_values = NULL, *mmsb_values = NULL;
            JphEncodedCb *band_cbs = NULL;
            exr_jph_tag_tree inc_tree, mmsb_tree;
            int have_inc_tree = 0, have_mmsb_tree = 0;
            int band_nonempty = 0;
            size_t band_bytes;

            memset(&inc_tree, 0, sizeof(inc_tree));
            memset(&mmsb_tree, 0, sizeof(mmsb_tree));
            if (!bands[b].exists) continue;
            cbw = jph_divceil_u32(bands[b].w, 128u);
            cbh = jph_divceil_u32(bands[b].h, 32u);
            if (cbw == 0u || cbh == 0u) continue;
            if (exr_mul_ovf((size_t)cbw, (size_t)cbh, &band_bytes)) {
                rc = EXR_ERROR_CORRUPT;
                goto band_done;
            }
            if (band_bytes > UINT32_MAX ||
                exr_mul_ovf(band_bytes, sizeof(*band_cbs), &band_bytes)) {
                rc = EXR_ERROR_CORRUPT;
                goto band_done;
            }
            ncb = cbw * cbh;
            band_cbs = (JphEncodedCb *)exr_calloc(a, ncb, sizeof(*band_cbs));
            inc_values = (uint32_t *)exr_malloc(a, (size_t)ncb * sizeof(uint32_t));
            mmsb_values = (uint32_t *)exr_malloc(a, (size_t)ncb * sizeof(uint32_t));
            if (!band_cbs || !inc_values || !mmsb_values) {
                rc = EXR_ERROR_OUT_OF_MEMORY;
                goto band_done;
            }
            jph_band_offsets(comp_size, num_decomps, res, (uint32_t)b,
                             &row_off, &col_off);

            for (uint32_t y = 0u; y < cbh; ++y) {
                for (uint32_t x = 0u; x < cbw; ++x) {
                    uint32_t bx0 = x * 128u;
                    uint32_t by0 = y * 32u;
                    uint32_t bwid = bands[b].w - bx0;
                    uint32_t bhgt = bands[b].h - by0;
                    uint32_t missing = kmax;
                    uint32_t lengths[2] = {0u, 0u};
                    size_t out_sz = 0u, idx = (size_t)y * cbw + x;
                    uint8_t *coded;
                    size_t coded_cap = 65536u + 3072u + 2u;
                    if (bwid > 128u) bwid = 128u;
                    if (bhgt > 32u) bhgt = 32u;
                    coded = (uint8_t *)exr_malloc(a, coded_cap);
                    if (!coded) {
                        rc = EXR_ERROR_OUT_OF_MEMORY;
                        goto band_done;
                    }
                    if (gpu_out) {
                        /* Consume the next precomputed GPU block output (same
                         * code-block enumeration order as collection). */
                        size_t cur = gpu_out->cursor++;
                        out_sz = gpu_out->size[cur];
                        missing = gpu_out->missing[cur];
                        lengths[0] = gpu_out->len0[cur];
                        lengths[1] = 0u;
                        if (out_sz > coded_cap) { exr_free(a, coded); rc = EXR_ERROR_CORRUPT; goto band_done; }
                        if (out_sz)
                            memcpy(coded, gpu_out->bytes + cur * gpu_out->out_stride, out_sz);
                        rc = EXR_SUCCESS;
                    } else {
                        rc = jph_encode_block(pl->data32 ? NULL : pl->data,
                                              pl->data32, pl->w, col_off + bx0,
                                              row_off + by0, bwid, bhgt, kmax,
                                              &missing, lengths, coded,
                                              coded_cap, &out_sz);
                    }
                    if (rc != EXR_SUCCESS) {
                        exr_free(a, coded);
                        goto band_done;
                    }
                    if (out_sz == 0u) {
                        exr_free(a, coded);
                        inc_values[idx] = 1u;
                        mmsb_values[idx] = kmax;
                    } else {
                        band_cbs[idx].data = coded;
                        band_cbs[idx].size = out_sz;
                        band_cbs[idx].missing_msbs = missing;
                        band_cbs[idx].length0 = lengths[0];
                        band_cbs[idx].length1 = lengths[1];
                        inc_values[idx] = 0u;
                        mmsb_values[idx] = missing;
                        band_nonempty = 1;
                    }
                }
            }

            if (!band_nonempty) {
                if (saw_nonempty) {
                    rc = jph_packet_put_bit(&bw, 0u);
                    if (rc != EXR_SUCCESS) goto band_done;
                } else {
                    skipped_subbands++;
                }
                goto band_done;
            }
            if (!saw_nonempty) {
                rc = jph_packet_put_bit(&bw, 1u);
                if (rc != EXR_SUCCESS) goto band_done;
                rc = jph_packet_put_zeros(&bw, skipped_subbands);
                if (rc != EXR_SUCCESS) goto band_done;
                skipped_subbands = 0u;
                saw_nonempty = 1;
            }

            rc = exr_jph_tag_tree_init(a, &inc_tree, cbw, cbh);
            if (rc != EXR_SUCCESS) goto band_done;
            have_inc_tree = 1;
            rc = exr_jph_tag_tree_init(a, &mmsb_tree, cbw, cbh);
            if (rc != EXR_SUCCESS) goto band_done;
            have_mmsb_tree = 1;
            rc = jph_tag_tree_build_values(&inc_tree, inc_values);
            if (rc != EXR_SUCCESS) goto band_done;
            rc = jph_tag_tree_build_values(&mmsb_tree, mmsb_values);
            if (rc != EXR_SUCCESS) goto band_done;

            for (uint32_t y = 0u; y < cbh; ++y) {
                for (uint32_t x = 0u; x < cbw; ++x) {
                    size_t idx = (size_t)y * cbw + x;
                    rc = jph_packet_write_tag_tree_leaf(&bw, &inc_tree,
                                                        x, y, 0);
                    if (rc != EXR_SUCCESS) goto band_done;
                    if (inc_values[idx] != 0u) continue;
                    rc = jph_packet_write_tag_tree_leaf(&bw, &mmsb_tree,
                                                        x, y, 1);
                    if (rc != EXR_SUCCESS) goto band_done;
                    rc = jph_packet_put_bit(&bw, 0u);
                    if (rc != EXR_SUCCESS) goto band_done;
                    rc = jph_packet_write_pass_lengths(&bw,
                                                       band_cbs[idx].length0,
                                                       band_cbs[idx].length1,
                                                       1u);
                    if (rc != EXR_SUCCESS) goto band_done;
                }
            }

            for (uint32_t y = 0u; y < cbh; ++y) {
                for (uint32_t x = 0u; x < cbw; ++x) {
                    size_t idx = (size_t)y * cbw + x;
                    if (!band_cbs[idx].data) continue;
                    rc = jph_encoded_cb_append(a, &body, &body_count,
                                               &body_cap, &band_cbs[idx]);
                    if (rc != EXR_SUCCESS) goto band_done;
                    memset(&band_cbs[idx], 0, sizeof(band_cbs[idx]));
                }
            }

band_done:
            if (have_mmsb_tree) exr_jph_tag_tree_free(a, &mmsb_tree);
            if (have_inc_tree) exr_jph_tag_tree_free(a, &inc_tree);
            if (band_cbs) {
                for (uint32_t i = 0u; i < ncb; ++i)
                    exr_free(a, band_cbs[i].data);
            }
            exr_free(a, band_cbs);
            exr_free(a, inc_values);
            exr_free(a, mmsb_values);
            if (rc != EXR_SUCCESS) goto done;
        }
    }

    if (!saw_nonempty) {
        rc = jph_packet_put_bit(&bw, 0u);
        if (rc != EXR_SUCCESS) goto done;
    }
    rc = jph_packet_writer_finish(&bw, p);
    if (rc != EXR_SUCCESS) goto done;

    for (size_t i = 0u; i < body_count; ++i) {
        if ((size_t)(end - *p) < body[i].size) {
            rc = EXR_ERROR_CORRUPT;
            goto done;
        }
        memcpy(*p, body[i].data, body[i].size);
        *p += body[i].size;
    }

done:
    jph_encoded_cb_free_all(a, body, body_count);
    return rc;
}

/* ----------------------------------------------------------------------------
 * exr_jph_compress - main entry point.
 * ------------------------------------------------------------------------- */

static exr_result jph_compress_impl(const exr_codec_ctx *ctx,
                                    const uint8_t *block, size_t n,
                                    uint8_t **out_data, size_t *out_size,
                                    JphEncCollect *collect) {
    const exr_allocator *a = ctx->alloc ? ctx->alloc : exr_default_allocator();
    JphPlane64 *planes = NULL;
    uint8_t *buf = NULL;
    size_t buf_cap, buf_pos = 0;
    uint32_t c;
    exr_result rc;
    int mc_trans = 0;
    uint32_t kmax = 20u;
    uint16_t *cs2f = NULL;
    int is_rgb = 0;
    JphGpuEncOutputs gpu_out_storage;
    JphGpuEncOutputs *gpu_out_ptr = NULL;
    uint8_t *gpu_enc_bytes = NULL;
    uint32_t *gpu_enc_missing = NULL, *gpu_enc_len0 = NULL, *gpu_enc_size = NULL;

    if (out_data) *out_data = NULL;
    if (out_size) *out_size = 0;
    if (!ctx || !block) return EXR_ERROR_INVALID_ARGUMENT;
    if (!collect && (!out_data || !out_size)) return EXR_ERROR_INVALID_ARGUMENT;

    for (c = 0; c < (uint32_t)ctx->num_channels; ++c) {
        const exr_channel *ch = &ctx->channels[c];
        if (ch->pixel_type != EXR_PIXEL_HALF &&
            ch->pixel_type != EXR_PIXEL_FLOAT &&
            ch->pixel_type != EXR_PIXEL_UINT) {
            return EXR_ERROR_UNSUPPORTED;
        }
        if (ch->x_sampling < 0 || ch->x_sampling > 255 ||
            ch->y_sampling < 0 || ch->y_sampling > 255) {
            return EXR_ERROR_INVALID_ARGUMENT;
        }
        /* Any 32-bit-precision component (FLOAT/UINT) widens kmax to 33; this
         * single value is written in the QCD and applies to every component
         * (incl. mixed half/float files, which are supported). The 32-bit
         * components' reversible-5/3 coefficients can exceed int32, so the
         * encode pipeline below carries int64 coefficients. */
        if (ch->pixel_type == EXR_PIXEL_FLOAT ||
            ch->pixel_type == EXR_PIXEL_UINT) {
            kmax = 33u;
        }
    }

    /* Detect an RGB triplet and build the codestream->file channel map so the
     * reversible color transform decorrelates R/G/B (codestream comps 0,1,2)
     * even when other channels (e.g. alpha) are present. */
    cs2f = (uint16_t *)exr_malloc(a, (size_t)ctx->num_channels * sizeof(uint16_t));
    if (!cs2f) return EXR_ERROR_OUT_OF_MEMORY;
    is_rgb = jph_make_channel_map(ctx, cs2f);
    mc_trans = is_rgb;

    jph_ensure_uvlc_enc_tables();
    jph_ensure_vlc_enc_tables();

    /* Allocate component planes */
    {
        uint16_t nch = (uint16_t)ctx->num_channels;
        if (nch == 0) { rc = EXR_ERROR_INVALID_ARGUMENT; goto done; }
        planes = (JphPlane64 *)exr_calloc(a, nch, sizeof(*planes));
        if (!planes) { rc = EXR_ERROR_OUT_OF_MEMORY; goto done; }
        for (c = 0; c < nch; ++c) {
            const exr_channel *ch = &ctx->channels[c];
            int xs = ch->x_sampling > 0 ? ch->x_sampling : 1;
            int ys = ch->y_sampling > 0 ? ch->y_sampling : 1;
            size_t pw = (size_t)exr_num_samples(ctx->x, ctx->x + ctx->width - 1, xs);
            size_t ph = (size_t)exr_num_samples(ctx->y, ctx->y + ctx->num_lines - 1, ys);
            if (pw == 0 || ph == 0) { pw = 1; ph = 1; }
            planes[c].w = (uint32_t)pw;
            planes[c].h = (uint32_t)ph;
            if (exr_mul_ovf(pw, ph, &pw)) { rc = EXR_ERROR_CORRUPT; goto done; }
            if (exr_mul_ovf(pw, sizeof(int64_t), &pw)) { rc = EXR_ERROR_CORRUPT; goto done; }
            /* The all-HALF path (kmax==20) carries coefficients in int32 planes
             * (i32_planes) and never touches this int64 buffer, so allocating it
             * here is pure waste (a large calloc+free per chunk). Only the int64
             * path (FLOAT/UINT, kmax>20) needs planes[c].data. */
            if (kmax != 20u) {
                planes[c].data = (int64_t *)exr_calloc(a, pw ? pw : 1u, 1);
                if (!planes[c].data) { rc = EXR_ERROR_OUT_OF_MEMORY; goto done; }
            }
        }
    }

    /* For all-HALF parts, use the int32 pipeline (faster wavelet, less memory
     * bandwidth), then widen back to int64 for the codeblock encoder. */
    if (kmax == 20u) {
        uint16_t nch = (uint16_t)ctx->num_channels;
        int32_t **i32_planes = NULL;

        i32_planes = (int32_t **)exr_calloc(a, nch, sizeof(*i32_planes));
        if (!i32_planes) { rc = EXR_ERROR_OUT_OF_MEMORY; goto done; }

        for (c = 0; c < nch; ++c) {
            size_t count = (size_t)planes[c].w * planes[c].h;
            size_t alloc_bytes;
            if (exr_mul_ovf(count, sizeof(int32_t), &alloc_bytes)) {
                for (uint32_t j = 0; j < c; ++j) exr_free(a, i32_planes[j]);
                exr_free(a, i32_planes); rc = EXR_ERROR_CORRUPT; goto done;
            }
            i32_planes[c] = (int32_t *)exr_malloc(a, alloc_bytes ? alloc_bytes : 1u);
            if (!i32_planes[c]) {
                for (uint32_t j = 0; j < c; ++j) exr_free(a, i32_planes[j]);
                exr_free(a, i32_planes); rc = EXR_ERROR_OUT_OF_MEMORY; goto done;
            }
        }

        /* Deinterleave to int32 (HALF pixels: 2 bytes -> sign-extend to int32) */
        {
            size_t off = 0;
#if defined(EXR_X86)
            int use_avx2 = (exr_cpu_caps() & EXR_SIMD_AVX2) != 0;
#endif
            for (uint32_t y = 0; y < (uint32_t)ctx->num_lines; ++y) {
                int32_t yy = ctx->y + (int32_t)y;
                for (c = 0; c < nch; ++c) {
                    const exr_channel *ch = &ctx->channels[c];
                    int32_t xs = ch->x_sampling > 0 ? ch->x_sampling : 1;
                    int32_t ys = ch->y_sampling > 0 ? ch->y_sampling : 1;
                    int32_t nx, row_i;
                    if ((yy % ys) != 0) continue;
                    nx = exr_num_samples(ctx->x, ctx->x + ctx->width - 1, xs);
                    if (nx < 0) nx = 0;
                    row_i = exr_num_samples(ctx->y, yy, ys) - 1;
                    if (row_i < 0) { rc = EXR_ERROR_CORRUPT; goto done; }
                    {
                        size_t needed = (size_t)nx * 2u;
                        size_t dst_off = (size_t)row_i * planes[c].w;
                        if (off + needed > n) { rc = EXR_ERROR_CORRUPT; goto done; }
#if defined(EXR_X86)
                        {
                            size_t processed;
                            if (use_avx2)
                                processed = jph_deinterleave_half_avx2(
                                    block + off, i32_planes[c] + dst_off,
                                    (size_t)nx);
                            else
                                processed = jph_deinterleave_half_sse2(
                                    block + off, i32_planes[c] + dst_off,
                                    (size_t)nx);
                            off += processed * 2u;
                            (void)processed;
                        }
#else
                        {
                            for (uint32_t x = 0; x < (uint32_t)nx; ++x) {
                                size_t idx = dst_off + x;
                                int16_t v = (int16_t)(block[off] |
                                            ((uint16_t)block[off+1] << 8));
                                i32_planes[c][idx] = (int32_t)v;
                                off += 2;
                            }
                        }
#endif
                        /* Fused forward NLT type-3 on the row just written, while
                         * it is still hot in cache (avoids a separate cold full-
                         * plane pass). NLT type-3 is an involution, so forward ==
                         * the vectorized inverse apply (~v-biasm1 == -v-bias for
                         * 16-bit input). Byte-identical to the prior per-plane
                         * pass. */
                        rc = exr_jph_apply_nlt_type3_i32(i32_planes[c] + dst_off,
                                                         (size_t)nx, 16u);
                        if (rc != EXR_SUCCESS) {
                            for (uint32_t j = 0; j < nch; ++j) exr_free(a, i32_planes[j]);
                            exr_free(a, i32_planes); goto done;
                        }
                    }
                }
            }
            if (off != n) { rc = EXR_ERROR_CORRUPT; goto done; }
        }

        /* Forward RCT int32 on the detected R/G/B planes (codestream comps
         * 0,1,2 = file channels cs2f[0..2]). */
        if (is_rgb) {
            uint16_t ri = cs2f[0], gi = cs2f[1], bi = cs2f[2];
            size_t cr = (size_t)planes[ri].w * planes[ri].h;
            rc = exr_jph_forward_rct_i32(i32_planes[ri], i32_planes[gi],
                                         i32_planes[bi], cr);
            if (rc != EXR_SUCCESS) {
                for (uint32_t j = 0; j < nch; ++j) exr_free(a, i32_planes[j]);
                exr_free(a, i32_planes); goto done;
            }
        }

        /* Forward 5/3 wavelet int32 */
        for (c = 0; c < nch; ++c) {
            rc = exr_jph_forward_53_2d_i32(a, i32_planes[c],
                                           planes[c].w, planes[c].h, 5);
            if (rc != EXR_SUCCESS) {
                for (uint32_t j = 0; j < nch; ++j) exr_free(a, i32_planes[j]);
                exr_free(a, i32_planes); goto done;
            }
        }

        /* Hand the int32 coefficient planes to the codeblock encoder directly
         * (no int32->int64 widening): the all-HALF reversible-5/3 coefficients
         * fit int32 and the encoder reads them natively. The now-unused int64
         * plane is released; ownership of each i32 plane transfers to data32. */
        for (c = 0; c < nch; ++c) {
            exr_free(a, planes[c].data);
            planes[c].data = NULL;
            planes[c].data32 = i32_planes[c];
        }
        exr_free(a, i32_planes);
    } else {
        /* Original int64 path for components with >16-bit precision */
        rc = jph_deinterleave_block(ctx, block, n, planes);
        if (rc != EXR_SUCCESS) goto done;

        for (c = 0; c < (uint32_t)ctx->num_channels; ++c) {
            size_t count = (size_t)planes[c].w * planes[c].h;
            const exr_channel *ch = &ctx->channels[c];
            if (ch->pixel_type != EXR_PIXEL_UINT) {
                uint32_t bit_depth = ch->pixel_type == EXR_PIXEL_HALF ? 16u : 32u;
                rc = jph_forward_nlt_type3_i64(planes[c].data, count, bit_depth);
                if (rc != EXR_SUCCESS) goto done;
            }
        }

        /* Forward RCT int64 on the detected R/G/B planes (codestream comps
         * 0,1,2 = file channels cs2f[0..2]); matching type/sampling is
         * guaranteed by the RGB detection. */
        if (is_rgb) {
            uint16_t ri = cs2f[0], gi = cs2f[1], bi = cs2f[2];
            size_t count0 = (size_t)planes[ri].w * planes[ri].h;
            rc = jph_forward_rct_i64(planes[ri].data, planes[gi].data,
                                     planes[bi].data, count0);
            if (rc != EXR_SUCCESS) goto done;
        }

        for (c = 0; c < (uint32_t)ctx->num_channels; ++c) {
            rc = jph_forward_53_2d_i64(a, planes[c].data, planes[c].w,
                                       planes[c].h, 5);
            if (rc != EXR_SUCCESS) goto done;
        }
    }

    /* GPU encode seam: collect each code-block's coefficient tile + dims (in
     * res->comp->band->y->x order, matching the packet writer). */
    if (collect) {
        for (uint32_t res = 0u; res <= 5u; ++res) {
            for (c = 0; c < (uint32_t)ctx->num_channels; ++c) {
                JphPlane64 *pl = &planes[cs2f[c]];
                JphSize cs;
                cs.w = pl->w;
                cs.h = pl->h;
                rc = jph_collect_packet_blocks(pl, cs, res, 5u, kmax, collect);
                if (rc != EXR_SUCCESS) goto done;
            }
        }
        if (collect->err) { rc = EXR_ERROR_OUT_OF_MEMORY; goto done; }
        if (!collect->gpu_enc_fn) { rc = EXR_SUCCESS; goto done; } /* pure collect */

        /* Whole-image GPU encode: encode all blocks on the device, then fall
         * through to assemble the codestream from those outputs. Fall back to
         * the CPU codec if any block is not i32-eligible. */
        {
            const uint32_t ENC_STRIDE = 20480u;
            size_t i, nrec = collect->count;
            exr_jph_enc_plan plan;
            for (i = 0; i < nrec; ++i)
                if (!collect->records[i].plane_is_i32 ||
                    collect->records[i].kmax < 1u ||
                    collect->records[i].kmax > 30u) {
                    rc = EXR_ERROR_UNSUPPORTED;
                    goto done;
                }
            gpu_enc_bytes = (uint8_t *)exr_malloc(a, (nrec ? nrec : 1u) * ENC_STRIDE);
            gpu_enc_missing = (uint32_t *)exr_malloc(a, (nrec ? nrec : 1u) * sizeof(uint32_t));
            gpu_enc_len0 = (uint32_t *)exr_malloc(a, (nrec ? nrec : 1u) * sizeof(uint32_t));
            gpu_enc_size = (uint32_t *)exr_malloc(a, (nrec ? nrec : 1u) * sizeof(uint32_t));
            if (!gpu_enc_bytes || !gpu_enc_missing || !gpu_enc_len0 || !gpu_enc_size) {
                rc = EXR_ERROR_OUT_OF_MEMORY;
                goto done;
            }
            memset(&plan, 0, sizeof(plan));
            plan.records = collect->records;
            plan.num_records = nrec;
            plan.coeffs = collect->coeffs;
            plan.coeff_count = collect->coeff_count;
            rc = collect->gpu_enc_fn(collect->gpu_enc_user, &plan, gpu_enc_bytes,
                                     ENC_STRIDE, gpu_enc_missing, gpu_enc_len0,
                                     gpu_enc_size);
            if (rc != EXR_SUCCESS) goto done;
            gpu_out_storage.bytes = gpu_enc_bytes;
            gpu_out_storage.out_stride = ENC_STRIDE;
            gpu_out_storage.missing = gpu_enc_missing;
            gpu_out_storage.len0 = gpu_enc_len0;
            gpu_out_storage.size = gpu_enc_size;
            gpu_out_storage.cursor = 0;
            gpu_out_ptr = &gpu_out_storage;
        }
    }

    /* Estimate output buffer size */
    if (exr_mul_ovf(n ? n : 1u, 3u, &buf_cap) ||
        exr_add_ovf(buf_cap, 65536u, &buf_cap)) {
        rc = EXR_ERROR_CORRUPT;
        goto done;
    }
    buf = (uint8_t *)exr_malloc(a, buf_cap);
    if (!buf) { rc = EXR_ERROR_OUT_OF_MEMORY; goto done; }

    {
        uint8_t *p = buf, *end = buf + buf_cap;
        uint8_t *psot_ptr = NULL;

        rc = jph_write_ht_header(&p, end, ctx, cs2f);
        if (rc != EXR_SUCCESS) goto done;

        rc = jph_write_codestream(&p, end, ctx, mc_trans, kmax, &psot_ptr, cs2f);
        if (rc != EXR_SUCCESS) goto done;

        for (uint32_t res = 0u; res <= 5u; ++res) {
            for (c = 0; c < (uint32_t)ctx->num_channels; ++c) {
                JphPlane64 *pl = &planes[cs2f[c]];
                JphSize cs;
                cs.w = pl->w;
                cs.h = pl->h;
                rc = jph_write_packet_for_component_res(a, &p, end, pl, cs,
                                                        res, 5u, kmax,
                                                        gpu_out_ptr);
                if (rc != EXR_SUCCESS) goto done;
            }
        }

        if (psot_ptr) {
            uint8_t *sot_start = psot_ptr - 6;
            size_t psot = (size_t)(p - sot_start);
            if (psot > UINT32_MAX) { rc = EXR_ERROR_CORRUPT; goto done; }
            psot_ptr[0] = (uint8_t)((uint32_t)psot >> 24);
            psot_ptr[1] = (uint8_t)(((uint32_t)psot >> 16) & 0xffu);
            psot_ptr[2] = (uint8_t)(((uint32_t)psot >> 8) & 0xffu);
            psot_ptr[3] = (uint8_t)((uint32_t)psot & 0xffu);
        }

        /* Write EOC */
        rc = jph_write_eoc(&p, end);
        if (rc != EXR_SUCCESS) goto done;

        buf_pos = (size_t)(p - buf);
    }

    {
        uint8_t *final = (uint8_t *)exr_malloc(a, buf_pos);
        if (!final) { rc = EXR_ERROR_OUT_OF_MEMORY; goto done; }
        memcpy(final, buf, buf_pos);
        *out_data = final;
        *out_size = buf_pos;
    }

done:
    if (rc != EXR_SUCCESS && out_data) {
        exr_free(a, *out_data);
        *out_data = NULL;
        if (out_size) *out_size = 0;
    }
    exr_free(a, buf);
    exr_free(a, cs2f);
    exr_free(a, gpu_enc_bytes);
    exr_free(a, gpu_enc_missing);
    exr_free(a, gpu_enc_len0);
    exr_free(a, gpu_enc_size);
    if (planes) {
        for (c = 0; c < (uint32_t)ctx->num_channels; ++c) {
            exr_free(a, planes[c].data);
            exr_free(a, planes[c].data32);
        }
        exr_free(a, planes);
    }
    if (rc != EXR_SUCCESS && out_data) { *out_data = NULL; if (out_size) *out_size = 0; }
    return rc;
}

exr_result exr_jph_compress(const exr_codec_ctx *ctx, const uint8_t *block,
                            size_t n, uint8_t **out_data, size_t *out_size) {
    return jph_compress_impl(ctx, block, n, out_data, out_size, NULL);
}

exr_result exr_jph_compress_gpu(const exr_codec_ctx *ctx, const uint8_t *block,
                                size_t n, uint8_t **out_data, size_t *out_size,
                                exr_jph_gpu_block_encode_fn fn, void *user) {
    JphEncCollect ec;
    exr_result rc;
    if (!ctx || !block || !out_data || !out_size || !fn)
        return EXR_ERROR_INVALID_ARGUMENT;
    memset(&ec, 0, sizeof(ec));
    ec.a = ctx->alloc ? ctx->alloc : exr_default_allocator();
    ec.gpu_enc_fn = fn;
    ec.gpu_enc_user = user;
    rc = jph_compress_impl(ctx, block, n, out_data, out_size, &ec);
    exr_free(ec.a, ec.records);
    exr_free(ec.a, ec.coeffs);
    return rc;
}

/* ---- GPU encode seam: public-internal entry points ---------------------- */
exr_result exr_jph_collect_encode_blocks(const exr_codec_ctx *ctx,
                                         const uint8_t *block, size_t n,
                                         exr_jph_enc_plan *out) {
    JphEncCollect ec;
    exr_result rc;
    if (!ctx || !block || !out) return EXR_ERROR_INVALID_ARGUMENT;
    memset(out, 0, sizeof(*out));
    memset(&ec, 0, sizeof(ec));
    ec.a = ctx->alloc ? ctx->alloc : exr_default_allocator();
    rc = jph_compress_impl(ctx, block, n, NULL, NULL, &ec);
    if (rc != EXR_SUCCESS || ec.err) {
        exr_free(ec.a, ec.records);
        exr_free(ec.a, ec.coeffs);
        return ec.err ? EXR_ERROR_OUT_OF_MEMORY : rc;
    }
    out->records = ec.records;
    out->num_records = ec.count;
    out->coeffs = ec.coeffs;
    out->coeff_count = ec.coeff_count;
    return EXR_SUCCESS;
}

void exr_jph_enc_plan_free(const exr_allocator *a, exr_jph_enc_plan *plan) {
    if (!plan) return;
    if (!a) a = exr_default_allocator();
    exr_free(a, plan->records);
    exr_free(a, plan->coeffs);
    memset(plan, 0, sizeof(*plan));
}

exr_result exr_jph_ht_enc_tables(const uint16_t **vlc0, const uint16_t **vlc1,
                                 const uint8_t **uvlc_packed) {
    static uint8_t packed[75 * 6];
    static int packed_ready = 0;
    int i;
    exr_jph_warmup_encode_tables();
    if (!packed_ready) {
        for (i = 0; i < 75; ++i) {
            packed[i * 6 + 0] = g_uvlc_enc_tbl[i].pre;
            packed[i * 6 + 1] = g_uvlc_enc_tbl[i].pre_len;
            packed[i * 6 + 2] = g_uvlc_enc_tbl[i].suf;
            packed[i * 6 + 3] = g_uvlc_enc_tbl[i].suf_len;
            packed[i * 6 + 4] = g_uvlc_enc_tbl[i].ext;
            packed[i * 6 + 5] = g_uvlc_enc_tbl[i].ext_len;
        }
        packed_ready = 1;
    }
    if (vlc0) *vlc0 = g_vlc_enc_tbl0;
    if (vlc1) *vlc1 = g_vlc_enc_tbl1;
    if (uvlc_packed) *uvlc_packed = packed;
    return EXR_SUCCESS;
}

exr_result exr_jph_encode_one_block_i32(const exr_jph_enc_record *rec,
                                        const int32_t *coeffs, uint8_t *out,
                                        size_t out_cap, uint32_t *out_missing,
                                        uint32_t *out_len0, size_t *out_size) {
    uint32_t lengths[2] = {0u, 0u};
    uint32_t missing = rec ? rec->kmax : 0u;
    exr_result rc;
    if (!rec || !coeffs || !out || !out_size) return EXR_ERROR_INVALID_ARGUMENT;
    exr_jph_warmup_encode_tables();
    rc = jph_encode_block(NULL, coeffs, rec->width, 0u, 0u, rec->width,
                          rec->height, rec->kmax, &missing, lengths, out,
                          out_cap, out_size);
    if (out_missing) *out_missing = missing;
    if (out_len0) *out_len0 = lengths[0];
    return rc;
}
