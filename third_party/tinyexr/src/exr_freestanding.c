/*
 * TinyEXR - freestanding mem/str implementations.
 *
 * Compiled and linked ONLY for freestanding builds (EXR_FREESTANDING), where no
 * hosted <string.h> is available. These satisfy both our explicit calls and any
 * mem* calls the compiler emits implicitly. MUST be built with -fno-builtin so
 * the byte loops are not turned back into calls to themselves.
 *
 * Copyright (c) 2014-2026 Syoyo Fujita and TinyEXR authors
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "exr_internal.h"

#ifdef EXR_FREESTANDING

/*
 * Weak so a real libc (if one is linked anyway, e.g. when running the
 * freestanding-compiled objects under a hosted test harness) takes precedence
 * and we avoid duplicate-definition errors. In a true no-libc link these are
 * the only definitions and are used as-is.
 */
#if defined(__GNUC__) || defined(__clang__)
#define EXR_FS_WEAK __attribute__((weak))
#else
#define EXR_FS_WEAK
#endif

EXR_FS_WEAK void *memcpy(void *dst, const void *src, size_t n) {
    unsigned char *d = (unsigned char *)dst;
    const unsigned char *s = (const unsigned char *)src;
    while (n--) *d++ = *s++;
    return dst;
}

EXR_FS_WEAK void *memmove(void *dst, const void *src, size_t n) {
    unsigned char *d = (unsigned char *)dst;
    const unsigned char *s = (const unsigned char *)src;
    if (d == s || n == 0) return dst;
    if (d < s) {
        while (n--) *d++ = *s++;
    } else {
        d += n;
        s += n;
        while (n--) *--d = *--s;
    }
    return dst;
}

EXR_FS_WEAK void *memset(void *dst, int c, size_t n) {
    unsigned char *d = (unsigned char *)dst;
    while (n--) *d++ = (unsigned char)c;
    return dst;
}

EXR_FS_WEAK int memcmp(const void *a, const void *b, size_t n) {
    const unsigned char *x = (const unsigned char *)a;
    const unsigned char *y = (const unsigned char *)b;
    while (n--) {
        if (*x != *y) return (int)*x - (int)*y;
        ++x;
        ++y;
    }
    return 0;
}

EXR_FS_WEAK size_t strlen(const char *s) {
    const char *p = s;
    while (*p) ++p;
    return (size_t)(p - s);
}

EXR_FS_WEAK int strcmp(const char *a, const char *b) {
    while (*a && *a == *b) {
        ++a;
        ++b;
    }
    return (int)(unsigned char)*a - (int)(unsigned char)*b;
}

#else
/* Keep the translation unit non-empty for hosted builds (if ever compiled). */
typedef int exr_freestanding_tu_not_empty;
#endif /* EXR_FREESTANDING */
