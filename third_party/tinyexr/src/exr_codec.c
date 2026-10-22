/*
 * TinyEXR - codec dispatch + shared post-processing (predictor / interleave).
 *
 * Copyright (c) 2014-2026 Syoyo Fujita and TinyEXR authors
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "exr_internal.h"

/* ---- runtime zlib backend selection (ZIP/ZIPS/PXR24) ---------------------
 * Statically initialised to the build default: libdeflate when it is compiled
 * in and chosen as the default (DEFLATE=auto|libdeflate), otherwise the in-tree
 * pure-C codec. exr_zlib_set_backend() overrides at runtime for tests/bench. */
#if defined(EXR_USE_LIBDEFLATE) && EXR_ZLIB_DEFAULT_LIBDEFLATE
_Atomic exr_zlib_inflate_fn exr_zlib_inflate = exr_ld_inflate_zlib;
_Atomic exr_zlib_deflate_fn exr_zlib_deflate = exr_ld_deflate_zlib;
#else
_Atomic exr_zlib_inflate_fn exr_zlib_inflate = exr_inflate_zlib;
_Atomic exr_zlib_deflate_fn exr_zlib_deflate = exr_deflate_zlib;
#endif

exr_result exr_zlib_set_backend(exr_zlib_backend backend) {
#if defined(EXR_USE_LIBDEFLATE)
    switch (backend) {
    case EXR_ZLIB_INTREE:
        exr_zlib_inflate = exr_inflate_zlib;
        exr_zlib_deflate = exr_deflate_zlib;
        return EXR_SUCCESS;
    case EXR_ZLIB_LIBDEFLATE:
        exr_zlib_inflate = exr_ld_inflate_zlib;
        exr_zlib_deflate = exr_ld_deflate_zlib;
        return EXR_SUCCESS;
    case EXR_ZLIB_AUTO:
    default:
#if EXR_ZLIB_DEFAULT_LIBDEFLATE
        exr_zlib_inflate = exr_ld_inflate_zlib;
        exr_zlib_deflate = exr_ld_deflate_zlib;
#else
        exr_zlib_inflate = exr_inflate_zlib;
        exr_zlib_deflate = exr_deflate_zlib;
#endif
        return EXR_SUCCESS;
    }
#else
    /* Only the in-tree codec is linked; libdeflate cannot be selected. */
    if (backend == EXR_ZLIB_LIBDEFLATE) return EXR_ERROR_UNSUPPORTED;
    exr_zlib_inflate = exr_inflate_zlib;
    exr_zlib_deflate = exr_deflate_zlib;
    return EXR_SUCCESS;
#endif
}

const char *exr_zlib_backend_name(void) {
#if defined(EXR_USE_LIBDEFLATE)
    return exr_zlib_inflate == exr_ld_inflate_zlib ? "libdeflate" : "in-tree";
#else
    return "in-tree";
#endif
}

/*
 * Decompress one chunk's compressed payload into the canonical uncompressed
 * block layout (per scanline, then per channel: sample data).
 *
 * Per OpenEXR convention, when the compressed payload is the same size as the
 * uncompressed block, the data was stored verbatim (compression did not help),
 * so all codecs fall through to a plain copy in that case.
 */
exr_result exr_decompress_block(const exr_codec_ctx *ctx, const uint8_t *src,
                                size_t src_size, uint8_t *dst, size_t dst_size) {
    if (src_size == dst_size) {
        memcpy(dst, src, dst_size);
        return EXR_SUCCESS;
    }
    if (src_size > dst_size &&
        ctx->compression != EXR_COMPRESSION_HTJ2K256 &&
        ctx->compression != EXR_COMPRESSION_HTJ2K32)
        return EXR_ERROR_CORRUPT;

    switch (ctx->compression) {
    case EXR_COMPRESSION_NONE:
        /* NONE never compresses; size mismatch means corruption. */
        return EXR_ERROR_CORRUPT;
    case EXR_COMPRESSION_RLE:
        return exr_rle_decompress(ctx->alloc, src, src_size, dst, dst_size);
    case EXR_COMPRESSION_ZIP:
    case EXR_COMPRESSION_ZIPS:
        return exr_zip_decompress(ctx->alloc, src, src_size, dst, dst_size);
    case EXR_COMPRESSION_PXR24:
        return exr_pxr24_decompress(ctx, src, src_size, dst, dst_size);
    case EXR_COMPRESSION_PIZ:
        return exr_piz_decompress(ctx, src, src_size, dst, dst_size);
    case EXR_COMPRESSION_B44:
        return exr_b44_decompress(ctx, src, src_size, dst, dst_size, 0);
    case EXR_COMPRESSION_B44A:
        return exr_b44_decompress(ctx, src, src_size, dst, dst_size, 1);
    case EXR_COMPRESSION_ZSTD:
#ifdef EXR_NO_ZSTD
        return EXR_ERROR_UNSUPPORTED;
#else
        return exr_zstd_decompress(ctx->alloc, src, src_size, dst, dst_size);
#endif
    case EXR_COMPRESSION_HTJ2K256:
    case EXR_COMPRESSION_HTJ2K32:
#ifdef EXR_NO_JPH
        return EXR_ERROR_UNSUPPORTED;
#else
        return exr_jph_decompress(ctx, src, src_size, dst, dst_size);
#endif
    case EXR_COMPRESSION_DWAA:
    case EXR_COMPRESSION_DWAB:
        return EXR_ERROR_UNSUPPORTED;
    }
    return EXR_ERROR_UNSUPPORTED;
}

/* Encode dispatch: compress one canonical block per ctx->compression.
 * Allocates *out_data (caller frees); NONE/store-raw yields *out_size == n. */
exr_result exr_compress_block(const exr_codec_ctx *ctx, const uint8_t *block,
                              size_t n, uint8_t **out_data, size_t *out_size) {
    switch (ctx->compression) {
    case EXR_COMPRESSION_NONE: {
        *out_data = (uint8_t *)exr_malloc(ctx->alloc, n ? n : 1);
        if (!*out_data) return EXR_ERROR_OUT_OF_MEMORY;
        memcpy(*out_data, block, n);
        *out_size = n;
        return EXR_SUCCESS;
    }
    case EXR_COMPRESSION_RLE:
        return exr_rle_compress(ctx->alloc, block, n, out_data, out_size);
    case EXR_COMPRESSION_ZIP:
    case EXR_COMPRESSION_ZIPS:
        return exr_zip_compress(ctx->alloc, block, n, out_data, out_size);
    case EXR_COMPRESSION_PIZ:
        return exr_piz_compress(ctx, block, n, out_data, out_size);
    case EXR_COMPRESSION_PXR24:
        return exr_pxr24_compress(ctx, block, n, out_data, out_size);
    case EXR_COMPRESSION_B44:
        return exr_b44_compress(ctx, block, n, out_data, out_size, 0);
    case EXR_COMPRESSION_B44A:
        return exr_b44_compress(ctx, block, n, out_data, out_size, 1);
    case EXR_COMPRESSION_ZSTD:
#ifdef EXR_NO_ZSTD
        return EXR_ERROR_UNSUPPORTED;
#else
        return exr_zstd_compress(ctx->alloc, block, n, out_data, out_size);
#endif
    case EXR_COMPRESSION_HTJ2K256:
    case EXR_COMPRESSION_HTJ2K32:
#ifdef EXR_NO_JPH
        return EXR_ERROR_UNSUPPORTED;
#else
        return exr_jph_compress(ctx, block, n, out_data, out_size);
#endif
    default:
        return EXR_ERROR_UNSUPPORTED;
    }
}

/* ----------------------------------------------------------------------------
 * EXR post-DEFLATE reconstruction passes (shared by ZIP/ZIPS/RLE).
 * These match OpenEXR's ImfZipCompressor reconstruction exactly:
 *   predictor: each byte holds (delta + 128) vs the previous byte;
 *   interleave: source is the even-byte half followed by the odd-byte half.
 * ------------------------------------------------------------------------- */

void exr_predictor_decode_scalar(uint8_t *p, size_t n) {
    size_t i;
    for (i = 1; i < n; ++i) {
        int d = (int)p[i - 1] + (int)p[i] - 128;
        p[i] = (uint8_t)d;
    }
}

void exr_predictor_decode(uint8_t *p, size_t n) {
    exr_simd_init();
    exr_simd.predictor_decode(p, n);
}

void exr_interleave_decode(const uint8_t *src, uint8_t *dst, size_t n) {
    exr_simd_init();
    exr_simd.interleave(src, dst, n);
}

/* Forward byte split: even-position bytes into the first half, odd into the
 * second. Inverse of exr_interleave_decode. */
void exr_interleave_encode(const uint8_t *src, uint8_t *dst, size_t n) {
    const uint8_t *s = src, *stop = src + n;
    uint8_t *t1 = dst, *t2 = dst + (n + 1) / 2;
    while (s < stop) {
        *t1++ = *s++;
        if (s < stop) *t2++ = *s++;
    }
}

/* Forward delta predictor: store (cur - prev + 128) per byte. Inverse of
 * exr_predictor_decode. */
void exr_predictor_encode_scalar(uint8_t *p, size_t n) {
    size_t i;
    int prev;
    if (n == 0) return;
    prev = p[0];
    for (i = 1; i < n; ++i) {
        int cur = p[i];
        p[i] = (uint8_t)(cur - prev + (128 + 256));
        prev = cur;
    }
}

void exr_predictor_encode(uint8_t *p, size_t n) {
    exr_simd_init();
    exr_simd.predictor_encode(p, n);
}
