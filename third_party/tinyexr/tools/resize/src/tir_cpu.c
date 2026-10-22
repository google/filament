/*
 * tir - CPU feature detection + kernel dispatch table.
 *
 * Runtime CPUID detection (x86) picks the best kernel set; the scalar set is
 * always present. NEON is selected at compile time on AArch64; SVE kernels
 * are compiled into their own unit and enabled only when getauxval reports
 * HWCAP_SVE (Apple Silicon has no non-streaming SVE: NEON is the path there).
 *
 * Copyright (c) 2014-2026 Syoyo Fujita and TinyEXR authors
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "tir_internal.h"

#if defined(TIR_X86)
#if defined(_MSC_VER)
#include <intrin.h>
static void cpuidex_(int out[4], int leaf, int subleaf) {
    __cpuidex(out, leaf, subleaf);
}
static unsigned long long xgetbv0_(void) { return _xgetbv(0); }
#else
#include <cpuid.h>
static void cpuidex_(int out[4], int leaf, int subleaf) {
    unsigned int a, b, c, d;
    if (!__get_cpuid_count((unsigned int)leaf, (unsigned int)subleaf, &a, &b,
                           &c, &d)) {
        out[0] = out[1] = out[2] = out[3] = 0;
        return;
    }
    out[0] = (int)a;
    out[1] = (int)b;
    out[2] = (int)c;
    out[3] = (int)d;
}
static unsigned long long xgetbv0_(void) {
    unsigned int eax, edx;
    __asm__ volatile("xgetbv" : "=a"(eax), "=d"(edx) : "c"(0));
    return ((unsigned long long)edx << 32) | eax;
}
#endif

static uint32_t detect_x86(void) {
    uint32_t caps = 1u << TIR_SIMD_SCALAR;
    int r[4];
    int max_leaf;
    cpuidex_(r, 0, 0);
    max_leaf = r[0];
    if (max_leaf < 1) return caps;
    cpuidex_(r, 1, 0);
    if (r[3] & (1 << 26)) caps |= 1u << TIR_SIMD_SSE2;  /* EDX.26 */
    if (r[2] & (1 << 19)) caps |= 1u << TIR_SIMD_SSE41; /* ECX.19 */
    {
        int have_osxsave = (r[2] & (1 << 27)) != 0;
        int have_avx = (r[2] & (1 << 28)) != 0;
        int have_f16c = (r[2] & (1 << 29)) != 0;
        int have_fma = (r[2] & (1 << 12)) != 0;
        int os_ymm = 0;
        if (have_osxsave) os_ymm = (xgetbv0_() & 0x6) == 0x6;
        if (have_avx && os_ymm && have_f16c && have_fma && max_leaf >= 7) {
            cpuidex_(r, 7, 0);
            if (r[1] & (1 << 5)) caps |= 1u << TIR_SIMD_AVX2; /* EBX.5 */
        }
    }
    return caps;
}
#endif /* TIR_X86 */

#if defined(TIR_NEON)
#if defined(__linux__)
#include <sys/auxv.h>
#ifndef HWCAP_SVE
#define HWCAP_SVE (1 << 22)
#endif
#endif
/* SVE requires both compiled-in kernels and kernel-reported support. There
 * is no non-streaming SVE on Apple Silicon, so NEON is the macOS path. */
static int sve_usable(void) {
    if (!tir__have_sve_kernels) return 0;
#if defined(__linux__)
    return (getauxval(AT_HWCAP) & HWCAP_SVE) != 0;
#else
    return 0;
#endif
}
#endif /* TIR_NEON */

static uint32_t caps_cached(void) {
    static int ready = 0;
    static uint32_t caps = 0;
    if (!ready) {
#if defined(TIR_X86)
        caps = detect_x86();
#elif defined(TIR_NEON)
        caps = (1u << TIR_SIMD_SCALAR) | (1u << TIR_SIMD_NEON);
        if (sve_usable()) caps |= 1u << TIR_SIMD_SVE;
#else
        caps = 1u << TIR_SIMD_SCALAR;
#endif
        ready = 1;
    }
    return caps;
}

uint32_t tir_simd_available(void) { return caps_cached(); }

/* Active table + the level it was built for (for tir_simd_info). */
tir_kernels tir__k;
static int g_level = -1; /* -1 = not initialized */

static void build_table(int level) {
    tir__kernels_set_scalar(&tir__k);
#if defined(TIR_X86)
    {
        uint32_t caps = caps_cached();
        if (level >= TIR_SIMD_SSE2 && (caps & (1u << TIR_SIMD_SSE2)))
            tir__kernels_set_sse2(&tir__k);
        if (level >= TIR_SIMD_SSE41 && (caps & (1u << TIR_SIMD_SSE41)))
            tir__kernels_set_sse41(&tir__k);
        if (level >= TIR_SIMD_AVX2 && (caps & (1u << TIR_SIMD_AVX2)))
            tir__kernels_set_avx2(&tir__k);
    }
#elif defined(TIR_NEON)
    {
        uint32_t caps = caps_cached();
        if (level >= TIR_SIMD_NEON) tir__kernels_set_neon(&tir__k);
        if (level >= TIR_SIMD_SVE && (caps & (1u << TIR_SIMD_SVE)))
            tir__kernels_set_sve(&tir__k);
    }
#endif
    g_level = level;
}

static int best_level(void) {
    uint32_t caps = caps_cached();
    int l;
    for (l = TIR_SIMD_SVE; l > 0; --l)
        if (caps & (1u << l)) return l;
    return TIR_SIMD_SCALAR;
}

void tir__kernels_init(void) {
    if (g_level < 0) build_table(best_level());
}

tir_result tir_simd_force(tir_simd_level level) {
    if ((int)level < 0 || level > TIR_SIMD_SVE)
        return TIR_ERROR_INVALID_ARGUMENT;
    if (!(caps_cached() & (1u << level))) return TIR_ERROR_UNSUPPORTED;
    build_table((int)level);
    return TIR_SUCCESS;
}

const char *tir_simd_info(void) {
    static const char *names[] = {"scalar", "sse2",  "sse4.1",
                                  "avx2",   "neon",  "sve"};
    int l = g_level < 0 ? best_level() : g_level;
    return names[l];
}
