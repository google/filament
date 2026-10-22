/*
 * TinyEXR - RLE codec (OpenEXR run-length encoding) + predictor + interleave.
 *
 * Wire format (signed byte count):
 *   count < 0 : -count literal bytes follow.
 *   count >= 0: the next byte is repeated count + 1 times.
 * After RLE decode, EXR applies the same predictor + interleave as ZIP.
 *
 * Copyright (c) 2014-2026 Syoyo Fujita and TinyEXR authors
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "exr_internal.h"

/* RLE-expand into dst (the pre-reconstruction buffer); no predictor/interleave. */
exr_result exr_rle_expand_only(const uint8_t *src, size_t src_size,
                               uint8_t *dst, size_t dst_size) {
    const signed char *in = (const signed char *)src;
    const signed char *in_end = in + src_size;
    uint8_t *out = dst;
    uint8_t *out_end = dst + dst_size;

    while (in < in_end && out < out_end) {
        int count = *in++;
        if (count < 0) {
            size_t len = (size_t)(-count);
            if ((size_t)(in_end - in) < len || (size_t)(out_end - out) < len)
                return EXR_ERROR_CORRUPT;
            memcpy(out, in, len);
            out += len;
            in += len;
        } else {
            size_t len = (size_t)count + 1;
            uint8_t val;
            if (in >= in_end || (size_t)(out_end - out) < len)
                return EXR_ERROR_CORRUPT;
            val = (uint8_t)*in++;
            memset(out, val, len);
            out += len;
        }
    }
    if ((size_t)(out - dst) != dst_size) return EXR_ERROR_CORRUPT;
    return EXR_SUCCESS;
}

exr_result exr_rle_decompress(const exr_allocator *a, const uint8_t *src,
                              size_t src_size, uint8_t *dst, size_t dst_size) {
    uint8_t *tmp;
    exr_result rc;

    tmp = (uint8_t *)exr_malloc(a, dst_size ? dst_size : 1);
    if (!tmp) return EXR_ERROR_OUT_OF_MEMORY;

    rc = exr_rle_expand_only(src, src_size, tmp, dst_size);
    if (EXR_OK(rc)) {
        exr_predictor_decode(tmp, dst_size);
        exr_interleave_decode(tmp, dst, dst_size);
    }
    exr_free(a, tmp);
    return rc;
}

/* OpenEXR RLE encode of an already split+predicted buffer. */
#define RLE_MIN_RUN 3
#define RLE_MAX_RUN 127
static size_t rle_encode(const uint8_t *in, size_t n, uint8_t *out) {
    const uint8_t *in_end = in + n;
    const uint8_t *run_start = in, *run_end = in + 1;
    uint8_t *w = out;
    while (run_start < in_end) {
        while (run_end < in_end && *run_start == *run_end &&
               (size_t)(run_end - run_start) - 1 < RLE_MAX_RUN)
            ++run_end;
        if (run_end - run_start >= RLE_MIN_RUN) {
            *w++ = (uint8_t)(signed char)((run_end - run_start) - 1);
            *w++ = *run_start;
            run_start = run_end;
        } else {
            while (run_end < in_end &&
                   ((run_end + 1 >= in_end || *run_end != *(run_end + 1)) ||
                    (run_end + 2 >= in_end || *(run_end + 1) != *(run_end + 2))) &&
                   (size_t)(run_end - run_start) < RLE_MAX_RUN)
                ++run_end;
            *w++ = (uint8_t)(signed char)(run_start - run_end);
            while (run_start < run_end) *w++ = *run_start++;
        }
        ++run_end;
    }
    return (size_t)(w - out);
}

exr_result exr_rle_compress(const exr_allocator *a, const uint8_t *src, size_t n,
                            uint8_t **out_data, size_t *out_size) {
    uint8_t *tmp, *rle;
    size_t rlen, cap;
    *out_data = NULL;
    *out_size = 0;
    if (n == 0) {
        *out_data = (uint8_t *)exr_malloc(a, 1);
        if (!*out_data) return EXR_ERROR_OUT_OF_MEMORY;
        return EXR_SUCCESS;
    }
    tmp = (uint8_t *)exr_malloc(a, n);
    cap = n + n / RLE_MAX_RUN + 16;
    rle = (uint8_t *)exr_malloc(a, cap);
    if (!tmp || !rle) {
        exr_free(a, tmp);
        exr_free(a, rle);
        return EXR_ERROR_OUT_OF_MEMORY;
    }
    exr_interleave_encode(src, tmp, n);
    exr_predictor_encode(tmp, n);
    rlen = rle_encode(tmp, n, rle);
    exr_free(a, tmp);

    if (rlen >= n) { /* did not help: store the original canonical block raw */
        exr_free(a, rle);
        *out_data = (uint8_t *)exr_malloc(a, n);
        if (!*out_data) return EXR_ERROR_OUT_OF_MEMORY;
        memcpy(*out_data, src, n);
        *out_size = n;
        return EXR_SUCCESS;
    }
    *out_data = rle;
    *out_size = rlen;
    return EXR_SUCCESS;
}
