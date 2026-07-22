/*
 * TinyEXR texpipe - multi-mip (and, in Phase 3, multi-face) container writers.
 *
 * DDS (DX10 header) for the BC family, KTX2 for everything (BC / ETC2 / EAC /
 * ASTC). Kept separate from texcomp's single-surface writers so texcomp's
 * shipped API is untouched.
 *
 * Copyright (c) 2014-2026 Syoyo Fujita and TinyEXR authors
 * SPDX-License-Identifier: Apache-2.0
 */
#include "texpipe_internal.h"

#include <string.h>

/* ================================================================== DDS */

/* DDS flag / caps constants. */
#define TP_DDSD_CAPS 0x1u
#define TP_DDSD_HEIGHT 0x2u
#define TP_DDSD_WIDTH 0x4u
#define TP_DDSD_PIXELFORMAT 0x1000u
#define TP_DDSD_MIPMAPCOUNT 0x20000u
#define TP_DDSD_LINEARSIZE 0x80000u
#define TP_DDSCAPS_COMPLEX 0x8u
#define TP_DDSCAPS_TEXTURE 0x1000u
#define TP_DDSCAPS_MIPMAP 0x400000u
#define TP_DDSCAPS2_CUBEMAP 0x200u
#define TP_DDSCAPS2_CUBEMAP_ALLFACES 0xFC00u /* +X..-Z */
#define TP_DDPF_FOURCC 0x4u
#define TP_DDS_MISC_TEXTURECUBE 0x4u

static size_t tp_dds_payload_size(const tp_blocks *b) {
    size_t total = 0;
    int i, n = b->num_faces * b->num_levels;
    for (i = 0; i < n; ++i) total += b->blk[i].size;
    return total;
}

size_t tp_dds_size(const tp_blocks *b, const tp_options *opt) {
    tp_codec_desc d;
    if (!b || !opt) return 0;
    if (!TP_OK(tp_codec_describe(opt->codec, opt, &d)) || d.dxgi_format == 0u)
        return 0; /* not a DDS/BC-representable codec */
    return 148u + tp_dds_payload_size(b);
}

tp_result tp_dds_write(const tp_blocks *b, const tp_options *opt, uint8_t *out,
                       size_t out_size, size_t *written) {
    tp_codec_desc d;
    size_t need, off;
    int face, level;
    uint32_t caps, caps2 = 0u, flags, misc = 0u;
    if (!b || !opt || !out) return TP_ERROR_INVALID_ARGUMENT;
    if (!TP_OK(tp_codec_describe(opt->codec, opt, &d)) || d.dxgi_format == 0u)
        return TP_ERROR_UNSUPPORTED;
    need = 148u + tp_dds_payload_size(b);
    if (out_size < need) return TP_ERROR_INVALID_ARGUMENT;

    memset(out, 0, 148);
    memcpy(out, "DDS ", 4);
    tp_wr_u32(out + 4, 124u);
    flags = TP_DDSD_CAPS | TP_DDSD_HEIGHT | TP_DDSD_WIDTH | TP_DDSD_PIXELFORMAT |
            TP_DDSD_LINEARSIZE;
    if (b->num_levels > 1) flags |= TP_DDSD_MIPMAPCOUNT;
    tp_wr_u32(out + 8, flags);
    tp_wr_u32(out + 12, b->blk[0].height);
    tp_wr_u32(out + 16, b->blk[0].width);
    tp_wr_u32(out + 20, (uint32_t)b->blk[0].size); /* level 0 linear size */
    tp_wr_u32(out + 28, (uint32_t)b->num_levels);
    tp_wr_u32(out + 76, 32u);              /* ddspf.dwSize */
    tp_wr_u32(out + 80, TP_DDPF_FOURCC);   /* ddspf.dwFlags */
    memcpy(out + 84, "DX10", 4);           /* ddspf.dwFourCC */
    caps = TP_DDSCAPS_TEXTURE;
    if (b->num_levels > 1) caps |= TP_DDSCAPS_COMPLEX | TP_DDSCAPS_MIPMAP;
    if (b->num_faces == 6) {
        caps |= TP_DDSCAPS_COMPLEX;
        caps2 = TP_DDSCAPS2_CUBEMAP | TP_DDSCAPS2_CUBEMAP_ALLFACES;
        misc = TP_DDS_MISC_TEXTURECUBE;
    }
    tp_wr_u32(out + 108, caps);
    tp_wr_u32(out + 112, caps2);
    /* DDS_HEADER_DXT10 begins at offset 128. */
    tp_wr_u32(out + 128, d.dxgi_format);
    tp_wr_u32(out + 132, 3u);                        /* D3D11_RESOURCE_DIMENSION_TEXTURE2D */
    tp_wr_u32(out + 136, misc);
    tp_wr_u32(out + 140, 1u);                        /* arraySize */
    tp_wr_u32(out + 144, 0u);

    /* Payload: face-major, level 0 (largest) first — matches tp_blocks order. */
    off = 148u;
    for (face = 0; face < b->num_faces; ++face) {
        for (level = 0; level < b->num_levels; ++level) {
            const tp_block_level *bl = &b->blk[face * b->num_levels + level];
            memcpy(out + off, bl->data, bl->size);
            off += bl->size;
        }
    }
    if (written) *written = need;
    return TP_SUCCESS;
}

/* ================================================================= KTX2 */

/* KHR Data Format color models. */
#define TP_KDF_MODEL_BC1A 128u
#define TP_KDF_MODEL_BC3 130u
#define TP_KDF_MODEL_BC5 132u
#define TP_KDF_MODEL_BC6H 133u
#define TP_KDF_MODEL_BC7 134u
#define TP_KDF_MODEL_ETC2 161u
#define TP_KDF_MODEL_ASTC 162u
#define TP_KDF_PRIMARIES_BT709 1u
#define TP_KDF_TRANSFER_LINEAR 1u
#define TP_KDF_TRANSFER_SRGB 2u

static uint32_t tp_kdf_model(tp_codec codec) {
    switch (codec) {
    case TP_CODEC_BC1: return TP_KDF_MODEL_BC1A;
    case TP_CODEC_BC3: return TP_KDF_MODEL_BC3;
    case TP_CODEC_BC5: return TP_KDF_MODEL_BC5;
    case TP_CODEC_BC6H: return TP_KDF_MODEL_BC6H;
    case TP_CODEC_BC7: return TP_KDF_MODEL_BC7;
    case TP_CODEC_ETC2_RGB:
    case TP_CODEC_ETC2_RGBA:
    case TP_CODEC_EAC_R11:
    case TP_CODEC_EAC_RG11: return TP_KDF_MODEL_ETC2;
    case TP_CODEC_ASTC:
    case TP_CODEC_ASTC_HDR: return TP_KDF_MODEL_ASTC;
    }
    return 0u;
}

/* One basic descriptor block with a single (block-spanning) sample. */
#define TP_DFD_SAMPLES 1u
#define TP_DFD_BLOCK_SIZE (24u + 16u * TP_DFD_SAMPLES)
#define TP_DFD_TOTAL (4u + TP_DFD_BLOCK_SIZE)

static void tp_write_dfd(uint8_t *p, const tp_codec_desc *d, tp_codec codec,
                         int srgb) {
    uint32_t model = tp_kdf_model(codec);
    uint32_t transfer = srgb ? TP_KDF_TRANSFER_SRGB : TP_KDF_TRANSFER_LINEAR;
    uint8_t bitlen = (uint8_t)(d->block_bytes * 8 - 1);
    if (d->is_hdr) transfer = TP_KDF_TRANSFER_LINEAR;
    memset(p, 0, TP_DFD_TOTAL);
    tp_wr_u32(p + 0, TP_DFD_TOTAL);
    /* descriptor block */
    tp_wr_u32(p + 4, 0u);                             /* vendorId|descriptorType */
    tp_wr_u32(p + 8, 2u | (TP_DFD_BLOCK_SIZE << 16)); /* version | blockSize */
    p[12] = (uint8_t)model;
    p[13] = (uint8_t)TP_KDF_PRIMARIES_BT709;
    p[14] = (uint8_t)transfer;
    p[15] = 0u;                                       /* flags */
    p[16] = (uint8_t)(d->block_w - 1);                /* texelBlockDimension0 */
    p[17] = (uint8_t)(d->block_h - 1);
    p[18] = 0u;
    p[19] = 0u;
    p[20] = (uint8_t)d->block_bytes;                  /* bytesPlane0 */
    /* bytesPlane1..7 = 0, texelBlockDimension2..3 = 0 (already memset) */
    /* sample 0 at offset 28 */
    p[28] = 0u; p[29] = 0u;                           /* bitOffset = 0 */
    p[30] = bitlen;                                   /* bitLength */
    p[31] = 0u;                                       /* channelType */
    /* samplePosition0..3 = 0 (p[32..35]) */
    tp_wr_u32(p + 36, 0u);                            /* sampleLower */
    tp_wr_u32(p + 40, 0xffffffffu);                   /* sampleUpper */
}

static size_t tp_align_up(size_t v, size_t a) {
    if (a <= 1) return v;
    return (v + (a - 1)) / a * a;
}

/* Byte length of one mip level = sum of all faces at that level. */
static size_t tp_ktx2_level_len(const tp_blocks *b, int level) {
    size_t total = 0;
    int face;
    for (face = 0; face < b->num_faces; ++face)
        total += b->blk[face * b->num_levels + level].size;
    return total;
}

/* Compute the file layout: data region base + per-level absolute offsets
 * (levels stored smallest-first, aligned to the texel block size). Returns the
 * total file size; fills level_off/level_len if non-NULL. */
static size_t tp_ktx2_layout(const tp_blocks *b, const tp_codec_desc *d,
                             uint64_t *level_off, uint64_t *level_len) {
    size_t idx_bytes = (size_t)b->num_levels * 24u;
    size_t align = (size_t)d->block_bytes;
    size_t cursor = 80u + idx_bytes + TP_DFD_TOTAL; /* + kvd(0) + sgd(0) */
    int ll;
    if (align < 4u) align = 4u;
    for (ll = b->num_levels - 1; ll >= 0; --ll) {
        size_t len = tp_ktx2_level_len(b, ll);
        cursor = tp_align_up(cursor, align);
        if (level_off) level_off[ll] = cursor;
        if (level_len) level_len[ll] = len;
        cursor += len;
    }
    return cursor;
}

size_t tp_ktx2_size(const tp_blocks *b, const tp_options *opt) {
    tp_codec_desc d;
    if (!b || !opt) return 0;
    if (!TP_OK(tp_codec_describe(opt->codec, opt, &d)) || d.vk_format == 0u) return 0;
    return tp_ktx2_layout(b, &d, NULL, NULL);
}

tp_result tp_ktx2_write(const tp_blocks *b, const tp_options *opt, uint8_t *out,
                        size_t out_size, size_t *written) {
    static const uint8_t id[12] = {0xAB, 0x4B, 0x54, 0x58, 0x20, 0x32,
                                   0x30, 0xBB, 0x0D, 0x0A, 0x1A, 0x0A};
    tp_codec_desc d;
    uint64_t level_off[32], level_len[32];
    size_t need, idx_base, dfd_off;
    int ll, face;
    if (!b || !opt || !out) return TP_ERROR_INVALID_ARGUMENT;
    if (b->num_levels > 32) return TP_ERROR_UNSUPPORTED;
    if (!TP_OK(tp_codec_describe(opt->codec, opt, &d)) || d.vk_format == 0u)
        return TP_ERROR_UNSUPPORTED;
    need = tp_ktx2_layout(b, &d, level_off, level_len);
    if (out_size < need) return TP_ERROR_INVALID_ARGUMENT;
    memset(out, 0, need);

    memcpy(out, id, 12);
    tp_wr_u32(out + 12, d.vk_format);
    tp_wr_u32(out + 16, 1u);                      /* typeSize */
    tp_wr_u32(out + 20, b->blk[0].width);
    tp_wr_u32(out + 24, b->blk[0].height);
    tp_wr_u32(out + 28, 0u);                      /* pixelDepth */
    tp_wr_u32(out + 32, 0u);                      /* layerCount (0 = non-array) */
    tp_wr_u32(out + 36, (uint32_t)b->num_faces);  /* faceCount */
    tp_wr_u32(out + 40, (uint32_t)b->num_levels); /* levelCount */
    tp_wr_u32(out + 44, 0u);                      /* supercompressionScheme */

    idx_base = 80u;
    dfd_off = idx_base + (size_t)b->num_levels * 24u;
    tp_wr_u32(out + 48, (uint32_t)dfd_off);       /* dfdByteOffset */
    tp_wr_u32(out + 52, TP_DFD_TOTAL);            /* dfdByteLength */
    tp_wr_u32(out + 56, 0u);                      /* kvdByteOffset */
    tp_wr_u32(out + 60, 0u);                      /* kvdByteLength */
    tp_wr_u64(out + 64, 0u);                      /* sgdByteOffset */
    tp_wr_u64(out + 72, 0u);                      /* sgdByteLength */

    /* Level index: one entry per level, index 0 = base (largest). */
    for (ll = 0; ll < b->num_levels; ++ll) {
        uint8_t *e = out + idx_base + (size_t)ll * 24u;
        tp_wr_u64(e + 0, level_off[ll]);
        tp_wr_u64(e + 8, level_len[ll]);
        tp_wr_u64(e + 16, level_len[ll]); /* uncompressedByteLength */
    }

    tp_write_dfd(out + dfd_off, &d, opt->codec, opt->srgb);

    /* Level data: for each level, faces contiguous (KTX2 order layer,face,z). */
    for (ll = 0; ll < b->num_levels; ++ll) {
        size_t off = (size_t)level_off[ll];
        for (face = 0; face < b->num_faces; ++face) {
            const tp_block_level *bl = &b->blk[face * b->num_levels + ll];
            memcpy(out + off, bl->data, bl->size);
            off += bl->size;
        }
    }
    if (written) *written = need;
    return TP_SUCCESS;
}

/* ---- KTX2 texture arrays (layerCount) ---- */

static size_t tp_ktx2_array_layout(const tp_blocks *layers, int num_layers,
                                   const tp_codec_desc *d, uint64_t *level_off,
                                   uint64_t *level_len) {
    size_t idx_bytes = (size_t)layers[0].num_levels * 24u;
    size_t align = (size_t)d->block_bytes;
    size_t cursor = 80u + idx_bytes + TP_DFD_TOTAL;
    int ll;
    if (align < 4u) align = 4u;
    for (ll = layers[0].num_levels - 1; ll >= 0; --ll) {
        size_t len = (size_t)num_layers * tp_ktx2_level_len(&layers[0], ll);
        cursor = tp_align_up(cursor, align);
        if (level_off) level_off[ll] = cursor;
        if (level_len) level_len[ll] = len;
        cursor += len;
    }
    return cursor;
}

size_t tp_ktx2_array_size(const tp_blocks *layers, int num_layers,
                          const tp_options *opt) {
    tp_codec_desc d;
    if (!layers || num_layers < 1 || !opt) return 0;
    if (!TP_OK(tp_codec_describe(opt->codec, opt, &d)) || d.vk_format == 0u) return 0;
    return tp_ktx2_array_layout(layers, num_layers, &d, NULL, NULL);
}

tp_result tp_write_ktx2_array(const tp_blocks *layers, int num_layers,
                              const tp_options *opt, uint8_t *out,
                              size_t out_size, size_t *written) {
    static const uint8_t id[12] = {0xAB, 0x4B, 0x54, 0x58, 0x20, 0x32,
                                   0x30, 0xBB, 0x0D, 0x0A, 0x1A, 0x0A};
    tp_codec_desc d;
    uint64_t level_off[32], level_len[32];
    size_t need, idx_base, dfd_off;
    int ll, layer, face, nlev, nface, li;
    if (!layers || num_layers < 1 || !opt || !out) return TP_ERROR_INVALID_ARGUMENT;
    nlev = layers[0].num_levels;
    nface = layers[0].num_faces;
    if (nlev > 32) return TP_ERROR_UNSUPPORTED;
    for (li = 1; li < num_layers; ++li)
        if (layers[li].num_levels != nlev || layers[li].num_faces != nface ||
            layers[li].codec != layers[0].codec)
            return TP_ERROR_INVALID_ARGUMENT;
    if (!TP_OK(tp_codec_describe(opt->codec, opt, &d)) || d.vk_format == 0u)
        return TP_ERROR_UNSUPPORTED;
    need = tp_ktx2_array_layout(layers, num_layers, &d, level_off, level_len);
    if (out_size < need) return TP_ERROR_INVALID_ARGUMENT;
    memset(out, 0, need);

    memcpy(out, id, 12);
    tp_wr_u32(out + 12, d.vk_format);
    tp_wr_u32(out + 16, 1u);
    tp_wr_u32(out + 20, layers[0].blk[0].width);
    tp_wr_u32(out + 24, layers[0].blk[0].height);
    tp_wr_u32(out + 28, 0u);
    tp_wr_u32(out + 32, (uint32_t)num_layers); /* layerCount */
    tp_wr_u32(out + 36, (uint32_t)nface);
    tp_wr_u32(out + 40, (uint32_t)nlev);
    tp_wr_u32(out + 44, 0u);
    idx_base = 80u;
    dfd_off = idx_base + (size_t)nlev * 24u;
    tp_wr_u32(out + 48, (uint32_t)dfd_off);
    tp_wr_u32(out + 52, TP_DFD_TOTAL);
    tp_wr_u32(out + 56, 0u);
    tp_wr_u32(out + 60, 0u);
    tp_wr_u64(out + 64, 0u);
    tp_wr_u64(out + 72, 0u);
    for (ll = 0; ll < nlev; ++ll) {
        uint8_t *e = out + idx_base + (size_t)ll * 24u;
        tp_wr_u64(e + 0, level_off[ll]);
        tp_wr_u64(e + 8, level_len[ll]);
        tp_wr_u64(e + 16, level_len[ll]);
    }
    tp_write_dfd(out + dfd_off, &d, opt->codec, opt->srgb);
    /* Level data: for each level, layer-major then face (KTX2 order). */
    for (ll = 0; ll < nlev; ++ll) {
        size_t off = (size_t)level_off[ll];
        for (layer = 0; layer < num_layers; ++layer)
            for (face = 0; face < nface; ++face) {
                const tp_block_level *bl = &layers[layer].blk[face * nlev + ll];
                memcpy(out + off, bl->data, bl->size);
                off += bl->size;
            }
    }
    if (written) *written = need;
    return TP_SUCCESS;
}
