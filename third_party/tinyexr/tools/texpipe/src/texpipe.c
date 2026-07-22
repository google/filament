/*
 * TinyEXR texpipe - core orchestration: options, codec dispatch, compression
 * of a mip chain, and the one-shot pipeline.
 *
 * Copyright (c) 2014-2026 Syoyo Fujita and TinyEXR authors
 * SPDX-License-Identifier: Apache-2.0
 */
#include "texpipe_internal.h"

#include <stdlib.h>
#include <string.h>

/* ------------------------------------------------------------------ utils */

void *tp_alloc(const tir_allocator *a, size_t size) {
    if (a && a->alloc) return a->alloc(a->user, size);
    return malloc(size);
}

void tp_dealloc(const tir_allocator *a, void *ptr) {
    if (!ptr) return;
    if (a && a->free) {
        a->free(a->user, ptr);
        return;
    }
    free(ptr);
}

void tp_free(const tir_allocator *a, void *ptr) { tp_dealloc(a, ptr); }

void tp_wr_u32(uint8_t *p, uint32_t v) {
    p[0] = (uint8_t)(v & 0xffu);
    p[1] = (uint8_t)((v >> 8) & 0xffu);
    p[2] = (uint8_t)((v >> 16) & 0xffu);
    p[3] = (uint8_t)((v >> 24) & 0xffu);
}

void tp_wr_u64(uint8_t *p, uint64_t v) {
    tp_wr_u32(p, (uint32_t)(v & 0xffffffffu));
    tp_wr_u32(p + 4, (uint32_t)((v >> 32) & 0xffffffffu));
}

const char *tp_result_string(tp_result r) {
    switch (r) {
    case TP_SUCCESS: return "success";
    case TP_ERROR_INVALID_ARGUMENT: return "invalid argument";
    case TP_ERROR_OUT_OF_MEMORY: return "out of memory";
    case TP_ERROR_UNSUPPORTED: return "unsupported";
    case TP_ERROR_IO: return "i/o error";
    }
    return "unknown";
}

int tp_level_dim(int base, int level) {
    int v = base >> level;
    return v < 1 ? 1 : v;
}

int tp_level_count(int w, int h, int max_levels) {
    int m = w > h ? w : h;
    int n = 1;
    while (m > 1) {
        m >>= 1;
        ++n;
    }
    if (max_levels > 0 && n > max_levels) n = max_levels;
    return n;
}

/* ------------------------------------------------------------------ codec */

/* DXGI_FORMAT constants (subset) — mirror texcomp.c's DDS writer choices. */
#define TP_DXGI_BC1_UNORM 71u
#define TP_DXGI_BC3_UNORM 77u
#define TP_DXGI_BC5_UNORM 83u
#define TP_DXGI_BC5_SNORM 84u
#define TP_DXGI_BC6H_UF16 95u
#define TP_DXGI_BC6H_SF16 96u
#define TP_DXGI_BC7_UNORM 98u

/* VkFormat constants (subset). */
#define TP_VK_BC1_RGBA_UNORM 133u
#define TP_VK_BC1_RGBA_SRGB 134u
#define TP_VK_BC3_UNORM 137u
#define TP_VK_BC3_SRGB 138u
#define TP_VK_BC5_UNORM 141u
#define TP_VK_BC5_SNORM 142u
#define TP_VK_BC6H_UFLOAT 143u
#define TP_VK_BC6H_SFLOAT 144u
#define TP_VK_BC7_UNORM 145u
#define TP_VK_BC7_SRGB 146u
#define TP_VK_ETC2_RGB_UNORM 147u
#define TP_VK_ETC2_RGB_SRGB 148u
#define TP_VK_ETC2_RGBA_UNORM 151u
#define TP_VK_ETC2_RGBA_SRGB 152u
#define TP_VK_EAC_R11_UNORM 153u
#define TP_VK_EAC_RG11_UNORM 155u
#define TP_VK_ASTC_4x4_UNORM 157u        /* LDR blocks step by 2 (unorm/srgb) */
#define TP_VK_ASTC_4x4_SFLOAT 1000066000u/* HDR blocks step by 1 */

/* ASTC block table in VkFormat order for the LDR base index. */
static uint32_t tp_astc_vk_format(uint32_t bx, uint32_t by, int srgb, int hdr) {
    static const uint8_t dims[14][2] = {
        {4, 4},  {5, 4},   {5, 5},   {6, 5},   {6, 6},   {8, 5},  {8, 6},
        {8, 8},  {10, 5},  {10, 6},  {10, 8},  {10, 10}, {12, 10},{12, 12}};
    int i;
    for (i = 0; i < 14; ++i) {
        if ((uint32_t)dims[i][0] == bx && (uint32_t)dims[i][1] == by) {
            if (hdr) return TP_VK_ASTC_4x4_SFLOAT + (uint32_t)i;
            return TP_VK_ASTC_4x4_UNORM + (uint32_t)(i * 2) + (srgb ? 1u : 0u);
        }
    }
    return 0u;
}

tp_result tp_codec_describe(tp_codec codec, const tp_options *opt,
                        tp_codec_desc *d) {
    int srgb = opt ? opt->srgb : 0;
    memset(d, 0, sizeof(*d));
    d->block_w = 4;
    d->block_h = 4;
    switch (codec) {
    case TP_CODEC_BC1:
        d->name = "bc1"; d->block_bytes = 8; d->channels_in = 4;
        d->dxgi_format = TP_DXGI_BC1_UNORM + (srgb ? 1u : 0u);
        d->vk_format = srgb ? TP_VK_BC1_RGBA_SRGB : TP_VK_BC1_RGBA_UNORM;
        return TP_SUCCESS;
    case TP_CODEC_BC3:
        d->name = "bc3"; d->block_bytes = 16; d->channels_in = 4;
        d->dxgi_format = TP_DXGI_BC3_UNORM + (srgb ? 1u : 0u);
        d->vk_format = srgb ? TP_VK_BC3_SRGB : TP_VK_BC3_UNORM;
        return TP_SUCCESS;
    case TP_CODEC_BC5:
        d->name = "bc5"; d->block_bytes = 16; d->channels_in = 4;
        d->dxgi_format = (opt && opt->bc5.snorm) ? TP_DXGI_BC5_SNORM
                                                 : TP_DXGI_BC5_UNORM;
        d->vk_format = (opt && opt->bc5.snorm) ? TP_VK_BC5_SNORM
                                               : TP_VK_BC5_UNORM;
        return TP_SUCCESS;
    case TP_CODEC_BC7:
        d->name = "bc7"; d->block_bytes = 16; d->channels_in = 4;
        d->dxgi_format = TP_DXGI_BC7_UNORM + (srgb ? 1u : 0u);
        d->vk_format = srgb ? TP_VK_BC7_SRGB : TP_VK_BC7_UNORM;
        return TP_SUCCESS;
    case TP_CODEC_BC6H:
        d->name = "bc6h"; d->block_bytes = 16; d->channels_in = 3; d->is_hdr = 1;
        d->dxgi_format = (opt && opt->bc6h.signed_float) ? TP_DXGI_BC6H_SF16
                                                         : TP_DXGI_BC6H_UF16;
        d->vk_format = (opt && opt->bc6h.signed_float) ? TP_VK_BC6H_SFLOAT
                                                       : TP_VK_BC6H_UFLOAT;
        return TP_SUCCESS;
    case TP_CODEC_ETC2_RGB:
        d->name = "etc2_rgb"; d->block_bytes = 8; d->channels_in = 4;
        d->vk_format = srgb ? TP_VK_ETC2_RGB_SRGB : TP_VK_ETC2_RGB_UNORM;
        return TP_SUCCESS;
    case TP_CODEC_ETC2_RGBA:
        d->name = "etc2_rgba"; d->block_bytes = 16; d->channels_in = 4;
        d->vk_format = srgb ? TP_VK_ETC2_RGBA_SRGB : TP_VK_ETC2_RGBA_UNORM;
        return TP_SUCCESS;
    case TP_CODEC_EAC_R11:
        d->name = "eac_r11"; d->block_bytes = 8; d->channels_in = 4;
        d->vk_format = TP_VK_EAC_R11_UNORM;
        return TP_SUCCESS;
    case TP_CODEC_EAC_RG11:
        d->name = "eac_rg11"; d->block_bytes = 16; d->channels_in = 4;
        d->vk_format = TP_VK_EAC_RG11_UNORM;
        return TP_SUCCESS;
    case TP_CODEC_ASTC: {
        uint32_t bx = opt ? opt->astc.block_x : 4u;
        uint32_t by = opt ? opt->astc.block_y : 4u;
        if (bx == 0u) bx = 4u;
        if (by == 0u) by = 4u;
        d->name = "astc"; d->block_bytes = 16; d->channels_in = 4;
        d->block_w = (int)bx; d->block_h = (int)by;
        d->vk_format = tp_astc_vk_format(bx, by, srgb, 0);
        if (!d->vk_format) return TP_ERROR_UNSUPPORTED;
        return TP_SUCCESS;
    }
    case TP_CODEC_ASTC_HDR:
        d->name = "astc_hdr"; d->block_bytes = 16; d->channels_in = 3;
        d->is_hdr = 1; d->block_w = 4; d->block_h = 4;
        d->vk_format = tp_astc_vk_format(4u, 4u, 0, 1);
        return TP_SUCCESS;
    }
    return TP_ERROR_UNSUPPORTED;
}

size_t tp_codec_block_size(tp_codec codec, const tp_options *opt, uint32_t w,
                           uint32_t h) {
    tp_codec_desc d;
    size_t bx, by;
    if (!TP_OK(tp_codec_describe(codec, opt, &d))) return 0;
    bx = ((size_t)w + (size_t)d.block_w - 1u) / (size_t)d.block_w;
    by = ((size_t)h + (size_t)d.block_h - 1u) / (size_t)d.block_h;
    return bx * by * (size_t)d.block_bytes;
}

static uint8_t tp_to_u8(float f) {
    int v;
    if (f < 0.0f) f = 0.0f;
    if (f > 1.0f) f = 1.0f;
    v = (int)(f * 255.0f + 0.5f);
    if (v < 0) v = 0;
    if (v > 255) v = 255;
    return (uint8_t)v;
}

/* Pack the (float) surface into a tightly-strided 8-bit RGBA buffer. */
static tp_result tp_surface_to_rgba8(const tp_surface *s, uint8_t *rgba) {
    int x, y, c;
    for (y = 0; y < s->height; ++y) {
        const float *row = (const float *)((const uint8_t *)s->data +
                                           (size_t)y * s->stride);
        uint8_t *drow = rgba + (size_t)y * (size_t)s->width * 4u;
        for (x = 0; x < s->width; ++x) {
            float px[4] = {0.0f, 0.0f, 0.0f, 1.0f};
            for (c = 0; c < s->channels && c < 4; ++c) px[c] = row[x * s->channels + c];
            drow[x * 4 + 0] = tp_to_u8(px[0]);
            drow[x * 4 + 1] = tp_to_u8(px[1]);
            drow[x * 4 + 2] = tp_to_u8(px[2]);
            drow[x * 4 + 3] = tp_to_u8(px[3]);
        }
    }
    return TP_SUCCESS;
}

tp_result tp_codec_compress(tp_codec codec, const tp_options *opt,
                            const tir_allocator *a, const tp_surface *s,
                            uint8_t *out, size_t out_size) {
    tp_codec_desc d;
    uint32_t w = (uint32_t)s->width, h = (uint32_t)s->height;
    tp_result pr = tp_codec_describe(codec, opt, &d);
    tc_result cr = TC_ERROR_UNSUPPORTED;
    if (!TP_OK(pr)) return pr;

    if (d.is_hdr) {
        /* Surface is float RGB; pass through with its own stride. */
        const float *rgb = s->data;
        size_t stride = s->stride;
        if (s->channels != 3) return TP_ERROR_INVALID_ARGUMENT;
        if (codec == TP_CODEC_BC6H)
            cr = tc_bc6h_compress_rgb32f(rgb, w, h, stride, &opt->bc6h, out,
                                         out_size);
        else if (codec == TP_CODEC_ASTC_HDR)
            cr = tc_astc_hdr_compress_rgbf(rgb, w, h, stride, &opt->astc_hdr,
                                           out, out_size);
        return (cr == TC_SUCCESS) ? TP_SUCCESS : TP_ERROR_UNSUPPORTED;
    }

    /* LDR path: pack to RGBA8 then dispatch. */
    {
        size_t n = (size_t)w * (size_t)h * 4u;
        uint8_t *rgba = (uint8_t *)tp_alloc(a, n);
        size_t stride = (size_t)w * 4u;
        if (!rgba) return TP_ERROR_OUT_OF_MEMORY;
        tp_surface_to_rgba8(s, rgba);
        /* Top-level srgb/threads are authoritative; copy into codec structs. */
        switch (codec) {
        case TP_CODEC_BC1: {
            tc_bc1_options o = opt->bc1; o.srgb = opt->srgb;
            cr = tc_bc1_compress_rgba8(rgba, w, h, stride, &o, out, out_size);
            break;
        }
        case TP_CODEC_BC3: {
            tc_bc3_options o = opt->bc3; o.srgb = opt->srgb;
            cr = tc_bc3_compress_rgba8(rgba, w, h, stride, &o, out, out_size);
            break;
        }
        case TP_CODEC_BC5:
            cr = tc_bc5_compress_rgba8(rgba, w, h, stride, &opt->bc5, out, out_size);
            break;
        case TP_CODEC_BC7: {
            tc_bc7_options o = opt->bc7; o.srgb = opt->srgb;
            if (opt->threads) o.threads = opt->threads;
            cr = tc_bc7_compress_rgba8(rgba, w, h, stride, &o, out, out_size);
            break;
        }
        case TP_CODEC_ETC2_RGB: {
            tc_etc2_options e = opt->etc2; e.alpha = 0; e.srgb = opt->srgb;
            cr = tc_etc2_compress_rgba8(rgba, w, h, stride, &e, out, out_size);
            break;
        }
        case TP_CODEC_ETC2_RGBA: {
            tc_etc2_options e = opt->etc2; e.alpha = 1; e.srgb = opt->srgb;
            cr = tc_etc2_compress_rgba8(rgba, w, h, stride, &e, out, out_size);
            break;
        }
        case TP_CODEC_EAC_R11:
            cr = tc_eac_compress_rgba8(rgba, w, h, stride, 0, out, out_size);
            break;
        case TP_CODEC_EAC_RG11:
            cr = tc_eac_compress_rgba8(rgba, w, h, stride, 1, out, out_size);
            break;
        case TP_CODEC_ASTC: {
            tc_astc_options o = opt->astc; o.srgb = opt->srgb;
            if (opt->threads) o.threads = opt->threads;
            if (o.block_x == 0u) o.block_x = 4u;
            if (o.block_y == 0u) o.block_y = 4u;
            cr = tc_astc_compress_rgba8(rgba, w, h, stride, &o, out, out_size);
            break;
        }
        default:
            cr = TC_ERROR_UNSUPPORTED;
            break;
        }
        tp_dealloc(a, rgba);
    }
    return (cr == TC_SUCCESS) ? TP_SUCCESS : TP_ERROR_UNSUPPORTED;
}

/* ------------------------------------------------------------- options */

void tp_options_init(tp_options *opt, tp_content content, tp_codec codec) {
    if (!opt) return;
    memset(opt, 0, sizeof(*opt));
    opt->content = content;
    opt->codec = codec;
    opt->container = TP_CONTAINER_KTX2;
    opt->mip_source = TP_MIP_FROM_BASE;
    opt->max_levels = 0;
    opt->filter = TIR_FILTER_AUTO;
    opt->edge_x = TIR_EDGE_CLAMP;
    opt->edge_y = TIR_EDGE_CLAMP;
    opt->srgb = 0;
    opt->alpha = TIR_ALPHA_PREMULTIPLY;
    opt->alpha_test_threshold = 0.5f;
    opt->preserve_alpha_coverage = (content == TP_CONTENT_ALPHA_TESTED) ? 1 : 0;
    opt->normal_encoding = TIR_NORMAL_SNORM;
    opt->renormalize = 1;
    opt->bake_toksvig_roughness = 0;
    opt->base_roughness = 0.0f;
    opt->is_cube = 0;
    opt->cube_layout = TP_CUBE_SEPARATE;
    opt->cube_seam_fixup = 1;
    opt->cube_fixup_max_level = -1;
    opt->projection = TP_PROJ_2D;
    opt->octa_seam_fixup = 1;
    tc_bc7_options_init(&opt->bc7);
    tc_bc1_options_init(&opt->bc1);
    tc_bc3_options_init(&opt->bc3);
    tc_bc5_options_init(&opt->bc5);
    tc_bc6h_options_init(&opt->bc6h);
    tc_etc2_options_init(&opt->etc2);
    tc_astc_options_init(&opt->astc);
    tc_astc_hdr_options_init(&opt->astc_hdr);
    if (opt->astc.block_x == 0u) opt->astc.block_x = 4u;
    if (opt->astc.block_y == 0u) opt->astc.block_y = 4u;
    /* DDS is the natural default for the BC family; KTX2 for everything else. */
    if (codec == TP_CODEC_BC1 || codec == TP_CODEC_BC3 || codec == TP_CODEC_BC5 ||
        codec == TP_CODEC_BC6H || codec == TP_CODEC_BC7)
        opt->container = TP_CONTAINER_DDS;
}

/* ------------------------------------------------------------- compress */

void tp_blocks_free(const tir_allocator *a, tp_blocks *b) {
    int i, n;
    if (!b || !b->blk) return;
    n = b->num_faces * b->num_levels;
    for (i = 0; i < n; ++i) tp_dealloc(a, b->blk[i].data);
    tp_dealloc(a, b->blk);
    b->blk = NULL;
}

tp_result tp_compress_chain(const tir_allocator *a, const tp_mip_chain *c,
                            const tp_options *opt, tp_blocks *out) {
    int i, n;
    tp_codec_desc d;
    if (!c || !opt || !out) return TP_ERROR_INVALID_ARGUMENT;
    if (!TP_OK(tp_codec_describe(opt->codec, opt, &d))) return TP_ERROR_UNSUPPORTED;
    memset(out, 0, sizeof(*out));
    out->codec = opt->codec;
    out->num_faces = c->num_faces;
    out->num_levels = c->num_levels;
    n = c->num_faces * c->num_levels;
    out->blk = (tp_block_level *)tp_alloc(a, (size_t)n * sizeof(tp_block_level));
    if (!out->blk) return TP_ERROR_OUT_OF_MEMORY;
    memset(out->blk, 0, (size_t)n * sizeof(tp_block_level));
    for (i = 0; i < n; ++i) {
        const tp_surface *s = &c->level[i];
        size_t bs = tp_codec_block_size(opt->codec, opt, (uint32_t)s->width,
                                        (uint32_t)s->height);
        tp_result r;
        if (!bs) {
            tp_blocks_free(a, out);
            return TP_ERROR_UNSUPPORTED;
        }
        out->blk[i].data = (uint8_t *)tp_alloc(a, bs);
        if (!out->blk[i].data) {
            tp_blocks_free(a, out);
            return TP_ERROR_OUT_OF_MEMORY;
        }
        out->blk[i].size = bs;
        out->blk[i].width = (uint32_t)s->width;
        out->blk[i].height = (uint32_t)s->height;
        r = tp_codec_compress(opt->codec, opt, a, s, out->blk[i].data, bs);
        if (!TP_OK(r)) {
            tp_blocks_free(a, out);
            return r;
        }
    }
    return TP_SUCCESS;
}

/* ------------------------------------------------------------- container */

size_t tp_container_size(const tp_blocks *b, const tp_options *opt) {
    if (!b || !opt) return 0;
    if (opt->container == TP_CONTAINER_DDS) return tp_dds_size(b, opt);
    return tp_ktx2_size(b, opt);
}

tp_result tp_write_container(const tp_blocks *b, const tp_options *opt,
                             uint8_t *out, size_t out_size, size_t *written) {
    if (!b || !opt || !out) return TP_ERROR_INVALID_ARGUMENT;
    if (opt->container == TP_CONTAINER_DDS)
        return tp_dds_write(b, opt, out, out_size, written);
    return tp_ktx2_write(b, opt, out, out_size, written);
}

/* ------------------------------------------------------------- one-shot */

tp_result tp_process(const tir_allocator *a, const tir_image_view *faces,
                     int num_faces, const tp_options *opt, uint8_t **out,
                     size_t *out_size) {
    tp_mip_chain chain;
    tp_blocks blocks;
    size_t need, wrote = 0;
    uint8_t *buf;
    tp_result r;
    if (!faces || !opt || !out || !out_size) return TP_ERROR_INVALID_ARGUMENT;
    *out = NULL;
    *out_size = 0;
    memset(&chain, 0, sizeof(chain));
    memset(&blocks, 0, sizeof(blocks));
    r = tp_build_mips(a, faces, num_faces, opt, &chain);
    if (!TP_OK(r)) return r;
    r = tp_compress_chain(a, &chain, opt, &blocks);
    if (!TP_OK(r)) {
        tp_mip_chain_free(a, &chain);
        return r;
    }
    need = tp_container_size(&blocks, opt);
    if (!need) {
        tp_blocks_free(a, &blocks);
        tp_mip_chain_free(a, &chain);
        return TP_ERROR_UNSUPPORTED;
    }
    buf = (uint8_t *)tp_alloc(a, need);
    if (!buf) {
        tp_blocks_free(a, &blocks);
        tp_mip_chain_free(a, &chain);
        return TP_ERROR_OUT_OF_MEMORY;
    }
    r = tp_write_container(&blocks, opt, buf, need, &wrote);
    tp_blocks_free(a, &blocks);
    tp_mip_chain_free(a, &chain);
    if (!TP_OK(r)) {
        tp_dealloc(a, buf);
        return r;
    }
    *out = buf;
    *out_size = wrote;
    return TP_SUCCESS;
}
