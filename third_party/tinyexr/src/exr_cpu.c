/*
 * TinyEXR - CPU feature detection + SIMD dispatch table.
 *
 * Runtime CPUID detection (x86) chooses the best available kernel; a scalar
 * implementation is always present as a fallback. NEON is selected at compile
 * time on ARM. The dispatch model follows fpng's CPUID approach.
 *
 * Copyright (c) 2014-2026 Syoyo Fujita and TinyEXR authors
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "exr_internal.h"

#define CAP_F16C (1u << 8)

#if defined(EXR_X86)
#if defined(_MSC_VER)
#include <intrin.h>
static void cpuidex(int out[4], int leaf, int subleaf) {
    __cpuidex(out, leaf, subleaf);
}
static unsigned long long xgetbv0(void) { return _xgetbv(0); }
#else
#include <cpuid.h>
static void cpuidex(int out[4], int leaf, int subleaf) {
    unsigned int a, b, c, d;
    if (!__get_cpuid_count((unsigned int)leaf, (unsigned int)subleaf, &a, &b, &c, &d)) {
        out[0] = out[1] = out[2] = out[3] = 0;
        return;
    }
    out[0] = (int)a; out[1] = (int)b; out[2] = (int)c; out[3] = (int)d;
}
static unsigned long long xgetbv0(void) {
    unsigned int eax, edx;
    __asm__ volatile("xgetbv" : "=a"(eax), "=d"(edx) : "c"(0));
    return ((unsigned long long)edx << 32) | eax;
}
#endif

static uint32_t detect_caps(void) {
    uint32_t caps = 0;
    int r[4];
    int max_leaf;
    int have_avx = 0, have_osxsave = 0;
    cpuidex(r, 0, 0);
    max_leaf = r[0];
    if (max_leaf < 1) return 0;
    cpuidex(r, 1, 0);
    if (r[3] & (1 << 26)) caps |= EXR_SIMD_SSE2;   /* EDX.26 */
    if (r[2] & (1 << 19)) caps |= EXR_SIMD_SSE41;  /* ECX.19 */
    have_osxsave = (r[2] & (1 << 27)) != 0;        /* ECX.27 */
    have_avx = (r[2] & (1 << 28)) != 0;            /* ECX.28 */
    {
        int os_ymm = 0;
        if (have_osxsave) os_ymm = (xgetbv0() & 0x6) == 0x6;
        if (have_avx && os_ymm) {
            if (r[2] & (1 << 29)) caps |= CAP_F16C; /* ECX.29 */
            if (max_leaf >= 7) {
                cpuidex(r, 7, 0);
                if (r[1] & (1 << 5)) caps |= EXR_SIMD_AVX2; /* EBX.5 */
            }
        }
    }
    return caps;
}
#endif /* EXR_X86 */

static uint32_t cpu_caps_cached(void) {
    static int ready = 0;
    static uint32_t caps = 0;
    if (!ready) {
#if defined(EXR_X86)
        caps = detect_caps();
#elif defined(EXR_NEON)
        caps = EXR_SIMD_NEON;
#else
        caps = 0;
#endif
        ready = 1;
    }
    return caps;
}

/* Benchmark-only forced tier: -1 = use real detection (default). */
static int g_caps_force = -1;

uint32_t exr_cpu_caps(void) {
    uint32_t real = cpu_caps_cached();
    if (g_caps_force < 0) return real;
    if (g_caps_force == 0) return 0; /* scalar */
    {
        uint32_t c = real & (EXR_SIMD_SSE2 | EXR_SIMD_SSE41);
        if (g_caps_force >= 2) c |= real & (EXR_SIMD_AVX2 | CAP_F16C);
        return c;
    }
}

void exr_cpu_caps_force(int level) { g_caps_force = level; }

/* The dispatch table (scalar by default; init upgrades it). */
exr_simd_vtbl exr_simd = {0};

/* Util-module kernels: scalar defaults, then optional SIMD upgrade. Shared by
 * exr_simd_init() and exr_simd_force() so parity tests can pin a tier. */
static void util_set_scalar(void) {
    exr_simd.u8_to_f32 = exr_u8_to_f32_scalar;
    exr_simd.u16_to_f32 = exr_u16_to_f32_scalar;
    exr_simd.f32_to_u8 = exr_f32_to_u8_scalar;
    exr_simd.f32_to_u16 = exr_f32_to_u16_scalar;
    exr_simd.axpy = exr_axpy_scalar;
    exr_simd.mat3 = exr_mat3_scalar;
}

static void util_set_simd(int level) {
    (void)level;
#if defined(EXR_X86)
    {
        uint32_t caps = cpu_caps_cached();
        if (level >= 1 && (caps & EXR_SIMD_SSE41)) {
            exr_simd.u8_to_f32 = exr_u8_to_f32_sse41;
            exr_simd.u16_to_f32 = exr_u16_to_f32_sse41;
            exr_simd.f32_to_u8 = exr_f32_to_u8_sse41;
            exr_simd.f32_to_u16 = exr_f32_to_u16_sse41;
        }
        if (level >= 2 && (caps & EXR_SIMD_AVX2)) {
            exr_simd.axpy = exr_axpy_avx2;
            exr_simd.mat3 = exr_mat3_avx2;
        }
    }
#elif defined(EXR_NEON)
    if (level >= 1) {
        exr_simd.u8_to_f32 = exr_u8_to_f32_neon;
        exr_simd.u16_to_f32 = exr_u16_to_f32_neon;
        exr_simd.f32_to_u8 = exr_f32_to_u8_neon;
        exr_simd.f32_to_u16 = exr_f32_to_u16_neon;
        exr_simd.axpy = exr_axpy_neon;
        exr_simd.mat3 = exr_mat3_neon;
    }
#endif
}

void exr_simd_init(void) {
    static int done = 0;
    uint32_t caps;
    if (done) return;
    caps = cpu_caps_cached();
    (void)caps; /* unused on non-x86 (NEON/scalar set the table directly) */

    exr_simd.half_to_float = exr_half_to_float_scalar;
    exr_simd.float_to_half = exr_float_to_half_scalar;
    exr_simd.interleave = exr_interleave_scalar;
    exr_simd.predictor_decode = exr_predictor_decode_scalar;
    exr_simd.predictor_encode = exr_predictor_encode_scalar;
    util_set_scalar();

#if defined(EXR_X86)
    if (caps & EXR_SIMD_SSE2) {
        exr_simd.interleave = exr_interleave_sse2;
        exr_simd.predictor_decode = exr_predictor_decode_sse2;
        exr_simd.predictor_encode = exr_predictor_encode_sse2;
    }
    if (caps & EXR_SIMD_AVX2) exr_simd.interleave = exr_interleave_avx2;
    if (caps & CAP_F16C) {
        exr_simd.half_to_float = exr_half_to_float_f16c;
        exr_simd.float_to_half = exr_float_to_half_f16c;
    }
    util_set_simd(2);
#endif
#if defined(EXR_NEON)
    exr_simd.half_to_float = exr_half_to_float_neon;
    exr_simd.float_to_half = exr_float_to_half_neon;
    exr_simd.interleave = exr_interleave_neon;
    exr_simd.predictor_decode = exr_predictor_decode_neon;
    exr_simd.predictor_encode = exr_predictor_encode_neon;
    util_set_simd(1);
#endif
    done = 1;
}

void exr_simd_force(int level) {
    exr_simd.half_to_float = exr_half_to_float_scalar;
    exr_simd.float_to_half = exr_float_to_half_scalar;
    exr_simd.interleave = exr_interleave_scalar;
    exr_simd.predictor_decode = exr_predictor_decode_scalar;
    exr_simd.predictor_encode = exr_predictor_encode_scalar;
    util_set_scalar();
    util_set_simd(level);
#if defined(EXR_X86)
    {
        uint32_t caps = cpu_caps_cached();
        if (level >= 1 && (caps & EXR_SIMD_SSE2)) {
            exr_simd.interleave = exr_interleave_sse2;
            exr_simd.predictor_decode = exr_predictor_decode_sse2;
            exr_simd.predictor_encode = exr_predictor_encode_sse2;
        }
        if (level >= 2 && (caps & EXR_SIMD_AVX2))
            exr_simd.interleave = exr_interleave_avx2;
        if (level >= 2 && (caps & CAP_F16C)) {
            exr_simd.half_to_float = exr_half_to_float_f16c;
            exr_simd.float_to_half = exr_float_to_half_f16c;
        }
    }
#elif defined(EXR_NEON)
    if (level >= 1) {
        exr_simd.half_to_float = exr_half_to_float_neon;
        exr_simd.float_to_half = exr_float_to_half_neon;
        exr_simd.interleave = exr_interleave_neon;
        exr_simd.predictor_decode = exr_predictor_decode_neon;
        exr_simd.predictor_encode = exr_predictor_encode_neon;
    }
#else
    (void)level;
#endif
}

uint32_t exr_simd_capabilities(void) {
    return cpu_caps_cached() &
           (EXR_SIMD_SSE2 | EXR_SIMD_SSE41 | EXR_SIMD_AVX2 | EXR_SIMD_NEON);
}

const char *exr_simd_info(void) {
    uint32_t c = cpu_caps_cached();
#if defined(EXR_NEON)
    (void)c;
    return "neon";
#else
    if (c & EXR_SIMD_AVX2) return (c & CAP_F16C) ? "avx2+f16c" : "avx2";
    if (c & EXR_SIMD_SSE41) return "sse4.1";
    if (c & EXR_SIMD_SSE2) return "sse2";
    return "scalar";
#endif
}
