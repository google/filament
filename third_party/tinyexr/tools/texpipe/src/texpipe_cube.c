/*
 * TinyEXR texpipe - seam-free cubemap LOD (AMD CubeMapGen-style edge fixup).
 *
 * Face order (KTX/D3D/GL): 0:+X 1:-X 2:+Y 3:-Y 4:+Z 5:-Z.
 * Per-face texel coords: u = col (left->right), v = row (top->bottom).
 * Edge ids:  0 = right col (u=w-1),  1 = left col (u=0),
 *            2 = bottom row (v=h-1), 3 = top row (v=0).
 *
 * The 12-edge adjacency and 8-corner incidence tables below were derived from
 * the standard cube-face direction convention
 *   +X:( 1,-v,-u) -X:(-1,-v, u) +Y:( u, 1, v)
 *   -Y:( u,-1,-v) +Z:( u,-v, 1) -Z:(-u,-v,-1)
 * by mapping each face corner (u,v = +-1) to its cube corner (+-1,+-1,+-1) and
 * grouping face-edges that share a corner pair. `reverse` means the two faces
 * walk the shared edge in opposite directions.
 *
 * Copyright (c) 2014-2026 Syoyo Fujita and TinyEXR authors
 * SPDX-License-Identifier: Apache-2.0
 */
#include "texpipe_internal.h"

#include <string.h>

/* (faceA, edgeA, faceB, edgeB, reverse) for the 12 cube edges. */
static const int TP_CUBE_EDGES[12][5] = {
    {0, 3, 2, 0, 1}, /* +X top    <-> +Y right   */
    {0, 2, 3, 0, 0}, /* +X bottom <-> -Y right   */
    {0, 1, 4, 0, 0}, /* +X left   <-> +Z right   */
    {0, 0, 5, 1, 0}, /* +X right  <-> -Z left    */
    {1, 3, 2, 1, 0}, /* -X top    <-> +Y left    */
    {1, 2, 3, 1, 1}, /* -X bottom <-> -Y left    */
    {1, 0, 4, 1, 0}, /* -X right  <-> +Z left    */
    {1, 1, 5, 0, 0}, /* -X left   <-> -Z right   */
    {2, 2, 4, 3, 0}, /* +Y bottom <-> +Z top     */
    {2, 3, 5, 3, 1}, /* +Y top    <-> -Z top     */
    {3, 3, 4, 2, 0}, /* -Y top    <-> +Z bottom  */
    {3, 2, 5, 2, 1}  /* -Y bottom <-> -Z bottom  */
};

/* Corner texel position codes: TL=0, TR=1, BL=2, BR=3
 * (col = pos&1 ? w-1 : 0,  row = pos&2 ? h-1 : 0).
 * Each of the 8 cube corners is shared by exactly 3 faces. */
static const int TP_CUBE_CORNERS[8][3][2] = {
    {{0, 0}, {2, 3}, {4, 1}}, /* C0 (+,+,+) */
    {{0, 1}, {2, 1}, {5, 0}}, /* C1 (+,+,-) */
    {{0, 2}, {3, 1}, {4, 3}}, /* C2 (+,-,+) */
    {{0, 3}, {3, 3}, {5, 2}}, /* C3 (+,-,-) */
    {{1, 1}, {2, 2}, {4, 0}}, /* C4 (-,+,+) */
    {{1, 0}, {2, 0}, {5, 1}}, /* C5 (-,+,-) */
    {{1, 3}, {3, 0}, {4, 2}}, /* C6 (-,-,+) */
    {{1, 2}, {3, 2}, {5, 3}}  /* C7 (-,-,-) */
};

/* Pointer to the i-th texel along `edge` (ordered as the derivation walks it:
 * left/right edges top->bottom, top/bottom edges left->right). */
static float *tp_edge_texel(tp_surface *s, int edge, int i) {
    int col = 0, row = 0;
    switch (edge) {
    case 0: col = s->width - 1; row = i; break;  /* right, walk v */
    case 1: col = 0;            row = i; break;  /* left,  walk v */
    case 2: col = i; row = s->height - 1; break; /* bottom, walk u */
    case 3: col = i; row = 0;             break; /* top,    walk u */
    }
    return (float *)((uint8_t *)s->data + (size_t)row * s->stride) +
           (size_t)col * (size_t)s->channels;
}

static float *tp_corner_texel(tp_surface *s, int pos) {
    int col = (pos & 1) ? s->width - 1 : 0;
    int row = (pos & 2) ? s->height - 1 : 0;
    return (float *)((uint8_t *)s->data + (size_t)row * s->stride) +
           (size_t)col * (size_t)s->channels;
}

tp_result tp_cube_seam_fixup(tp_surface faces[6], const tp_options *opt) {
    int f, e, i, c, n, ch;
    (void)opt;
    if (!faces) return TP_ERROR_INVALID_ARGUMENT;
    n = faces[0].width;
    ch = faces[0].channels;
    for (f = 0; f < 6; ++f) {
        if (faces[f].width != n || faces[f].height != n)
            return TP_ERROR_INVALID_ARGUMENT; /* faces must be square + equal */
        if (faces[f].channels != ch) return TP_ERROR_INVALID_ARGUMENT;
    }
    if (n < 1) return TP_SUCCESS;

    /* Edges: average matching border texel pairs, write back to both. */
    for (e = 0; e < 12; ++e) {
        int fa = TP_CUBE_EDGES[e][0], ea = TP_CUBE_EDGES[e][1];
        int fb = TP_CUBE_EDGES[e][2], eb = TP_CUBE_EDGES[e][3];
        int rev = TP_CUBE_EDGES[e][4];
        for (i = 0; i < n; ++i) {
            float *a = tp_edge_texel(&faces[fa], ea, i);
            float *b = tp_edge_texel(&faces[fb], eb, rev ? n - 1 - i : i);
            for (c = 0; c < ch; ++c) {
                float m = 0.5f * (a[c] + b[c]);
                a[c] = m;
                b[c] = m;
            }
        }
    }

    /* Corners: 3-way average (run after edges so endpoints are consistent). */
    for (i = 0; i < 8; ++i) {
        float *p0 = tp_corner_texel(&faces[TP_CUBE_CORNERS[i][0][0]],
                                    TP_CUBE_CORNERS[i][0][1]);
        float *p1 = tp_corner_texel(&faces[TP_CUBE_CORNERS[i][1][0]],
                                    TP_CUBE_CORNERS[i][1][1]);
        float *p2 = tp_corner_texel(&faces[TP_CUBE_CORNERS[i][2][0]],
                                    TP_CUBE_CORNERS[i][2][1]);
        for (c = 0; c < ch; ++c) {
            float m = (p0[c] + p1[c] + p2[c]) * (1.0f / 3.0f);
            p0[c] = m;
            p1[c] = m;
            p2[c] = m;
        }
    }
    return TP_SUCCESS;
}

/* -------------------------------------------------------- layout split */

static size_t tp_view_type_bytes(tir_pixel_type t) {
    switch (t) {
    case TIR_F32: return 4;
    case TIR_F16: return 2;
    case TIR_U8: return 1;
    case TIR_U16: return 2;
    }
    return 0;
}

tp_result tp_cube_split(const tir_image_view *src, tp_cube_layout layout,
                        tir_image_view out[6]) {
    /* Grid dimensions and per-face (grid_col, grid_row). Face order
     * +X,-X,+Y,-Y,+Z,-Z. Cross layouts use the common placements below with
     * no per-face rotation. */
    int cols, rows, f;
    int pos[6][2];
    int tile_w, tile_h;
    size_t row_stride, tb;
    if (!src || !out) return TP_ERROR_INVALID_ARGUMENT;

    switch (layout) {
    case TP_CUBE_STRIP_H:
        cols = 6; rows = 1;
        for (f = 0; f < 6; ++f) { pos[f][0] = f; pos[f][1] = 0; }
        break;
    case TP_CUBE_STRIP_V:
        cols = 1; rows = 6;
        for (f = 0; f < 6; ++f) { pos[f][0] = 0; pos[f][1] = f; }
        break;
    case TP_CUBE_CROSS_H: {
        static const int p[6][2] = {{2, 1}, {0, 1}, {1, 0}, {1, 2}, {1, 1}, {3, 1}};
        cols = 4; rows = 3;
        memcpy(pos, p, sizeof(p));
        break;
    }
    case TP_CUBE_CROSS_V: {
        static const int p[6][2] = {{2, 1}, {0, 1}, {1, 0}, {1, 2}, {1, 1}, {1, 3}};
        cols = 3; rows = 4;
        memcpy(pos, p, sizeof(p));
        break;
    }
    case TP_CUBE_SEPARATE:
    default:
        return TP_ERROR_INVALID_ARGUMENT; /* separate faces are loaded directly */
    }

    if (src->width % cols != 0 || src->height % rows != 0)
        return TP_ERROR_INVALID_ARGUMENT;
    tile_w = src->width / cols;
    tile_h = src->height / rows;
    if (tile_w != tile_h) return TP_ERROR_INVALID_ARGUMENT; /* faces square */

    tb = tp_view_type_bytes(src->type);
    row_stride = src->row_stride_bytes
                     ? src->row_stride_bytes
                     : (size_t)src->width * (size_t)src->channels * tb;

    for (f = 0; f < 6; ++f) {
        size_t off = (size_t)pos[f][1] * (size_t)tile_h * row_stride +
                     (size_t)pos[f][0] * (size_t)tile_w * (size_t)src->channels * tb;
        out[f].data = (uint8_t *)src->data + off;
        out[f].width = tile_w;
        out[f].height = tile_h;
        out[f].channels = src->channels;
        out[f].type = src->type;
        out[f].row_stride_bytes = row_stride;
    }
    return TP_SUCCESS;
}
