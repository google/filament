/*
 * TinyEXR - optional libdeflate backend for ZIP/ZIPS/PXR24.
 *
 * Gated by EXR_USE_LIBDEFLATE (off by default). When enabled, ZIP/ZIPS/PXR24
 * route their zlib (DEFLATE) compress/decompress through the vendored libdeflate
 * (deps/libdeflate, MIT-licensed, see deps/libdeflate/COPYING) instead of the
 * in-tree pure-C codec. The in-tree codec remains the default and the only path
 * for freestanding builds; this file compiles to nothing unless the flag is set.
 *
 * libdeflate compression level defaults to 4 (matching OpenEXR's default ZIP
 * level); override with -DEXR_LIBDEFLATE_LEVEL=N (0..12).
 *
 * Copyright (c) 2014-2026 Syoyo Fujita and TinyEXR authors
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * libdeflate itself (deps/libdeflate) is Copyright 2016 Eric Biggers,
 * Copyright 2024 Google LLC, MIT License - see deps/libdeflate/COPYING.
 */

#include "exr_internal.h"

#ifdef EXR_USE_LIBDEFLATE

#include "libdeflate.h"

#ifndef EXR_LIBDEFLATE_LEVEL
#define EXR_LIBDEFLATE_LEVEL 4
#endif

/* Produce a zlib (DEFLATE) stream into a freshly allocated *out_data, matching
 * the contract of exr_deflate_zlib: on success returns the exact length; the
 * caller stores the block raw when the result is not smaller than the input. */
exr_result exr_ld_deflate_zlib(const exr_allocator *a, const uint8_t *src,
                               size_t n, uint8_t **out_data, size_t *out_size) {
    struct libdeflate_compressor *c;
    size_t bound, got;
    uint8_t *out;

    *out_data = NULL;
    *out_size = 0;
    if (n == 0) {
        *out_data = (uint8_t *)exr_malloc(a, 1);
        return *out_data ? EXR_SUCCESS : EXR_ERROR_OUT_OF_MEMORY;
    }

    c = libdeflate_alloc_compressor(EXR_LIBDEFLATE_LEVEL);
    if (!c) return EXR_ERROR_OUT_OF_MEMORY;
    bound = libdeflate_zlib_compress_bound(c, n);
    out = (uint8_t *)exr_malloc(a, bound ? bound : 1);
    if (!out) {
        libdeflate_free_compressor(c);
        return EXR_ERROR_OUT_OF_MEMORY;
    }
    got = libdeflate_zlib_compress(c, src, n, out, bound);
    libdeflate_free_compressor(c);
    if (got == 0) { /* did not fit in bound (should not happen) */
        exr_free(a, out);
        return EXR_ERROR_CORRUPT;
    }
    *out_data = out;
    *out_size = got;
    return EXR_SUCCESS;
}

/* Inflate a zlib stream into dst[0..dst_cap); *out_size gets the decoded length.
 * Mirrors exr_inflate_zlib. */
exr_result exr_ld_inflate_zlib(const uint8_t *src, size_t src_size, uint8_t *dst,
                               size_t dst_cap, size_t *out_size) {
    struct libdeflate_decompressor *d;
    enum libdeflate_result r;
    size_t actual = 0;

    if (out_size) *out_size = 0;
    /* Allocate per block rather than caching a per-thread decompressor. The
     * decompressor is a fixed-size struct whose decode tables are (re)built
     * inside libdeflate_zlib_decompress, so the alloc is a single malloc that
     * is negligible next to the decode: measured identical throughput vs a
     * reused _Thread_local decompressor, both single-threaded and 8-way (the
     * 128-512 KB per-block decode dwarfs the alloc, and glibc's per-thread
     * arenas absorb it). Per-call keeps this trivially thread-safe and leak
     * free under exr_parallel_for's spawn/join workers. */
    d = libdeflate_alloc_decompressor();
    if (!d) return EXR_ERROR_OUT_OF_MEMORY;
    r = libdeflate_zlib_decompress(d, src, src_size, dst, dst_cap, &actual);
    libdeflate_free_decompressor(d);
    if (r != LIBDEFLATE_SUCCESS) return EXR_ERROR_CORRUPT;
    if (out_size) *out_size = actual;
    return EXR_SUCCESS;
}

#else
/* Keep this a non-empty translation unit when the backend is compiled out. */
typedef int exr_libdeflate_translation_unit_not_empty;
#endif /* EXR_USE_LIBDEFLATE */
