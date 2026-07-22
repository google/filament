/*
 * TinyEXR texcomp - BC7/DDS implementation.
 *
 * Copyright (c) 2014-2026 Syoyo Fujita and TinyEXR authors
 * SPDX-License-Identifier: Apache-2.0
 */

#include "texcomp.h"
#include "texcomp_internal.h"

#include <limits.h>
#include <stdlib.h>
#include <string.h>

#if defined(__i386__) || defined(__x86_64__) || defined(_M_IX86) || defined(_M_X64)
#define TC_X86 1
#if defined(_MSC_VER)
#include <intrin.h>
#else
#include <cpuid.h>
#include <immintrin.h>
#endif
#endif

#if defined(__ARM_NEON) || defined(__ARM_NEON__)
#include <arm_neon.h>
#endif

#if defined(__GNUC__) || defined(__clang__)
#define TC_TARGET(x) __attribute__((target(x)))
#else
#define TC_TARGET(x)
#endif

#define TC_CPU_SSE2 1u
#define TC_CPU_SSE41 2u
#define TC_CPU_AVX2 4u
#define TC_CPU_NEON 8u
#define TC_CPU_SVE 16u


const char *tc_result_string(tc_result r) {
    switch (r) {
        case TC_SUCCESS: return "success";
        case TC_ERROR_INVALID_ARGUMENT: return "invalid argument";
        case TC_ERROR_OUT_OF_MEMORY: return "out of memory";
        case TC_ERROR_IO: return "I/O error";
        case TC_ERROR_UNSUPPORTED: return "unsupported";
        case TC_ERROR_CORRUPT: return "corrupt data";
        default: return "unknown error";
    }
}

#if defined(TC_X86)
#if defined(_MSC_VER)
static void tc_cpuidex(int out[4], int leaf, int subleaf) {
    __cpuidex(out, leaf, subleaf);
}
static unsigned long long tc_xgetbv0(void) { return _xgetbv(0); }
#else
static void tc_cpuidex(int out[4], int leaf, int subleaf) {
    unsigned int a, b, c, d;
    if (!__get_cpuid_count((unsigned int)leaf, (unsigned int)subleaf, &a, &b, &c, &d)) {
        out[0] = out[1] = out[2] = out[3] = 0;
        return;
    }
    out[0] = (int)a;
    out[1] = (int)b;
    out[2] = (int)c;
    out[3] = (int)d;
}
static unsigned long long tc_xgetbv0(void) {
    unsigned int eax, edx;
    __asm__ volatile("xgetbv" : "=a"(eax), "=d"(edx) : "c"(0));
    return ((unsigned long long)edx << 32) | eax;
}
#endif

static uint32_t tc_detect_cpu_caps(void) {
    int r[4];
    int max_leaf, os_ymm = 0;
    uint32_t caps = 0;
    tc_cpuidex(r, 0, 0);
    max_leaf = r[0];
    if (max_leaf < 1) return 0;
    tc_cpuidex(r, 1, 0);
    if (r[3] & (1 << 26)) caps |= TC_CPU_SSE2;
    if (r[2] & (1 << 19)) caps |= TC_CPU_SSE41;
    if ((r[2] & (1 << 27)) && (r[2] & (1 << 28)))
        os_ymm = (tc_xgetbv0() & 0x6) == 0x6;
    if (os_ymm && max_leaf >= 7) {
        tc_cpuidex(r, 7, 0);
        if (r[1] & (1 << 5)) caps |= TC_CPU_AVX2;
    }
    return caps;
}
#else
static uint32_t tc_detect_cpu_caps(void) {
    uint32_t caps = 0;
#if defined(__ARM_FEATURE_SVE)
    caps |= TC_CPU_SVE;
#endif
#if defined(__ARM_NEON) || defined(__ARM_NEON__)
    caps |= TC_CPU_NEON;
#endif
    return caps;
}
#endif

static uint32_t tc_backend_force = TC_BACKEND_ALL;

uint32_t tc_backend_available_mask(void) {
    static int ready = 0;
    static uint32_t caps = 0;
    if (!ready) {
        caps = tc_detect_cpu_caps();
        ready = 1;
    }
    return caps;
}

void tc_backend_force_mask(uint32_t mask) { tc_backend_force = mask; }

uint32_t tc_cpu_caps(void) { return tc_backend_available_mask() & tc_backend_force; }

const char *tc_backend_name(void) {
    uint32_t caps = tc_cpu_caps();
    if (caps & TC_CPU_AVX2) return "avx2";
    if (caps & TC_CPU_SSE41) return "sse4.1";
    if (caps & TC_CPU_SSE2) return "sse2";
    if (caps & TC_CPU_SVE) return "sve";
    if (caps & TC_CPU_NEON) return "neon";
    return "scalar";
}

void tc_bc1_options_init(tc_bc1_options *opt) {
    if (!opt) return;
    memset(opt, 0, sizeof(*opt));
}

void tc_bc3_options_init(tc_bc3_options *opt) {
    if (!opt) return;
    memset(opt, 0, sizeof(*opt));
}

void tc_bc5_options_init(tc_bc5_options *opt) {
    if (!opt) return;
    memset(opt, 0, sizeof(*opt));
}

void tc_astc_options_init(tc_astc_options *opt) {
    if (!opt) return;
    memset(opt, 0, sizeof(*opt));
    opt->block_x = 6;
    opt->block_y = 6;
    opt->quality = 2;
    opt->threads = 1;
}

int tc_mul_ovf_size(size_t a, size_t b, size_t *out) {
#if defined(__GNUC__) || defined(__clang__)
    return __builtin_mul_overflow(a, b, out);
#else
    if (a != 0 && b > SIZE_MAX / a) return 1;
    *out = a * b;
    return 0;
#endif
}

size_t tc_bc1_compressed_size(uint32_t width, uint32_t height) {
    size_t bc = tc_bc7_compressed_size(width, height);
    return bc ? bc / 2u : 0u; /* BC1: 8 bytes per 4x4 block */
}

size_t tc_bc3_compressed_size(uint32_t width, uint32_t height) {
    return tc_bc7_compressed_size(width, height); /* BC3: 16 bytes per block */
}

size_t tc_bc5_compressed_size(uint32_t width, uint32_t height) {
    return tc_bc7_compressed_size(width, height);
}

size_t tc_eac_r11_compressed_size(uint32_t width, uint32_t height) {
    return tc_etc2_rgb_compressed_size(width, height);
}

size_t tc_eac_rg11_compressed_size(uint32_t width, uint32_t height) {
    return tc_etc2_rgba_compressed_size(width, height);
}

int tc_astc_valid_block(uint32_t bx, uint32_t by) {
    return (bx == 4u && by == 4u) || (bx == 5u && by == 4u) ||
           (bx == 5u && by == 5u) || (bx == 6u && by == 5u) ||
           (bx == 6u && by == 6u) || (bx == 8u && by == 5u) ||
           (bx == 8u && by == 6u) || (bx == 8u && by == 8u) ||
           (bx == 10u && by == 5u) || (bx == 10u && by == 6u) ||
           (bx == 10u && by == 8u) || (bx == 10u && by == 10u) ||
           (bx == 12u && by == 10u) || (bx == 12u && by == 12u);
}

size_t tc_astc_compressed_size(uint32_t width, uint32_t height,
                               const tc_astc_options *opt) {
    tc_astc_options defopt;
    size_t bx, by, blocks;
    if (!width || !height) return 0;
    if (!opt) {
        tc_astc_options_init(&defopt);
        opt = &defopt;
    }
    if (!tc_astc_valid_block(opt->block_x, opt->block_y)) return 0;
    bx = ((size_t)width + opt->block_x - 1u) / opt->block_x;
    by = ((size_t)height + opt->block_y - 1u) / opt->block_y;
    if (tc_mul_ovf_size(bx, by, &blocks)) return 0;
    if (blocks > SIZE_MAX / 16u) return 0;
    return blocks * 16u;
}

static void tc_wr_u32(uint8_t *p, uint32_t v) {
    p[0] = (uint8_t)v;
    p[1] = (uint8_t)(v >> 8);
    p[2] = (uint8_t)(v >> 16);
    p[3] = (uint8_t)(v >> 24);
}

void tc_wr_u64(uint8_t *p, uint64_t v) {
    p[0] = (uint8_t)v;
    p[1] = (uint8_t)(v >> 8);
    p[2] = (uint8_t)(v >> 16);
    p[3] = (uint8_t)(v >> 24);
    p[4] = (uint8_t)(v >> 32);
    p[5] = (uint8_t)(v >> 40);
    p[6] = (uint8_t)(v >> 48);
    p[7] = (uint8_t)(v >> 56);
}

void tc_wr_u24(uint8_t *p, uint32_t v) {
    p[0] = (uint8_t)v;
    p[1] = (uint8_t)(v >> 8);
    p[2] = (uint8_t)(v >> 16);
}

uint32_t tc_bswap32(uint32_t v) {
    return ((v & 0x000000ffu) << 24) | ((v & 0x0000ff00u) << 8) |
           ((v & 0x00ff0000u) >> 8) | ((v & 0xff000000u) >> 24);
}

uint64_t tc_bswap64(uint64_t v) {
    return ((v & 0x00000000000000ffULL) << 56) |
           ((v & 0x000000000000ff00ULL) << 40) |
           ((v & 0x0000000000ff0000ULL) << 24) |
           ((v & 0x00000000ff000000ULL) << 8) |
           ((v & 0x000000ff00000000ULL) >> 8) |
           ((v & 0x0000ff0000000000ULL) >> 24) |
           ((v & 0x00ff000000000000ULL) >> 40) |
           ((v & 0xff00000000000000ULL) >> 56);
}

int32_t tc_clamp_i32(int32_t v, int32_t lo, int32_t hi) {
    if (v < lo) return lo;
    if (v > hi) return hi;
    return v;
}

uint8_t tc_clamp_u8_i32(int32_t v) {
    return (uint8_t)tc_clamp_i32(v, 0, 255);
}

void tc_set_bits(uint8_t *dst, uint32_t *bitpos, uint32_t val,
                        uint32_t nbits) {
    uint32_t n;
    while (nbits) {
        n = 8u - (*bitpos & 7u);
        if (n > nbits) n = nbits;
        dst[*bitpos >> 3] |= (uint8_t)((val & ((1u << n) - 1u)) << (*bitpos & 7u));
        val >>= n;
        *bitpos += n;
        nbits -= n;
    }
}

uint32_t tc_luma_u8(const uint8_t *p) {
    return (uint32_t)p[0] * 38u + (uint32_t)p[1] * 76u + (uint32_t)p[2] * 14u;
}

size_t tc_dds_bc7_size(uint32_t width, uint32_t height) {
    size_t bc7 = tc_bc7_compressed_size(width, height);
    if (!bc7 || bc7 > SIZE_MAX - 148u) return 0;
    return 148u + bc7;
}

size_t tc_dds_bc1_size(uint32_t width, uint32_t height) {
    size_t bc1 = tc_bc1_compressed_size(width, height);
    if (!bc1 || bc1 > SIZE_MAX - 148u) return 0;
    return 148u + bc1;
}

size_t tc_dds_bc3_size(uint32_t width, uint32_t height) {
    size_t bc3 = tc_bc3_compressed_size(width, height);
    if (!bc3 || bc3 > SIZE_MAX - 148u) return 0;
    return 148u + bc3;
}

size_t tc_dds_bc5_size(uint32_t width, uint32_t height) {
    size_t bc5 = tc_bc5_compressed_size(width, height);
    if (!bc5 || bc5 > SIZE_MAX - 148u) return 0;
    return 148u + bc5;
}

size_t tc_dds_bc6h_size(uint32_t width, uint32_t height) {
    size_t bc6h = tc_bc6h_compressed_size(width, height);
    if (!bc6h || bc6h > SIZE_MAX - 148u) return 0;
    return 148u + bc6h;
}

static tc_result tc_dds_write_bc_memory(const uint8_t *bc, uint32_t width,
                                        uint32_t height, uint32_t dxgi_format,
                                        uint32_t block_bytes, uint8_t *out_dds,
                                        size_t out_size) {
    size_t bc_size, need, pitch;
    if (!bc || !out_dds || !width || !height) return TC_ERROR_INVALID_ARGUMENT;
    bc_size = (size_t)(((size_t)width + 3u) >> 2) *
              (((size_t)height + 3u) >> 2) * block_bytes;
    need = 148u + bc_size;
    if (!need || out_size < need) return TC_ERROR_INVALID_ARGUMENT;
    pitch = (((size_t)width + 3u) >> 2) * block_bytes;

    memset(out_dds, 0, 148);
    memcpy(out_dds, "DDS ", 4);
    tc_wr_u32(out_dds + 4, 124);
    tc_wr_u32(out_dds + 8, 0x0002100Fu);
    tc_wr_u32(out_dds + 12, height);
    tc_wr_u32(out_dds + 16, width);
    tc_wr_u32(out_dds + 20, (uint32_t)pitch);
    tc_wr_u32(out_dds + 28, 1);
    tc_wr_u32(out_dds + 76, 32);
    tc_wr_u32(out_dds + 80, 0x00000004u);
    memcpy(out_dds + 84, "DX10", 4);
    tc_wr_u32(out_dds + 108, 0x00001000u);
    tc_wr_u32(out_dds + 112, dxgi_format);
    tc_wr_u32(out_dds + 116, 3u);
    tc_wr_u32(out_dds + 124, 1u);
    memcpy(out_dds + 148, bc, bc_size);
    return TC_SUCCESS;
}

tc_result tc_dds_write_bc7_memory(const uint8_t *bc7, uint32_t width,
                                  uint32_t height, const tc_bc7_options *opt,
                                  uint8_t *out_dds, size_t out_size) {
    tc_bc7_options defopt;
    if (!opt) {
        tc_bc7_options_init(&defopt);
        opt = &defopt;
    }
    return tc_dds_write_bc_memory(bc7, width, height, 98u + (opt->srgb ? 1u : 0u),
                                  16u, out_dds, out_size);
}

tc_result tc_dds_write_bc1_memory(const uint8_t *bc1, uint32_t width,
                                  uint32_t height, const tc_bc1_options *opt,
                                  uint8_t *out_dds, size_t out_size) {
    tc_bc1_options defopt;
    if (!opt) {
        tc_bc1_options_init(&defopt);
        opt = &defopt;
    }
    return tc_dds_write_bc_memory(bc1, width, height, 71u + (opt->srgb ? 1u : 0u),
                                  8u, out_dds, out_size);
}

tc_result tc_dds_write_bc3_memory(const uint8_t *bc3, uint32_t width,
                                  uint32_t height, const tc_bc3_options *opt,
                                  uint8_t *out_dds, size_t out_size) {
    tc_bc3_options defopt;
    if (!opt) {
        tc_bc3_options_init(&defopt);
        opt = &defopt;
    }
    return tc_dds_write_bc_memory(bc3, width, height, 77u + (opt->srgb ? 1u : 0u),
                                  16u, out_dds, out_size);
}

tc_result tc_dds_write_bc5_memory(const uint8_t *bc5, uint32_t width,
                                  uint32_t height, const tc_bc5_options *opt,
                                  uint8_t *out_dds, size_t out_size) {
    tc_bc5_options defopt;
    if (!opt) {
        tc_bc5_options_init(&defopt);
        opt = &defopt;
    }
    return tc_dds_write_bc_memory(bc5, width, height, opt->snorm ? 84u : 83u,
                                  16u, out_dds, out_size);
}

tc_result tc_dds_write_bc6h_memory(const uint8_t *bc6h, uint32_t width,
                                   uint32_t height,
                                   const tc_bc6h_options *opt,
                                   uint8_t *out_dds, size_t out_size) {
    tc_bc6h_options defopt;
    if (!opt) {
        tc_bc6h_options_init(&defopt);
        opt = &defopt;
    }
    return tc_dds_write_bc_memory(bc6h, width, height, opt->signed_float ? 96u : 95u,
                                  16u, out_dds, out_size);
}

size_t tc_ktx_etc2_size(uint32_t width, uint32_t height,
                        const tc_etc2_options *opt) {
    tc_etc2_options defopt;
    size_t payload;
    if (!opt) {
        tc_etc2_options_init(&defopt);
        opt = &defopt;
    }
    payload = opt->alpha ? tc_etc2_rgba_compressed_size(width, height)
                         : tc_etc2_rgb_compressed_size(width, height);
    if (!payload || payload > SIZE_MAX - 68u) return 0;
    return 68u + payload;
}

static tc_result tc_ktx_write_compressed_memory(const uint8_t *payload_data,
                                                uint32_t width, uint32_t height,
                                                uint32_t internal_format,
                                                uint32_t base_format,
                                                size_t payload,
                                                uint8_t *out_ktx,
                                                size_t out_size) {
    size_t need;
    static const uint8_t ktx_id[12] = {0xab, 0x4b, 0x54, 0x58, 0x20, 0x31,
                                       0x31, 0xbb, 0x0d, 0x0a, 0x1a, 0x0a};
    if (!payload_data || !out_ktx || !width || !height || !payload)
        return TC_ERROR_INVALID_ARGUMENT;
    if (payload > SIZE_MAX - 68u) return TC_ERROR_INVALID_ARGUMENT;
    need = 68u + payload;
    if (out_size < need) return TC_ERROR_INVALID_ARGUMENT;

    memset(out_ktx, 0, need);
    memcpy(out_ktx, ktx_id, sizeof(ktx_id));
    tc_wr_u32(out_ktx + 12, 0x04030201u);
    tc_wr_u32(out_ktx + 20, 1u);
    tc_wr_u32(out_ktx + 28, internal_format);
    tc_wr_u32(out_ktx + 32, base_format);
    tc_wr_u32(out_ktx + 36, width);
    tc_wr_u32(out_ktx + 40, height);
    tc_wr_u32(out_ktx + 52, 1u);
    tc_wr_u32(out_ktx + 56, 1u);
    tc_wr_u32(out_ktx + 64, (uint32_t)payload);
    memcpy(out_ktx + 68, payload_data, payload);
    return TC_SUCCESS;
}

tc_result tc_ktx_write_etc2_memory(const uint8_t *etc2, uint32_t width,
                                   uint32_t height,
                                   const tc_etc2_options *opt,
                                   uint8_t *out_ktx, size_t out_size) {
    tc_etc2_options defopt;
    size_t payload;
    if (!opt) {
        tc_etc2_options_init(&defopt);
        opt = &defopt;
    }
    payload = opt->alpha ? tc_etc2_rgba_compressed_size(width, height)
                         : tc_etc2_rgb_compressed_size(width, height);
    return tc_ktx_write_compressed_memory(
        etc2, width, height,
        opt->alpha ? (opt->srgb ? 0x9279u : 0x9278u)
                   : (opt->srgb ? 0x9275u : 0x9274u),
        opt->alpha ? 0x1908u : 0x1907u, payload, out_ktx, out_size);
}

tc_result tc_ktx_write_eac_memory(const uint8_t *eac, uint32_t width,
                                  uint32_t height, int rg11,
                                  uint8_t *out_ktx, size_t out_size) {
    size_t payload = rg11 ? tc_eac_rg11_compressed_size(width, height)
                          : tc_eac_r11_compressed_size(width, height);
    return tc_ktx_write_compressed_memory(eac, width, height,
                                          rg11 ? 0x9272u : 0x9270u,
                                          rg11 ? 0x8227u : 0x1903u, payload,
                                          out_ktx, out_size);
}
