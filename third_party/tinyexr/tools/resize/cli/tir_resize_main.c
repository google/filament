/*
 * tir_resize - resize EXR images with the tir library.
 *
 *   tir_resize in.exr -o out.exr --scale 0.5 --filter lanczos3 --antiring 1
 *   tir_resize in.exr -o out.exr --size 640x480 --mode height --reg vertex
 *   tir_resize normal.exr -o small.exr --mode normal
 *
 * EXR I/O via the TinyEXR v3 C API (libtinyexr3.a); part 0, channels R,G,B,A
 * (or the first 1..4 channels in file order). HALF/FLOAT channels only.
 *
 * Copyright (c) 2014-2026 Syoyo Fujita and TinyEXR authors
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "exr.h"
#include "tir.h"

/* ---- local half <-> float (file channels stay in their native type) ------- */
static float half_to_f32(uint16_t h) {
    uint16_t buf = h;
    float out;
    /* tiny scalar conversion (same semantics as the library converters) */
    uint32_t sign = ((uint32_t)h & 0x8000u) << 16;
    uint32_t exp = (h >> 10) & 0x1Fu;
    uint32_t man = h & 0x3FFu;
    uint32_t bits;
    (void)buf;
    if (exp == 0) {
        if (man == 0) {
            bits = sign;
        } else {
            int e = -1;
            do {
                man <<= 1;
                e++;
            } while (!(man & 0x400u));
            man &= 0x3FFu;
            bits = sign | (uint32_t)(127 - 15 - e) << 23 | (man << 13);
        }
    } else if (exp == 31) {
        bits = sign | 0x7F800000u | (man << 13);
    } else {
        bits = sign | ((exp + (127 - 15)) << 23) | (man << 13);
    }
    memcpy(&out, &bits, 4);
    return out;
}

static uint16_t f32_to_half(float f) {
    uint32_t bits;
    uint32_t sign, exp, man;
    memcpy(&bits, &f, 4);
    sign = (bits >> 16) & 0x8000u;
    exp = (bits >> 23) & 0xFFu;
    man = bits & 0x7FFFFFu;
    if (exp == 255)
        return (uint16_t)(sign | 0x7C00u | (man ? 0x200u | (man >> 13) : 0u));
    {
        int e = (int)exp - 127 + 15;
        if (e >= 31) return (uint16_t)(sign | 0x7C00u);
        if (e <= 0) {
            uint32_t m, shift, hm, rem, half;
            if (e < -10) return (uint16_t)sign;
            m = man | 0x800000u;
            shift = (uint32_t)(14 - e);
            hm = m >> shift;
            rem = m & ((1u << shift) - 1u);
            half = 1u << (shift - 1);
            if (rem > half || (rem == half && (hm & 1u))) hm++;
            return (uint16_t)(sign | hm);
        }
        {
            uint32_t hm = man >> 13;
            uint32_t rem = man & 0x1FFFu;
            if (rem > 0x1000u || (rem == 0x1000u && (hm & 1u))) {
                hm++;
                if (hm == 0x400u) {
                    hm = 0;
                    e++;
                    if (e >= 31) return (uint16_t)(sign | 0x7C00u);
                }
            }
            return (uint16_t)(sign | ((uint32_t)e << 10) | hm);
        }
    }
}

/* ---- option parsing -------------------------------------------------------- */

static int parse_filter(const char *s, tir_filter *out) {
    static const struct {
        const char *n;
        tir_filter f;
    } T[] = {{"auto", TIR_FILTER_AUTO},         {"box", TIR_FILTER_BOX},
             {"triangle", TIR_FILTER_TRIANGLE}, {"bspline", TIR_FILTER_BSPLINE},
             {"gaussian", TIR_FILTER_GAUSSIAN}, {"mitchell", TIR_FILTER_MITCHELL},
             {"catrom", TIR_FILTER_CATMULL_ROM},
             {"lanczos2", TIR_FILTER_LANCZOS2}, {"lanczos3", TIR_FILTER_LANCZOS3}};
    size_t i;
    for (i = 0; i < sizeof(T) / sizeof(T[0]); ++i)
        if (strcmp(s, T[i].n) == 0) {
            *out = T[i].f;
            return 1;
        }
    return 0;
}

static void usage(void) {
    printf(
        "usage: tir_resize in.exr -o out.exr (--size WxH | --scale S) [opts]\n"
        "  --filter F       box triangle bspline gaussian mitchell catrom\n"
        "                   lanczos2 lanczos3 (default: auto per mode)\n"
        "  --mode M         general | normal | height (default general)\n"
        "  --edge E         clamp | reflect | wrap (default clamp)\n"
        "  --antiring S     anti-ringing strength 0..1 (default: auto)\n"
        "  --clamp-min V    final output clamp (e.g. 0 = no negative pixels)\n"
        "  --clamp-max V\n"
        "  --hicomp         filter under highlight compression (OIIO curve)\n"
        "  --nonfinite P    keep | zero | repair | error (default keep)\n"
        "  --alpha A        premultiply | premultiplied | straight\n"
        "  --reg R          cell | vertex (heightmap registration)\n"
        "  --normal-enc N   snorm | unorm | c127 | rg\n"
        "  --threads N      band threads (needs TIR_ENABLE_THREADS build)\n"
        "  --simd L         scalar sse2 sse4.1 avx2 neon sve (cap kernels)\n"
        "  --stats          print output min/max/mean and negative count\n");
}

int main(int argc, char **argv) {
    const char *in_path = NULL, *out_path = NULL;
    int out_w = 0, out_h = 0, stats = 0, i;
    double scale = 0.0;
    tir_options opt;
    tir_options_init(&opt);

    for (i = 1; i < argc; ++i) {
        const char *a = argv[i];
        if (strcmp(a, "-o") == 0 && i + 1 < argc) {
            out_path = argv[++i];
        } else if (strcmp(a, "--size") == 0 && i + 1 < argc) {
            if (sscanf(argv[++i], "%dx%d", &out_w, &out_h) != 2) {
                fprintf(stderr, "bad --size (want WxH)\n");
                return 1;
            }
        } else if (strcmp(a, "--scale") == 0 && i + 1 < argc) {
            scale = atof(argv[++i]);
        } else if (strcmp(a, "--filter") == 0 && i + 1 < argc) {
            if (!parse_filter(argv[++i], &opt.filter_x)) {
                fprintf(stderr, "unknown filter '%s'\n", argv[i]);
                return 1;
            }
            opt.filter_y = opt.filter_x;
        } else if (strcmp(a, "--mode") == 0 && i + 1 < argc) {
            const char *m = argv[++i];
            opt.mode = strcmp(m, "normal") == 0    ? TIR_MODE_NORMAL_MAP
                       : strcmp(m, "height") == 0  ? TIR_MODE_HEIGHTMAP
                       : strcmp(m, "general") == 0 ? TIR_MODE_GENERAL
                                                   : (tir_mode)-1;
            if ((int)opt.mode < 0) {
                fprintf(stderr, "unknown mode '%s'\n", m);
                return 1;
            }
        } else if (strcmp(a, "--edge") == 0 && i + 1 < argc) {
            const char *e = argv[++i];
            opt.edge_x = opt.edge_y = strcmp(e, "reflect") == 0
                                          ? TIR_EDGE_REFLECT
                                      : strcmp(e, "wrap") == 0 ? TIR_EDGE_WRAP
                                                               : TIR_EDGE_CLAMP;
        } else if (strcmp(a, "--antiring") == 0 && i + 1 < argc) {
            opt.antiring = (float)atof(argv[++i]);
        } else if (strcmp(a, "--clamp-min") == 0 && i + 1 < argc) {
            opt.clamp_min = (float)atof(argv[++i]);
        } else if (strcmp(a, "--clamp-max") == 0 && i + 1 < argc) {
            opt.clamp_max = (float)atof(argv[++i]);
        } else if (strcmp(a, "--hicomp") == 0) {
            opt.hicomp = 1;
        } else if (strcmp(a, "--nonfinite") == 0 && i + 1 < argc) {
            const char *p = argv[++i];
            opt.nonfinite = strcmp(p, "zero") == 0     ? TIR_NONFINITE_ZERO
                            : strcmp(p, "repair") == 0 ? TIR_NONFINITE_REPAIR
                            : strcmp(p, "error") == 0  ? TIR_NONFINITE_ERROR
                                                       : TIR_NONFINITE_KEEP;
        } else if (strcmp(a, "--alpha") == 0 && i + 1 < argc) {
            const char *p = argv[++i];
            opt.alpha = strcmp(p, "premultiplied") == 0 ? TIR_ALPHA_PREMULTIPLIED
                        : strcmp(p, "straight") == 0    ? TIR_ALPHA_STRAIGHT
                                                        : TIR_ALPHA_PREMULTIPLY;
        } else if (strcmp(a, "--reg") == 0 && i + 1 < argc) {
            opt.registration = strcmp(argv[++i], "vertex") == 0
                                   ? TIR_REG_GRID_VERTEX
                                   : TIR_REG_CELL_CENTERED;
        } else if (strcmp(a, "--normal-enc") == 0 && i + 1 < argc) {
            const char *p = argv[++i];
            opt.normal_encoding = strcmp(p, "unorm") == 0 ? TIR_NORMAL_UNORM
                                  : strcmp(p, "c127") == 0
                                      ? TIR_NORMAL_UNORM_C127
                                  : strcmp(p, "rg") == 0 ? TIR_NORMAL_RG
                                                         : TIR_NORMAL_SNORM;
        } else if (strcmp(a, "--threads") == 0 && i + 1 < argc) {
            opt.num_threads = atoi(argv[++i]);
        } else if (strcmp(a, "--simd") == 0 && i + 1 < argc) {
            static const char *N[] = {"scalar", "sse2", "sse4.1",
                                      "avx2",   "neon", "sve"};
            const char *p = argv[++i];
            int l;
            for (l = 0; l < 6; ++l)
                if (strcmp(p, N[l]) == 0) break;
            if (l == 6 || tir_simd_force((tir_simd_level)l) != TIR_SUCCESS) {
                fprintf(stderr, "simd level '%s' unavailable (have: %s)\n", p,
                        tir_simd_info());
                return 1;
            }
        } else if (strcmp(a, "--stats") == 0) {
            stats = 1;
        } else if (strcmp(a, "--help") == 0 || strcmp(a, "-h") == 0) {
            usage();
            return 0;
        } else if (a[0] != '-' && !in_path) {
            in_path = a;
        } else {
            fprintf(stderr, "unknown argument '%s'\n", a);
            usage();
            return 1;
        }
    }
    if (!in_path || !out_path || (out_w <= 0 && scale <= 0.0)) {
        usage();
        return 1;
    }

    {
        exr_image img;
        exr_part *part;
        exr_result erc;
        int nch, ch_idx[4], sw, sh, dw, dh, c, status = 0;
        float *src_i = NULL, *dst_i = NULL;
        void **out_planes = NULL;
        exr_image out_img;
        exr_part out_part;

        memset(&img, 0, sizeof(img));
        erc = exr_load_from_file(in_path, NULL, &img);
        if (!EXR_OK(erc)) {
            fprintf(stderr, "load %s: %s\n", in_path, exr_result_string(erc));
            return 1;
        }
        part = &img.parts[0];
        if (img.num_parts > 1)
            fprintf(stderr, "note: %d parts, resizing part 0 only\n",
                    img.num_parts);
        if (part->is_deep || !part->images) {
            fprintf(stderr, "deep parts are not supported\n");
            status = 1;
            goto cleanup;
        }
        sw = part->width;
        sh = part->height;
        dw = out_w > 0 ? out_w : (int)(sw * scale + 0.5);
        dh = out_h > 0 ? out_h : (int)(sh * scale + 0.5);
        if (dw < 1) dw = 1;
        if (dh < 1) dh = 1;

        /* channel selection: R,G,B,A when present, else file order (<= 4) */
        {
            static const char *want[4] = {"R", "G", "B", "A"};
            int found = 0, k;
            for (k = 0; k < 4; ++k) {
                for (c = 0; c < part->header.num_channels; ++c)
                    if (strcmp(part->header.channels[c].name, want[k]) == 0)
                        break;
                if (c < part->header.num_channels) ch_idx[found++] = c;
                else if (k < 3) break; /* need contiguous RGB; A optional */
            }
            if (found < 1) {
                found = part->header.num_channels < 4
                            ? part->header.num_channels
                            : 4;
                for (c = 0; c < found; ++c) ch_idx[c] = c;
            }
            nch = found;
        }
        for (c = 0; c < nch; ++c) {
            exr_pixel_type t = part->header.channels[ch_idx[c]].pixel_type;
            if (t != EXR_PIXEL_HALF && t != EXR_PIXEL_FLOAT) {
                fprintf(stderr, "channel %s: only HALF/FLOAT supported\n",
                        part->header.channels[ch_idx[c]].name);
                status = 1;
                goto cleanup;
            }
        }

        src_i = (float *)malloc((size_t)sw * sh * nch * sizeof(float));
        dst_i = (float *)malloc((size_t)dw * dh * nch * sizeof(float));
        out_planes = (void **)calloc((size_t)part->header.num_channels,
                                     sizeof(void *));
        if (!src_i || !dst_i || !out_planes) {
            fprintf(stderr, "out of memory\n");
            status = 1;
            goto cleanup;
        }
        /* gather planar -> interleaved f32 */
        for (c = 0; c < nch; ++c) {
            exr_pixel_type t = part->header.channels[ch_idx[c]].pixel_type;
            size_t n = (size_t)sw * sh, p;
            if (t == EXR_PIXEL_FLOAT) {
                const float *pl = (const float *)part->images[ch_idx[c]];
                for (p = 0; p < n; ++p) src_i[p * nch + c] = pl[p];
            } else {
                const uint16_t *pl = (const uint16_t *)part->images[ch_idx[c]];
                for (p = 0; p < n; ++p) src_i[p * nch + c] = half_to_f32(pl[p]);
            }
        }

        {
            tir_image_view sv = {src_i, sw, sh, nch, TIR_F32, 0};
            tir_image_view dv = {dst_i, dw, dh, nch, TIR_F32, 0};
            tir_result trc = tir_resize(NULL, &sv, &dv, &opt);
            if (trc != TIR_SUCCESS) {
                fprintf(stderr, "resize: %s\n", tir_result_string(trc));
                status = 1;
                goto cleanup;
            }
        }

        if (stats) {
            for (c = 0; c < nch; ++c) {
                double mn = INFINITY, mx = -INFINITY, mean = 0;
                long neg = 0;
                size_t n = (size_t)dw * dh, p;
                for (p = 0; p < n; ++p) {
                    double v = dst_i[p * nch + c];
                    if (v < mn) mn = v;
                    if (v > mx) mx = v;
                    mean += v;
                    if (v < 0) neg++;
                }
                printf("  %s: min %g max %g mean %g negatives %ld\n",
                       part->header.channels[ch_idx[c]].name, mn, mx,
                       mean / (double)n, neg);
            }
        }

        /* scatter interleaved f32 -> planar output in the original types */
        for (c = 0; c < nch; ++c) {
            exr_pixel_type t = part->header.channels[ch_idx[c]].pixel_type;
            size_t n = (size_t)dw * dh, p;
            if (t == EXR_PIXEL_FLOAT) {
                float *pl = (float *)malloc(n * sizeof(float));
                if (!pl) {
                    fprintf(stderr, "out of memory\n");
                    status = 1;
                    goto cleanup;
                }
                for (p = 0; p < n; ++p) pl[p] = dst_i[p * nch + c];
                out_planes[ch_idx[c]] = pl;
            } else {
                uint16_t *pl = (uint16_t *)malloc(n * 2);
                if (!pl) {
                    fprintf(stderr, "out of memory\n");
                    status = 1;
                    goto cleanup;
                }
                for (p = 0; p < n; ++p)
                    pl[p] = f32_to_half(dst_i[p * nch + c]);
                out_planes[ch_idx[c]] = pl;
            }
        }

        /* build the output image: selected channels only, resized window */
        memset(&out_img, 0, sizeof(out_img));
        memset(&out_part, 0, sizeof(out_part));
        out_img.num_parts = 1;
        out_img.parts = &out_part;
        out_part.header.num_channels = nch;
        {
            static exr_channel out_ch[4];
            void *packed[4];
            for (c = 0; c < nch; ++c) {
                out_ch[c] = part->header.channels[ch_idx[c]];
                out_ch[c].x_sampling = 1;
                out_ch[c].y_sampling = 1;
                packed[c] = out_planes[ch_idx[c]];
            }
            out_part.header.channels = out_ch;
            out_part.images = packed;
            out_part.header.data_window.min_x = 0;
            out_part.header.data_window.min_y = 0;
            out_part.header.data_window.max_x = dw - 1;
            out_part.header.data_window.max_y = dh - 1;
            out_part.header.display_window = out_part.header.data_window;
            out_part.width = dw;
            out_part.height = dh;
            out_part.header.compression = part->header.compression;
            erc = exr_save_to_file(out_path, &out_img,
                                   part->header.compression);
        }
        if (!EXR_OK(erc)) {
            fprintf(stderr, "save %s: %s\n", out_path, exr_result_string(erc));
            status = 1;
            goto cleanup;
        }
        printf("%s (%dx%d) -> %s (%dx%d), %d ch, %s kernels\n", in_path, sw,
               sh, out_path, dw, dh, nch, tir_simd_info());

    cleanup:
        if (out_planes) {
            for (c = 0; c < part->header.num_channels; ++c)
                free(out_planes[c]);
            free(out_planes);
        }
        free(src_i);
        free(dst_i);
        exr_image_free(&img);
        return status;
    }
}
