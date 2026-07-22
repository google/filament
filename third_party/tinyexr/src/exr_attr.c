/*
 * TinyEXR - attribute parsing/serialization.
 *
 * Copyright (c) 2014-2026 Syoyo Fujita and TinyEXR authors
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "exr_internal.h"

/* Parse a "chlist" attribute value into a channel array (sorted as stored). */
exr_result exr_parse_chlist(const exr_allocator *a, const uint8_t *data,
                            uint32_t size, exr_channel **out_channels,
                            int32_t *out_count) {
    exr_channel *chans = NULL;
    int32_t count = 0, cap = 0;
    uint32_t pos = 0;

    *out_channels = NULL;
    *out_count = 0;

    while (pos < size) {
        uint32_t name_start, name_len;
        const exr_channel zero = {{0}, EXR_PIXEL_HALF, 1, 1, 0};
        exr_channel ch;
        int32_t pt;

        /* terminating empty name */
        if (data[pos] == 0) {
            pos++;
            break;
        }

        /* channel name (NUL-terminated) */
        name_start = pos;
        while (pos < size && data[pos] != 0) pos++;
        if (pos >= size) goto corrupt; /* unterminated */
        name_len = pos - name_start;
        pos++; /* skip NUL */
        if (name_len >= EXR_MAX_NAME) goto corrupt;

        /* fixed 16-byte tail: type(4) pLinear(1) reserved(3) xSamp(4) ySamp(4) */
        if (pos + 16 > size) goto corrupt;

        ch = zero;
        memcpy(ch.name, data + name_start, name_len);
        ch.name[name_len] = '\0';
        pt = exr_rd_i32(data + pos);
        if (pt < 0 || pt > 2) goto corrupt;
        ch.pixel_type = (exr_pixel_type)pt;
        ch.p_linear = data[pos + 4];
        ch.x_sampling = exr_rd_i32(data + pos + 8);
        ch.y_sampling = exr_rd_i32(data + pos + 12);
        if (ch.x_sampling <= 0 || ch.y_sampling <= 0) goto corrupt;
        pos += 16;

        if (count >= EXR_MAX_CHANNELS) goto corrupt;
        if (count == cap) {
            int32_t ncap = cap ? cap * 2 : 4;
            exr_channel *nc = (exr_channel *)exr_calloc(a, (size_t)ncap,
                                                        sizeof(exr_channel));
            if (!nc) {
                exr_free(a, chans);
                return EXR_ERROR_OUT_OF_MEMORY;
            }
            if (chans) {
                memcpy(nc, chans, (size_t)count * sizeof(exr_channel));
                exr_free(a, chans);
            }
            chans = nc;
            cap = ncap;
        }
        chans[count++] = ch;
    }

    if (count == 0) goto corrupt;
    *out_channels = chans;
    *out_count = count;
    return EXR_SUCCESS;

corrupt:
    exr_free(a, chans);
    return EXR_ERROR_CORRUPT;
}
