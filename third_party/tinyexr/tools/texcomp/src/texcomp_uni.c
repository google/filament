/*
 * TinyEXR texcomp - "uni": compact universal transcodable texture.
 *
 * Each 4x4 block stores a full UASTC LDR block (16 bytes, any of the 19
 * UASTC modes). Encode once, then transcode cheaply at load to a
 * frequently-used GPU block format:
 *   - ASTC (4x4): byte-copy (the stored blocks ARE valid ASTC blocks).
 *   - BC7, BC1, ETC2: decode->re-encode (UASTC decode + target encode).
 *
 * Copyright (c) 2014-2026 Syoyo Fujita and TinyEXR authors
 * SPDX-License-Identifier: Apache-2.0
 */
#include "texcomp.h"
#include "texcomp_internal.h"

#include <stdlib.h>
#include <string.h>

/* UASTC block size: 16 bytes/block, same as the old flat format. */
#define TC_UNI_BLOCK_BYTES 16u

size_t tc_uni_compressed_size(uint32_t width, uint32_t height) {
    size_t bx = ((size_t)width + 3u) >> 2, by = ((size_t)height + 3u) >> 2;
    return bx * by * TC_UNI_BLOCK_BYTES;
}

/* ---- encode: route through the public UASTC encoder ---- */
tc_result tc_uni_compress_rgba8(const uint8_t *rgba, uint32_t width,
                                uint32_t height, size_t stride, uint8_t *out,
                                size_t out_size) {
    tc_astc_options ao;
    size_t need = tc_uni_compressed_size(width, height);
    if (!rgba || !out || !width || !height) return TC_ERROR_INVALID_ARGUMENT;
    if (stride < (size_t)width * 4u || out_size < need) return TC_ERROR_INVALID_ARGUMENT;
    tc_astc_options_init(&ao);
    ao.block_x = 4u;
    ao.block_y = 4u;
    ao.uastc = 1;
    return tc_astc_compress_rgba8(rgba, width, height, stride, &ao, out, out_size);
}

/* ---- decode: UASTC block decode ---- */
tc_result tc_uni_decompress_rgba8(const uint8_t *uni, uint32_t width,
                                  uint32_t height, size_t stride, uint8_t *out,
                                  size_t out_size) {
    uint32_t bx, by, xx;
    size_t off = 0;
    if (!uni || !out || !width || !height) return TC_ERROR_INVALID_ARGUMENT;
    if (stride < (size_t)width * 4u || out_size < (size_t)height * stride)
        return TC_ERROR_INVALID_ARGUMENT;
    for (by = 0; by < height; by += 4) {
        for (bx = 0; bx < width; bx += 4) {
            uint8_t tmp[16 * 4];
            uint32_t py, i;
            const uint8_t *b = uni + off;
            if (!tc_astc_decode_block_rgba8(b, 4, 4, tmp)) return TC_ERROR_CORRUPT;
            for (py = 0; py < 4; ++py) {
                uint32_t y = by + py;
                if (y >= height) continue;
                for (xx = 0; xx < 4; ++xx) {
                    uint32_t x = bx + xx;
                    if (x >= width) continue;
                    i = py * 4 + xx;
                    memcpy(out + (size_t)y * stride + (size_t)x * 4u, tmp + i * 4u, 4);
                }
            }
            off += TC_UNI_BLOCK_BYTES;
        }
    }
    return TC_SUCCESS;
}

/* ---- transcode: ASTC 4x4 == byte copy (the stored blocks ARE ASTC) ---- */
tc_result tc_uni_transcode_astc(const uint8_t *uni, uint32_t width,
                                uint32_t height, uint8_t *out, size_t out_size) {
    size_t sz = tc_uni_compressed_size(width, height);
    if (!uni || !out || out_size < sz) return TC_ERROR_INVALID_ARGUMENT;
    memcpy(out, uni, sz);
    return TC_SUCCESS;
}

/* ---- transcode: BC7, BC1, ETC2 == decode UASTC -> target encode ---- */

tc_result tc_uni_transcode_bc7(const uint8_t *uni, uint32_t width,
                               uint32_t height, uint8_t *out, size_t out_size) {
    uint8_t *rgba;
    tc_bc7_options bo;
    tc_result r;
    size_t n = (size_t)width * height * 4u;
    if (!uni || !out) return TC_ERROR_INVALID_ARGUMENT;
    rgba = (uint8_t *)malloc(n);
    if (!rgba) return TC_ERROR_OUT_OF_MEMORY;
    r = tc_uni_decompress_rgba8(uni, width, height, (size_t)width * 4u, rgba, n);
    if (r == TC_SUCCESS) {
        tc_bc7_options_init(&bo);
        r = tc_bc7_compress_rgba8(rgba, width, height, (size_t)width * 4u, &bo,
                                  out, out_size);
    }
    free(rgba);
    return r;
}

tc_result tc_uni_transcode_bc1(const uint8_t *uni, uint32_t width,
                               uint32_t height, uint8_t *out, size_t out_size) {
    uint8_t *rgba;
    tc_bc1_options b1o;
    tc_result r;
    size_t n = (size_t)width * height * 4u;
    if (!uni || !out) return TC_ERROR_INVALID_ARGUMENT;
    rgba = (uint8_t *)malloc(n);
    if (!rgba) return TC_ERROR_OUT_OF_MEMORY;
    r = tc_uni_decompress_rgba8(uni, width, height, (size_t)width * 4u, rgba, n);
    if (r == TC_SUCCESS) {
        tc_bc1_options_init(&b1o);
        r = tc_bc1_compress_rgba8(rgba, width, height, (size_t)width * 4u, &b1o,
                                  out, out_size);
    }
    free(rgba);
    return r;
}

tc_result tc_uni_transcode_etc2(const uint8_t *uni, uint32_t width,
                                uint32_t height, int alpha, uint8_t *out,
                                size_t out_size) {
    uint8_t *rgba;
    tc_etc2_options eo;
    tc_result r;
    size_t n = (size_t)width * height * 4u;
    if (!uni || !out) return TC_ERROR_INVALID_ARGUMENT;
    rgba = (uint8_t *)malloc(n);
    if (!rgba) return TC_ERROR_OUT_OF_MEMORY;
    r = tc_uni_decompress_rgba8(uni, width, height, (size_t)width * 4u, rgba, n);
    if (r == TC_SUCCESS) {
        tc_etc2_options_init(&eo);
        eo.alpha = alpha ? 1 : 0;
        r = tc_etc2_compress_rgba8(rgba, width, height, (size_t)width * 4u, &eo,
                                   out, out_size);
    }
    free(rgba);
    return r;
}
