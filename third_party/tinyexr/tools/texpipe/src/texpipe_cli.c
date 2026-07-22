/*
 * TinyEXR texpipe CLI - resize-aware texture compression driver.
 *
 * Loads an EXR/PNG, builds a content-aware mip chain (tir) and compresses it
 * (texcomp) into a multi-mip DDS or KTX2 container.
 *
 * This is the only translation unit in texpipe that performs file I/O; the
 * library core stays free of <stdio.h> (see the texpipe-c11-gate target).
 *
 * Copyright (c) 2014-2026 Syoyo Fujita and TinyEXR authors
 * SPDX-License-Identifier: Apache-2.0
 */
#include "texpipe.h"
#include "exr.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define STB_IMAGE_IMPLEMENTATION
#define STBI_ONLY_PNG
#define STBI_NO_LINEAR
#define STBI_NO_HDR
#if defined(__GNUC__) || defined(__clang__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#pragma GCC diagnostic ignored "-Wunused-function"
#endif
#include "../../../examples/common/stb_image.h"
#if defined(__GNUC__) || defined(__clang__)
#pragma GCC diagnostic pop
#endif

static int ends_with(const char *s, const char *suffix) {
    size_t ns, nf;
    if (!s || !suffix) return 0;
    ns = strlen(s);
    nf = strlen(suffix);
    return ns >= nf && strcmp(s + ns - nf, suffix) == 0;
}

static int channel_index(const exr_part *part, const char *name) {
    int32_t i;
    for (i = 0; i < part->header.num_channels; ++i)
        if (strcmp(part->header.channels[i].name, name) == 0) return (int)i;
    return -1;
}

static float read_exr_sample(const exr_part *part, int ch, size_t idx) {
    exr_pixel_type t = part->header.channels[ch].pixel_type;
    if (t == EXR_PIXEL_UINT)
        return (float)((const uint32_t *)part->images[ch])[idx] / 255.0f;
    if (t == EXR_PIXEL_HALF) {
        float f;
        exr_half_to_float(&((const uint16_t *)part->images[ch])[idx], &f, 1);
        return f;
    }
    return ((const float *)part->images[ch])[idx];
}

static uint8_t to_u8(float f) {
    int v;
    if (f < 0.0f) f = 0.0f;
    if (f > 1.0f) f = 1.0f;
    v = (int)(f * 255.0f + 0.5f);
    return (uint8_t)(v < 0 ? 0 : (v > 255 ? 255 : v));
}

/* Load EXR into 8-bit RGBA (for LDR codecs). */
static int load_exr_rgba(const char *path, int part_index, uint8_t **rgba,
                         uint32_t *w, uint32_t *h) {
    exr_image img;
    exr_part *part;
    int r, g, b, a;
    size_t i, n;
    memset(&img, 0, sizeof(img));
    if (!EXR_OK(exr_load_from_file(path, NULL, &img))) return 0;
    if (part_index < 0) part_index = 0;
    if (part_index >= img.num_parts) { exr_image_free(&img); return 0; }
    part = &img.parts[part_index];
    if (part->is_deep || !part->images || part->width <= 0 || part->height <= 0) {
        exr_image_free(&img);
        return 0;
    }
    r = channel_index(part, "R");
    g = channel_index(part, "G");
    b = channel_index(part, "B");
    a = channel_index(part, "A");
    if (r < 0 || g < 0 || b < 0) { exr_image_free(&img); return 0; }
    n = (size_t)part->width * (size_t)part->height;
    *rgba = (uint8_t *)malloc(n * 4u);
    if (!*rgba) { exr_image_free(&img); return 0; }
    for (i = 0; i < n; ++i) {
        (*rgba)[i * 4 + 0] = to_u8(read_exr_sample(part, r, i));
        (*rgba)[i * 4 + 1] = to_u8(read_exr_sample(part, g, i));
        (*rgba)[i * 4 + 2] = to_u8(read_exr_sample(part, b, i));
        (*rgba)[i * 4 + 3] = a >= 0 ? to_u8(read_exr_sample(part, a, i)) : 255u;
    }
    *w = (uint32_t)part->width;
    *h = (uint32_t)part->height;
    exr_image_free(&img);
    return 1;
}

/* Load EXR into float RGB (for HDR codecs). */
static int load_exr_rgbf(const char *path, int part_index, float **rgb,
                         uint32_t *w, uint32_t *h) {
    exr_image img;
    exr_part *part;
    int r, g, b;
    size_t i, n;
    memset(&img, 0, sizeof(img));
    if (!EXR_OK(exr_load_from_file(path, NULL, &img))) return 0;
    if (part_index < 0) part_index = 0;
    if (part_index >= img.num_parts) { exr_image_free(&img); return 0; }
    part = &img.parts[part_index];
    if (part->is_deep || !part->images || part->width <= 0 || part->height <= 0) {
        exr_image_free(&img);
        return 0;
    }
    r = channel_index(part, "R");
    g = channel_index(part, "G");
    b = channel_index(part, "B");
    if (r < 0 || g < 0 || b < 0) { exr_image_free(&img); return 0; }
    n = (size_t)part->width * (size_t)part->height;
    *rgb = (float *)malloc(n * 3u * sizeof(float));
    if (!*rgb) { exr_image_free(&img); return 0; }
    for (i = 0; i < n; ++i) {
        (*rgb)[i * 3 + 0] = read_exr_sample(part, r, i);
        (*rgb)[i * 3 + 1] = read_exr_sample(part, g, i);
        (*rgb)[i * 3 + 2] = read_exr_sample(part, b, i);
    }
    *w = (uint32_t)part->width;
    *h = (uint32_t)part->height;
    exr_image_free(&img);
    return 1;
}

static int write_file(const char *path, const void *data, size_t size) {
    FILE *f = fopen(path, "wb");
    if (!f) return 0;
    if (fwrite(data, 1, size, f) != size) { fclose(f); return 0; }
    return fclose(f) == 0;
}

/* Compress a prebuilt chain and write the container to `path`. */
static int emit_container(const tp_mip_chain *chain, const tp_options *o,
                          const char *path) {
    tp_blocks b;
    size_t need, wrote = 0;
    uint8_t *buf;
    tp_result r;
    memset(&b, 0, sizeof(b));
    if (!TP_OK(tp_compress_chain(NULL, chain, o, &b))) return 0;
    need = tp_container_size(&b, o);
    if (!need) { tp_blocks_free(NULL, &b); return 0; }
    buf = (uint8_t *)malloc(need);
    if (!buf) { tp_blocks_free(NULL, &b); return 0; }
    r = tp_write_container(&b, o, buf, need, &wrote);
    tp_blocks_free(NULL, &b);
    if (!TP_OK(r)) { free(buf); return 0; }
    if (!write_file(path, buf, wrote)) { free(buf); return 0; }
    free(buf);
    return 1;
}

static int parse_codec(const char *s, tp_codec *codec, int *is_hdr) {
    *is_hdr = 0;
    if (!strcmp(s, "bc1") || !strcmp(s, "dxt1")) *codec = TP_CODEC_BC1;
    else if (!strcmp(s, "bc3") || !strcmp(s, "dxt5")) *codec = TP_CODEC_BC3;
    else if (!strcmp(s, "bc5")) *codec = TP_CODEC_BC5;
    else if (!strcmp(s, "bc7")) *codec = TP_CODEC_BC7;
    else if (!strcmp(s, "bc6h") || !strcmp(s, "bc6")) { *codec = TP_CODEC_BC6H; *is_hdr = 1; }
    else if (!strcmp(s, "etc2") || !strcmp(s, "etc2_rgba")) *codec = TP_CODEC_ETC2_RGBA;
    else if (!strcmp(s, "etc2_rgb")) *codec = TP_CODEC_ETC2_RGB;
    else if (!strcmp(s, "eac_r11")) *codec = TP_CODEC_EAC_R11;
    else if (!strcmp(s, "eac_rg11")) *codec = TP_CODEC_EAC_RG11;
    else if (!strcmp(s, "astc")) *codec = TP_CODEC_ASTC;
    else if (!strcmp(s, "astc_hdr")) { *codec = TP_CODEC_ASTC_HDR; *is_hdr = 1; }
    else return 0;
    return 1;
}

static int parse_content(const char *s, tp_content *c) {
    if (!strcmp(s, "color")) *c = TP_CONTENT_COLOR;
    else if (!strcmp(s, "alpha") || !strcmp(s, "alpha_tested")) *c = TP_CONTENT_ALPHA_TESTED;
    else if (!strcmp(s, "normal")) *c = TP_CONTENT_NORMAL;
    else if (!strcmp(s, "height")) *c = TP_CONTENT_HEIGHT;
    else return 0;
    return 1;
}

static int parse_filter(const char *s, tir_filter *f) {
    if (!strcmp(s, "auto")) *f = TIR_FILTER_AUTO;
    else if (!strcmp(s, "box")) *f = TIR_FILTER_BOX;
    else if (!strcmp(s, "triangle")) *f = TIR_FILTER_TRIANGLE;
    else if (!strcmp(s, "mitchell")) *f = TIR_FILTER_MITCHELL;
    else if (!strcmp(s, "catmull")) *f = TIR_FILTER_CATMULL_ROM;
    else if (!strcmp(s, "lanczos2")) *f = TIR_FILTER_LANCZOS2;
    else if (!strcmp(s, "lanczos3")) *f = TIR_FILTER_LANCZOS3;
    else if (!strcmp(s, "kaiser")) *f = TIR_FILTER_KAISER; /* sharp, low-ring */
    else if (!strcmp(s, "bspline")) *f = TIR_FILTER_BSPLINE;
    else if (!strcmp(s, "gaussian")) *f = TIR_FILTER_GAUSSIAN;
    else return 0;
    return 1;
}

static int parse_edge(const char *s, tir_edge_mode *e) {
    if (!strcmp(s, "clamp")) *e = TIR_EDGE_CLAMP;
    else if (!strcmp(s, "wrap")) *e = TIR_EDGE_WRAP;
    else if (!strcmp(s, "reflect")) *e = TIR_EDGE_REFLECT;
    else return 0;
    return 1;
}

/* Cube face name -> index in +X,-X,+Y,-Y,+Z,-Z order. */
static int cube_face_index(const char *s) {
    if (!strcmp(s, "+x") || !strcmp(s, "px")) return 0;
    if (!strcmp(s, "-x") || !strcmp(s, "nx")) return 1;
    if (!strcmp(s, "+y") || !strcmp(s, "py")) return 2;
    if (!strcmp(s, "-y") || !strcmp(s, "ny")) return 3;
    if (!strcmp(s, "+z") || !strcmp(s, "pz")) return 4;
    if (!strcmp(s, "-z") || !strcmp(s, "nz")) return 5;
    return -1;
}

static int parse_normal_enc(const char *s, tir_normal_enc *e) {
    if (!strcmp(s, "snorm")) *e = TIR_NORMAL_SNORM;
    else if (!strcmp(s, "unorm")) *e = TIR_NORMAL_UNORM;
    else if (!strcmp(s, "unorm_c127")) *e = TIR_NORMAL_UNORM_C127;
    else if (!strcmp(s, "rg")) *e = TIR_NORMAL_RG;
    else return 0;
    return 1;
}

static int parse_cube_layout(const char *s, tp_cube_layout *l) {
    if (!strcmp(s, "cross_h")) *l = TP_CUBE_CROSS_H;
    else if (!strcmp(s, "cross_v")) *l = TP_CUBE_CROSS_V;
    else if (!strcmp(s, "strip_h")) *l = TP_CUBE_STRIP_H;
    else if (!strcmp(s, "strip_v")) *l = TP_CUBE_STRIP_V;
    else return 0;
    return 1;
}

/* Load one face (EXR or PNG) in the codec's input domain into a fresh view.
 * LDR -> U8 RGBA, HDR -> F32 RGB. Caller frees view->data with free(). */
static int load_face(const char *path, int is_hdr, int part, tir_image_view *v) {
    uint32_t w = 0, h = 0;
    if (is_hdr) {
        float *rgbf = NULL;
        if (!ends_with(path, ".exr") || !load_exr_rgbf(path, part, &rgbf, &w, &h))
            return 0;
        v->data = rgbf; v->channels = 3; v->type = TIR_F32;
    } else {
        uint8_t *rgba = NULL;
        if (ends_with(path, ".exr")) {
            if (!load_exr_rgba(path, part, &rgba, &w, &h)) return 0;
        } else {
            int x, y, n;
            rgba = stbi_load(path, &x, &y, &n, 4);
            if (!rgba || x <= 0 || y <= 0) return 0;
            w = (uint32_t)x; h = (uint32_t)y;
        }
        v->data = rgba; v->channels = 4; v->type = TIR_U8;
    }
    v->width = (int)w; v->height = (int)h; v->row_stride_bytes = 0;
    return 1;
}

static void usage(void) {
    printf(
        "texpipe - resize-aware texture compression (mip chain + block compress)\n"
        "usage: texpipe -i in.{exr,png} -o out.{ktx2,dds} --format FMT [opts]\n"
        "  --format   bc1|bc3|bc5|bc7|bc6h|etc2|etc2_rgb|eac_r11|eac_rg11|astc|astc_hdr\n"
        "  --content  color|alpha|normal|height        (default color)\n"
        "  --container dds|ktx2                         (default per codec)\n"
        "  --filter   auto|box|triangle|mitchell|catmull|lanczos2|lanczos3|kaiser|bspline|gaussian\n"
        "  --edge     clamp|wrap|reflect               (per-axis both)\n"
        "  --levels N            cap mip levels (0=full chain)\n"
        "  --mip-source base|previous\n"
        "  --srgb               tag container sRGB\n"
        "  --alpha    premultiply|straight|premultiplied\n"
        "  --alpha-threshold F  (default 0.5)\n"
        "  --astc-block WxH     (default 4x4)\n"
        "  --threads N\n"
        "  --part N             EXR part index\n"
        "  normal/height (--content normal|height):\n"
        "    --normal-enc snorm|unorm|unorm_c127|rg   (default unorm)\n"
        "    --bake-roughness     write a companion Toksvig roughness mip chain\n"
        "    --base-roughness F   material base roughness (default 0.1)\n"
        "    --rough-out FILE     roughness companion path (default <out>.rough.ktx2)\n"
        "  cubemap (seam-free LOD):\n"
        "    --cube-face <+x|-x|+y|-y|+z|-z> FILE   (give all 6; square, equal size)\n"
        "    --cube-layout cross_h|cross_v|strip_h|strip_v  (split single -i input)\n"
        "    --no-seam-fixup      disable cross-face edge/corner averaging\n"
        "  octahedral env map:\n"
        "    --octa               fold-seam-aware mips for an octahedral 2D map\n"
        "  packed material maps (ORM/mask):\n"
        "    --channel-ops l,l,m,l  per-channel R,G,B,A: l=linear, m=majority(binary), r=roughness\n"
        "  --srgb-resize          decode sRGB->linear, filter, re-encode (correct albedo mips)\n"
        "  --ycocg                store colour as YCoCg (shader inverts; helps low-bit codecs)\n"
        "  --bc7-weights R,G,B,A  per-channel BC7 error weights (e.g. 2,2,1,1)\n"
        "  displacement / atlas:\n"
        "    --minmax             build a (min,max) height pyramid (BC5) for POM/relief\n"
        "    --dilate N           flood valid texels into alpha<0.5 gutter N passes\n");
}

int main(int argc, char **argv) {
    const char *in = NULL, *out = NULL;
    const char *fmt = NULL;
    tp_options opt;
    tp_content content = TP_CONTENT_COLOR;
    tp_codec codec = TP_CODEC_BC7;
    tir_filter filter = TIR_FILTER_AUTO;
    tir_edge_mode edge = TIR_EDGE_CLAMP;
    int is_hdr = 0, have_content = 0, have_filter = 0, have_edge = 0;
    int levels = 0, threads = 0, part = 0, srgb = 0;
    int container = -1; /* -1 = default per codec */
    int mip_prev = 0;
    float alpha_threshold = 0.5f;
    int alpha_mode = -1; /* -1 default */
    uint32_t astc_bx = 4, astc_by = 4;
    int seam_fixup = 1;
    int octa = 0;
    int chan_ops[4] = {0, 0, 0, 0};
    int have_chan_ops = 0;
    int srgb_resize = 0;
    int dilate = 0, minmax = 0, ycocg = 0;
    uint8_t bc7_w[4] = {0, 0, 0, 0};
    int have_bc7_w = 0;
    const char *cube_files[6] = {0};
    int cube_n = 0;
    int cube_layout = -1; /* -1 = none */
    tir_normal_enc normal_enc = TIR_NORMAL_UNORM;
    int bake_roughness = 0;
    float base_roughness = 0.1f;
    const char *rough_out = NULL;
    int i;

    uint32_t w = 0, h = 0;
    tir_image_view view;
    tir_image_view views[6];
    void *face_data[6] = {0};
    int num_faces = 1;
    tp_mip_chain chain;
    tp_result r;

    for (i = 1; i < argc; ++i) {
        const char *a = argv[i];
#define NEXT() (++i < argc ? argv[i] : NULL)
        if (!strcmp(a, "-i")) in = NEXT();
        else if (!strcmp(a, "-o")) out = NEXT();
        else if (!strcmp(a, "--format")) fmt = NEXT();
        else if (!strcmp(a, "--content")) { const char *v = NEXT(); if (!v || !parse_content(v, &content)) { usage(); return 2; } have_content = 1; }
        else if (!strcmp(a, "--container")) { const char *v = NEXT(); if (!v) { usage(); return 2; } container = !strcmp(v, "dds") ? (int)TP_CONTAINER_DDS : (!strcmp(v, "ktx2") ? (int)TP_CONTAINER_KTX2 : -2); if (container == -2) { usage(); return 2; } }
        else if (!strcmp(a, "--filter")) { const char *v = NEXT(); if (!v || !parse_filter(v, &filter)) { usage(); return 2; } have_filter = 1; }
        else if (!strcmp(a, "--edge")) { const char *v = NEXT(); if (!v || !parse_edge(v, &edge)) { usage(); return 2; } have_edge = 1; }
        else if (!strcmp(a, "--levels")) { const char *v = NEXT(); if (!v) { usage(); return 2; } levels = atoi(v); }
        else if (!strcmp(a, "--mip-source")) { const char *v = NEXT(); if (!v) { usage(); return 2; } mip_prev = !strcmp(v, "previous"); }
        else if (!strcmp(a, "--srgb")) srgb = 1;
        else if (!strcmp(a, "--alpha")) { const char *v = NEXT(); if (!v) { usage(); return 2; } alpha_mode = !strcmp(v, "straight") ? TIR_ALPHA_STRAIGHT : (!strcmp(v, "premultiplied") ? TIR_ALPHA_PREMULTIPLIED : TIR_ALPHA_PREMULTIPLY); }
        else if (!strcmp(a, "--alpha-threshold")) { const char *v = NEXT(); if (!v) { usage(); return 2; } alpha_threshold = (float)atof(v); }
        else if (!strcmp(a, "--astc-block")) { const char *v = NEXT(); unsigned bx, by; if (!v || sscanf(v, "%ux%u", &bx, &by) != 2) { usage(); return 2; } astc_bx = bx; astc_by = by; }
        else if (!strcmp(a, "--threads")) { const char *v = NEXT(); if (!v) { usage(); return 2; } threads = atoi(v); }
        else if (!strcmp(a, "--part")) { const char *v = NEXT(); if (!v) { usage(); return 2; } part = atoi(v); }
        else if (!strcmp(a, "--cube-face")) { const char *nm = NEXT(); const char *fl = NEXT(); int idx; if (!nm || !fl || (idx = cube_face_index(nm)) < 0) { usage(); return 2; } cube_files[idx] = fl; ++cube_n; }
        else if (!strcmp(a, "--cube-layout")) { const char *v = NEXT(); if (!v || !parse_cube_layout(v, (tp_cube_layout *)&cube_layout)) { usage(); return 2; } }
        else if (!strcmp(a, "--no-seam-fixup")) seam_fixup = 0;
        else if (!strcmp(a, "--octa")) octa = 1;
        else if (!strcmp(a, "--channel-ops")) {
            const char *vv = NEXT();
            int ci = 0;
            if (!vv) { usage(); return 2; }
            /* comma list of l|m per channel R,G,B,A (l=linear, m=majority) */
            while (*vv && ci < 4) {
                if (*vv == 'm' || *vv == 'M') chan_ops[ci++] = 1;
                else if (*vv == 'r' || *vv == 'R') chan_ops[ci++] = 2;
                else if (*vv == 'l' || *vv == 'L') chan_ops[ci++] = 0;
                else if (*vv == ',') { ++vv; continue; }
                ++vv;
            }
            have_chan_ops = 1;
        }
        else if (!strcmp(a, "--srgb-resize")) { srgb_resize = 1; srgb = 1; }
        else if (!strcmp(a, "--dilate")) { const char *v = NEXT(); if (!v) { usage(); return 2; } dilate = atoi(v); }
        else if (!strcmp(a, "--minmax")) minmax = 1;
        else if (!strcmp(a, "--ycocg")) ycocg = 1;
        else if (!strcmp(a, "--bc7-weights")) {
            const char *v = NEXT();
            unsigned wr = 1, wg = 1, wb = 1, wa = 1;
            if (!v || sscanf(v, "%u,%u,%u,%u", &wr, &wg, &wb, &wa) != 4) { usage(); return 2; }
            bc7_w[0] = (uint8_t)(wr > 255 ? 255 : wr); bc7_w[1] = (uint8_t)(wg > 255 ? 255 : wg);
            bc7_w[2] = (uint8_t)(wb > 255 ? 255 : wb); bc7_w[3] = (uint8_t)(wa > 255 ? 255 : wa);
            have_bc7_w = 1;
        }
        else if (!strcmp(a, "--normal-enc")) { const char *v = NEXT(); if (!v || !parse_normal_enc(v, &normal_enc)) { usage(); return 2; } }
        else if (!strcmp(a, "--bake-roughness")) bake_roughness = 1;
        else if (!strcmp(a, "--base-roughness")) { const char *v = NEXT(); if (!v) { usage(); return 2; } base_roughness = (float)atof(v); }
        else if (!strcmp(a, "--rough-out")) rough_out = NEXT();
        else if (!strcmp(a, "-h") || !strcmp(a, "--help")) { usage(); return 0; }
        else { fprintf(stderr, "unknown argument: %s\n", a); usage(); return 2; }
#undef NEXT
    }

    if (!out) { usage(); return 2; }
    if (minmax) { codec = TP_CODEC_BC5; is_hdr = 0; } /* min-max forces BC5 */
    else if (!fmt) { usage(); return 2; }
    else if (!parse_codec(fmt, &codec, &is_hdr)) { fprintf(stderr, "bad --format %s\n", fmt); return 2; }

    if (cube_n > 0 || cube_layout >= 0) {
        num_faces = 6;
        if (cube_layout >= 0) {
            /* Single packed image split into 6 sub-rect face views. */
            if (!in) { usage(); return 2; }
            if (!load_face(in, is_hdr, part, &view)) { fprintf(stderr, "failed to load %s\n", in); return 1; }
            face_data[0] = view.data;
            if (!TP_OK(tp_cube_split(&view, (tp_cube_layout)cube_layout, views))) {
                fprintf(stderr, "cube split failed: image %dx%d does not fit the layout (need square, equal faces)\n", view.width, view.height);
                free(view.data);
                return 1;
            }
            w = (uint32_t)view.width; h = (uint32_t)view.height;
        } else {
            int f;
            if (cube_n != 6) { fprintf(stderr, "cube needs all 6 --cube-face entries (+x -x +y -y +z -z)\n"); return 1; }
            for (f = 0; f < 6; ++f) {
                if (!cube_files[f]) { fprintf(stderr, "missing a cube face\n"); for (i = 0; i < 6; ++i) free(face_data[i]); return 1; }
                if (!load_face(cube_files[f], is_hdr, part, &views[f])) { fprintf(stderr, "failed to load %s\n", cube_files[f]); for (i = 0; i < 6; ++i) free(face_data[i]); return 1; }
                face_data[f] = views[f].data;
                if (views[f].width != views[0].width || views[f].height != views[0].height) {
                    fprintf(stderr, "cube faces must all be the same size\n");
                    for (i = 0; i < 6; ++i) free(face_data[i]);
                    return 1;
                }
            }
            if (views[0].width != views[0].height) { fprintf(stderr, "cube faces must be square\n"); for (i = 0; i < 6; ++i) free(face_data[i]); return 1; }
            w = (uint32_t)views[0].width; h = (uint32_t)views[0].height;
        }
    } else {
        if (!in) { usage(); return 2; }
        if (!load_face(in, is_hdr, part, &view)) { fprintf(stderr, "failed to load %s\n", in); return 1; }
        face_data[0] = view.data;
        views[0] = view;
        w = (uint32_t)view.width; h = (uint32_t)view.height;
    }

    tp_options_init(&opt, content, codec);
    if (have_filter) opt.filter = filter;
    if (have_edge) { opt.edge_x = edge; opt.edge_y = edge; }
    if (container >= 0) opt.container = (tp_container)container;
    opt.max_levels = levels;
    opt.mip_source = mip_prev ? TP_MIP_FROM_PREVIOUS : TP_MIP_FROM_BASE;
    opt.srgb = srgb;
    if (alpha_mode >= 0) opt.alpha = (tir_alpha_mode)alpha_mode;
    opt.alpha_test_threshold = alpha_threshold;
    if (have_content && content == TP_CONTENT_ALPHA_TESTED) opt.preserve_alpha_coverage = 1;
    opt.astc.block_x = astc_bx;
    opt.astc.block_y = astc_by;
    opt.threads = threads;
    opt.is_cube = (num_faces == 6);
    opt.cube_seam_fixup = seam_fixup;
    if (octa) { opt.projection = TP_PROJ_OCTA; opt.octa_seam_fixup = seam_fixup; }
    if (have_chan_ops) {
        int ci;
        for (ci = 0; ci < 4; ++ci)
            opt.channel_op[ci] = (chan_ops[ci] == 2) ? TP_CH_ROUGHNESS
                                 : (chan_ops[ci] == 1) ? TP_CH_MAJORITY
                                                       : TP_CH_LINEAR;
    }
    if (srgb_resize) opt.srgb_aware = 1;
    if (ycocg) opt.ycocg = 1;
    if (have_bc7_w) { int k; for (k = 0; k < 4; ++k) opt.bc7.channel_weights[k] = bc7_w[k]; }
    if (content == TP_CONTENT_NORMAL) {
        opt.normal_encoding = normal_enc;
        opt.base_roughness = base_roughness;
        opt.bake_toksvig_roughness = bake_roughness;
    }

    /* Gutter dilation pre-pass on the (LDR) base so mips don't bleed borders. */
    if (dilate > 0 && num_faces == 1 && view.type == TIR_U8 && view.channels == 4) {
        int W = view.width, H = view.height, k;
        uint8_t *u = (uint8_t *)view.data;
        float *fd = (float *)malloc((size_t)W * H * 4 * sizeof(float));
        if (fd) {
            tp_surface s;
            for (k = 0; k < W * H * 4; ++k) fd[k] = u[k] / 255.0f;
            s.width = W; s.height = H; s.channels = 4;
            s.data = fd; s.stride = (size_t)W * 4 * sizeof(float);
            tp_dilate(&s, 3, 0.5f, dilate);
            for (k = 0; k < W * H; ++k) {
                u[k * 4 + 0] = to_u8(fd[k * 4 + 0]);
                u[k * 4 + 1] = to_u8(fd[k * 4 + 1]);
                u[k * 4 + 2] = to_u8(fd[k * 4 + 2]);
            }
            free(fd);
        }
    }

    /* Min-max height pyramid (POM/relief): 2-channel (min,max) -> BC5. */
    if (minmax && num_faces == 1) {
        tp_mip_chain mm;
        memset(&mm, 0, sizeof(mm));
        r = tp_build_minmax_pyramid(NULL, &view, 0, opt.max_levels, &mm);
        for (i = 0; i < 6; ++i) free(face_data[i]);
        if (!TP_OK(r)) { fprintf(stderr, "texpipe: %s\n", tp_result_string(r)); return 1; }
        opt.codec = TP_CODEC_BC5;
        if (!emit_container(&mm, &opt, out)) {
            fprintf(stderr, "texpipe: failed to write %s\n", out);
            tp_mip_chain_free(NULL, &mm);
            return 1;
        }
        printf("wrote %s (min-max height pyramid, BC5, %s, %ux%u)\n", out,
               opt.container == TP_CONTAINER_DDS ? "dds" : "ktx2", w, h);
        tp_mip_chain_free(NULL, &mm);
        return 0;
    }

    /* Staged: build the chain once so we can also emit a roughness companion. */
    memset(&chain, 0, sizeof(chain));
    r = tp_build_mips(NULL, views, num_faces, &opt, &chain);
    for (i = 0; i < 6; ++i) free(face_data[i]);
    if (!TP_OK(r)) { fprintf(stderr, "texpipe: %s\n", tp_result_string(r)); return 1; }

    if (!emit_container(&chain, &opt, out)) {
        fprintf(stderr, "texpipe: failed to encode/write %s\n", out);
        tp_mip_chain_free(NULL, &chain);
        return 1;
    }
    printf("wrote %s (%s, %ux%u%s)\n", out,
           opt.container == TP_CONTAINER_DDS ? "dds" : "ktx2", w, h,
           num_faces == 6 ? ", cubemap" : "");

    /* Toksvig roughness companion (EAC_R11 KTX2). */
    if (content == TP_CONTENT_NORMAL && bake_roughness && chain.roughness) {
        tp_mip_chain rc;
        tp_options ro;
        char *derived = NULL;
        const char *rpath = rough_out;
        memset(&rc, 0, sizeof(rc));
        if (!rpath) {
            size_t n = strlen(out);
            derived = (char *)malloc(n + 12);
            if (derived) { memcpy(derived, out, n); memcpy(derived + n, ".rough.ktx2", 12); rpath = derived; }
        }
        if (rpath && TP_OK(tp_build_roughness_chain(NULL, &chain, &rc))) {
            tp_options_init(&ro, TP_CONTENT_COLOR, TP_CODEC_EAC_R11);
            ro.container = TP_CONTAINER_KTX2;
            ro.max_levels = levels;
            ro.threads = threads;
            if (emit_container(&rc, &ro, rpath))
                printf("wrote %s (roughness, eac_r11, %ux%u)\n", rpath, w, h);
            else
                fprintf(stderr, "warning: failed to write roughness companion\n");
            tp_mip_chain_free(NULL, &rc);
        }
        free(derived);
    }

    tp_mip_chain_free(NULL, &chain);
    return 0;
}
