/*
 * TinyEXR - CUDA GPU backend (optional; built only with -DEXR_USE_CUDA).
 *
 * Hybrid model: the bit-serial EXR entropy stages stay on the CPU; the parallel
 * reconstruction passes (byte predictor, even/odd deinterleave) and channel
 * split/gather run on the GPU for ZIP/ZIPS/RLE/uncompressed scanline parts, as
 * does all image processing (resize, convert, color, tonemap, transfer, LUT).
 * Kernels are compiled at runtime with NVRTC (no CUDA SDK at build time); the
 * CUDA driver + NVRTC are resolved via cuew/dlopen.
 *
 * The GPU decode/encode fast path covers non-deep, non-subsampled SCANLINE
 * parts; everything else (tiled, deep, subsampled, multipart-with-ineligible-
 * parts) transparently falls back to the CPU load/save path so the result is
 * always correct.
 *
 * Copyright (c) 2014-2026 Syoyo Fujita and TinyEXR authors
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "exr_gpu.h"

#ifndef EXR_USE_CUDA
/* ===========================================================================
 * Stub backend: compiled when CUDA support is OFF. No external dependencies,
 * strict C11, links nothing extra. Every entry reports "unsupported".
 * ========================================================================= */

int exr_gpu_available(void) { return 0; }
int exr_gpu_device_count(void) { return 0; }
exr_result exr_gpu_device_name(int device, char *buf, size_t buf_size) {
    (void)device;
    if (buf && buf_size) buf[0] = '\0';
    return EXR_ERROR_UNSUPPORTED;
}
exr_result exr_gpu_context_create(const exr_allocator *alloc,
                                  const exr_gpu_options *opts,
                                  exr_gpu_context **out) {
    (void)alloc;
    (void)opts;
    if (out) *out = NULL;
    return EXR_ERROR_UNSUPPORTED;
}
void exr_gpu_context_destroy(exr_gpu_context *ctx) { (void)ctx; }

exr_result exr_gpu_load_from_file(exr_gpu_context *ctx, const char *path,
                                  const exr_allocator *alloc, exr_image *out) {
    (void)ctx;
    return exr_load_from_file(path, alloc, out);
}
exr_result exr_gpu_load_from_memory(exr_gpu_context *ctx, const void *data,
                                    size_t size, const exr_allocator *alloc,
                                    exr_image *out) {
    (void)ctx;
    return exr_load_from_memory(data, size, alloc, out);
}
exr_result exr_gpu_save_to_file(exr_gpu_context *ctx, const char *path,
                                const exr_image *img, exr_compression comp) {
    (void)ctx;
    return exr_save_to_file(path, img, comp);
}
exr_result exr_gpu_save_to_memory(exr_gpu_context *ctx, void **out_data,
                                  size_t *out_size, const exr_allocator *alloc,
                                  const exr_image *img, exr_compression comp) {
    (void)ctx;
    return exr_save_to_memory(out_data, out_size, alloc, img, comp);
}

exr_result exr_gpu_jph_decode_plan(exr_gpu_context *ctx,
                                   const struct exr_jph_cb_plan *plan,
                                   const size_t *tile_offsets, size_t out_count,
                                   int32_t *out_coeffs) {
    (void)ctx;
    (void)plan;
    (void)tile_offsets;
    (void)out_count;
    (void)out_coeffs;
    return EXR_ERROR_UNSUPPORTED;
}

exr_result exr_gpu_jph_encode_plan(exr_gpu_context *ctx,
                                   const struct exr_jph_enc_plan *plan,
                                   unsigned char *out_bytes,
                                   unsigned int out_stride,
                                   unsigned int *out_missing,
                                   unsigned int *out_len0,
                                   unsigned int *out_size) {
    (void)ctx; (void)plan; (void)out_bytes; (void)out_stride;
    (void)out_missing; (void)out_len0; (void)out_size;
    return EXR_ERROR_UNSUPPORTED;
}

exr_result exr_gpu_resize_float(exr_gpu_context *ctx, const float *src, int sw,
                                int sh, size_t ss, float *dst, int dw, int dh,
                                size_t ds, int ch, exr_resize_filter f,
                                exr_edge_mode e, int ac) {
    (void)ctx;
    return exr_resize_float(NULL, src, sw, sh, ss, dst, dw, dh, ds, ch, f, e, ac);
}
exr_result exr_gpu_convert_pixels(exr_gpu_context *ctx, void *dst,
                                  exr_pixel_type dt, const void *src,
                                  exr_pixel_type st, size_t n,
                                  exr_convert_mode m) {
    (void)ctx;
    return exr_convert_pixels(dst, dt, src, st, n, m);
}
exr_result exr_gpu_color_apply_matrix(exr_gpu_context *ctx, float *dst,
                                      const float *src, size_t n, int ch,
                                      const float m[9]) {
    (void)ctx;
    return exr_color_apply_matrix(dst, src, n, ch, m);
}
exr_result exr_gpu_tonemap_float(exr_gpu_context *ctx, float *dst,
                                 const float *src, size_t n, int ch,
                                 exr_tonemap_op op,
                                 const exr_tonemap_params *p) {
    (void)ctx;
    return exr_tonemap_float(dst, src, n, ch, op, p);
}
exr_result exr_gpu_encode_transfer(exr_gpu_context *ctx, float *dst,
                                   const float *src, size_t n, exr_transfer tf) {
    (void)ctx;
    return exr_encode_transfer(dst, src, n, tf);
}
exr_result exr_gpu_decode_transfer(exr_gpu_context *ctx, float *dst,
                                   const float *src, size_t n, exr_transfer tf) {
    (void)ctx;
    return exr_decode_transfer(dst, src, n, tf);
}
exr_result exr_gpu_lut3d_apply(exr_gpu_context *ctx, float *dst, const float *src,
                               size_t n, int ch, const exr_lut3d *lut,
                               exr_lut_interp interp) {
    (void)ctx;
    return exr_lut3d_apply(dst, src, n, ch, lut, interp);
}
exr_result exr_gpu_part_to_rgba_float(exr_gpu_context *ctx, const exr_allocator *a,
                                      const exr_part *part, float **out, int *ow,
                                      int *oh, int *oc) {
    (void)ctx;
    return exr_part_to_rgba_float(a, part, out, ow, oh, oc);
}
exr_result exr_gpu_rgba_float_to_part(exr_gpu_context *ctx, const exr_allocator *a,
                                      const float *rgba, int w, int h, int ch,
                                      exr_pixel_type dt, exr_part *out) {
    (void)ctx;
    return exr_rgba_float_to_part(a, rgba, w, h, ch, dt, out);
}

#else /* EXR_USE_CUDA ======================================================= */

#include "exr_internal.h"
#include "cuew.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "exr_gpu_kernels.cuh.inc"
#include "exr_gpu_jph_kernels.cuh.inc"

#define EXR_GPU_MAX_STREAMS 4

struct exr_gpu_context {
    exr_allocator alloc;
    CUdevice dev;
    CUcontext cuctx;
    CUmodule mod;
    CUstream stream;
    int verbose;
    size_t max_device_bytes;
    /* cached kernels */
    CUfunction k_pred_partials, k_pred_offsets, k_pred_apply, k_pred_encode;
    CUfunction k_deinterleave, k_interleave, k_strided_copy;
    CUfunction k_gather_f32, k_scatter_f32, k_convert;
    CUfunction k_color_matrix, k_transfer, k_tonemap, k_lut3d;
    CUfunction k_resize_h, k_resize_v, k_premult;
    /* HTJ2K HT block coder (separate NVRTC module) */
    CUmodule jph_mod;
    CUfunction k_jph_decode, k_jph_encode;
};

/* ---- one-time availability probe ---------------------------------------- */
static int g_probe_done = 0;
static int g_probe_ok = 0;

static void gpu_probe(void) {
    int n = 0;
    if (g_probe_done) return;
    g_probe_done = 1;
    if (cuewInit(CUEW_INIT_CUDA | CUEW_INIT_NVRTC) != CUEW_SUCCESS) return;
    if (!cuInit || !nvrtcCreateProgram) return;
    if (cuInit(0) != CUDA_SUCCESS) return;
    if (!cuDeviceGetCount || cuDeviceGetCount(&n) != CUDA_SUCCESS) return;
    if (n <= 0) return;
    g_probe_ok = 1;
}

int exr_gpu_available(void) {
    gpu_probe();
    return g_probe_ok;
}

int exr_gpu_device_count(void) {
    int n = 0;
    gpu_probe();
    if (!g_probe_ok) return 0;
    if (cuDeviceGetCount(&n) != CUDA_SUCCESS) return 0;
    return n;
}

exr_result exr_gpu_device_name(int device, char *buf, size_t buf_size) {
    CUdevice d;
    if (!buf || buf_size == 0) return EXR_ERROR_INVALID_ARGUMENT;
    buf[0] = '\0';
    gpu_probe();
    if (!g_probe_ok) return EXR_ERROR_UNSUPPORTED;
    if (cuDeviceGet(&d, device) != CUDA_SUCCESS) return EXR_ERROR_INVALID_ARGUMENT;
    if (cuDeviceGetName(buf, (int)buf_size, d) != CUDA_SUCCESS)
        return EXR_ERROR_IO;
    buf[buf_size - 1] = '\0';
    return EXR_SUCCESS;
}

/* ---- NVRTC compile + module load ---------------------------------------- */
static exr_result compile_src_to_module(exr_gpu_context *c, const char *src,
                                        const char *name, CUmodule *out_mod) {
    int major = 0, minor = 0;
    char arch[64];
    const char *opts[2];
    nvrtcProgram prog;
    size_t ptx_size = 0;
    char *ptx = NULL;
    CUresult cr;

    cuDeviceGetAttribute(&major, CU_DEVICE_ATTRIBUTE_COMPUTE_CAPABILITY_MAJOR,
                         c->dev);
    cuDeviceGetAttribute(&minor, CU_DEVICE_ATTRIBUTE_COMPUTE_CAPABILITY_MINOR,
                         c->dev);
    snprintf(arch, sizeof(arch), "--gpu-architecture=compute_%d%d", major, minor);
    opts[0] = arch;
    opts[1] = "--std=c++11";

    if (nvrtcCreateProgram(&prog, src, name, 0, NULL, NULL) != NVRTC_SUCCESS)
        return EXR_ERROR_IO;
    if (nvrtcCompileProgram(prog, 2, opts) != NVRTC_SUCCESS) {
        if (c->verbose) {
            size_t ls = 0;
            char *log;
            nvrtcGetProgramLogSize(prog, &ls);
            log = (char *)malloc(ls + 1);
            if (log) {
                nvrtcGetProgramLog(prog, log);
                log[ls] = '\0';
                fprintf(stderr, "[exr_gpu] NVRTC compile failed (%s):\n%s\n",
                        name, log);
                free(log);
            }
        }
        nvrtcDestroyProgram(&prog);
        return EXR_ERROR_IO;
    }
    if (nvrtcGetPTXSize(prog, &ptx_size) != NVRTC_SUCCESS || ptx_size == 0) {
        nvrtcDestroyProgram(&prog);
        return EXR_ERROR_IO;
    }
    ptx = (char *)malloc(ptx_size);
    if (!ptx) {
        nvrtcDestroyProgram(&prog);
        return EXR_ERROR_OUT_OF_MEMORY;
    }
    nvrtcGetPTX(prog, ptx);
    nvrtcDestroyProgram(&prog);
    cr = cuModuleLoadDataEx(out_mod, ptx, 0, NULL, NULL);
    free(ptx);
    return cr == CUDA_SUCCESS ? EXR_SUCCESS : EXR_ERROR_IO;
}

static exr_result compile_module(exr_gpu_context *c) {
    exr_result rc = compile_src_to_module(c, EXR_GPU_KERNEL_SRC,
                                          "exr_gpu_kernels.cu", &c->mod);
    if (!EXR_OK(rc)) return rc;
    rc = compile_src_to_module(c, EXR_GPU_JPH_KERNEL_SRC,
                               "exr_gpu_jph_kernels.cu", &c->jph_mod);
    if (!EXR_OK(rc)) return rc;
    if (cuModuleGetFunction(&c->k_jph_decode, c->jph_mod,
                            "exrg_jph_decode_blocks") != CUDA_SUCCESS)
        return EXR_ERROR_IO;
    if (cuModuleGetFunction(&c->k_jph_encode, c->jph_mod,
                            "exrg_jph_encode_blocks") != CUDA_SUCCESS)
        return EXR_ERROR_IO;

#define GETFN(field, name)                                                     \
    if (cuModuleGetFunction(&c->field, c->mod, name) != CUDA_SUCCESS)          \
        return EXR_ERROR_IO;
    GETFN(k_pred_partials, "exrg_pred_partials")
    GETFN(k_pred_offsets, "exrg_pred_offsets")
    GETFN(k_pred_apply, "exrg_pred_apply")
    GETFN(k_pred_encode, "exrg_pred_encode")
    GETFN(k_deinterleave, "exrg_deinterleave")
    GETFN(k_interleave, "exrg_interleave")
    GETFN(k_strided_copy, "exrg_strided_copy")
    GETFN(k_gather_f32, "exrg_gather_f32")
    GETFN(k_scatter_f32, "exrg_scatter_f32")
    GETFN(k_convert, "exrg_convert")
    GETFN(k_color_matrix, "exrg_color_matrix")
    GETFN(k_transfer, "exrg_transfer")
    GETFN(k_tonemap, "exrg_tonemap")
    GETFN(k_lut3d, "exrg_lut3d")
    GETFN(k_resize_h, "exrg_resize_h")
    GETFN(k_resize_v, "exrg_resize_v")
    GETFN(k_premult, "exrg_premult")
#undef GETFN
    return EXR_SUCCESS;
}

exr_result exr_gpu_context_create(const exr_allocator *alloc,
                                  const exr_gpu_options *opts,
                                  exr_gpu_context **out) {
    exr_gpu_context *c;
    int device = (opts && opts->device >= 0) ? opts->device : 0;
    exr_result rc;

    if (!out) return EXR_ERROR_INVALID_ARGUMENT;
    *out = NULL;
    gpu_probe();
    if (!g_probe_ok) return EXR_ERROR_UNSUPPORTED;

    if (!alloc) alloc = exr_default_allocator();
    c = (exr_gpu_context *)exr_calloc(alloc, 1, sizeof(*c));
    if (!c) return EXR_ERROR_OUT_OF_MEMORY;
    c->alloc = *alloc; /* resolved (never NULL): used for all host scratch */
    c->verbose = opts ? opts->verbose : 0;
    c->max_device_bytes = opts ? opts->max_device_bytes : 0;

    if (cuDeviceGet(&c->dev, device) != CUDA_SUCCESS) {
        exr_free(alloc, c);
        return EXR_ERROR_INVALID_ARGUMENT;
    }
    if (cuCtxCreate(&c->cuctx, 0, c->dev) != CUDA_SUCCESS) {
        exr_free(alloc, c);
        return EXR_ERROR_IO;
    }
    if (cuStreamCreate(&c->stream, CU_STREAM_DEFAULT) != CUDA_SUCCESS) {
        cuCtxDestroy(c->cuctx);
        exr_free(alloc, c);
        return EXR_ERROR_IO;
    }
    rc = compile_module(c);
    if (!EXR_OK(rc)) {
        cuStreamDestroy(c->stream);
        cuCtxDestroy(c->cuctx);
        exr_free(alloc, c);
        return rc;
    }
    *out = c;
    return EXR_SUCCESS;
}

void exr_gpu_context_destroy(exr_gpu_context *c) {
    exr_allocator a;
    if (!c) return;
    a = c->alloc;
    cuCtxSetCurrent(c->cuctx);
    cuStreamSynchronize(c->stream);
    if (c->jph_mod) cuModuleUnload(c->jph_mod);
    if (c->mod) cuModuleUnload(c->mod);
    if (c->stream) cuStreamDestroy(c->stream);
    if (c->cuctx) cuCtxDestroy(c->cuctx);
    exr_free(a.alloc ? &a : NULL, c);
}

/* ---- device memory + launch helpers ------------------------------------- */
static exr_result d_alloc(CUdeviceptr *p, size_t n) {
    if (n == 0) n = 1;
    return cuMemAlloc(p, n) == CUDA_SUCCESS ? EXR_SUCCESS : EXR_ERROR_OUT_OF_MEMORY;
}
static void d_free(CUdeviceptr p) {
    if (p) cuMemFree(p);
}
static exr_result h2d(exr_gpu_context *c, CUdeviceptr d, const void *h, size_t n) {
    if (n == 0) return EXR_SUCCESS;
    return cuMemcpyHtoDAsync(d, h, n, c->stream) == CUDA_SUCCESS ? EXR_SUCCESS
                                                                 : EXR_ERROR_IO;
}
static exr_result d2h(exr_gpu_context *c, void *h, CUdeviceptr d, size_t n) {
    if (n == 0) return EXR_SUCCESS;
    return cuMemcpyDtoHAsync(h, d, n, c->stream) == CUDA_SUCCESS ? EXR_SUCCESS
                                                                 : EXR_ERROR_IO;
}
static exr_result launch1d(exr_gpu_context *c, CUfunction f, long n,
                           void **params) {
    unsigned bs = 256;
    unsigned long long grid;
    if (n <= 0) return EXR_SUCCESS;
    grid = ((unsigned long long)n + bs - 1) / bs;
    return cuLaunchKernel(f, (unsigned)grid, 1, 1, bs, 1, 1, 0, c->stream, params,
                          NULL) == CUDA_SUCCESS
               ? EXR_SUCCESS
               : EXR_ERROR_IO;
}
static exr_result sync(exr_gpu_context *c) {
    return cuStreamSynchronize(c->stream) == CUDA_SUCCESS ? EXR_SUCCESS
                                                          : EXR_ERROR_IO;
}

/* ---- HTJ2K HT block coder: decode a collected plan on the GPU ----------- */
/* Host mirror of the kernel's JphGpuRec (must match field order/alignment). */
typedef struct {
    uint32_t width, height, missing_msbs, active_passes;
    uint32_t length0, length1, kmax;
    uint64_t data_offset;
    uint64_t out_offset;
    uint32_t out_stride;
    uint32_t eligible;
} JphGpuRecH;

/* Worst-case per-thread scratch element counts for a 128x32 code-block. */
#define JPH_GPU_SSTR_MAX 136u                       /* ((128+2)+7)&~7 */
#define JPH_GPU_SCRATCH_STRIDE (JPH_GPU_SSTR_MAX * 17u)   /* sstr*((32+1)/2+1) */
#define JPH_GPU_BUF_STRIDE (JPH_GPU_SSTR_MAX * 32u)
#define JPH_GPU_VN_STRIDE 516u

exr_result exr_gpu_jph_decode_plan(exr_gpu_context *c,
                                   const struct exr_jph_cb_plan *plan,
                                   const size_t *tile_offsets, size_t out_count,
                                   int32_t *out_coeffs) {
    size_t n, i, nelig = 0;
    JphGpuRecH *hrecs = NULL;
    const uint16_t *vlc0, *vlc1, *uvlc0, *uvlc1;
    uint32_t magsgn_stride = 4u, sigprop_stride = 4u;
    uint32_t scratch_stride = JPH_GPU_SCRATCH_STRIDE;
    uint32_t buf_stride = JPH_GPU_BUF_STRIDE;
    uint32_t vn_stride = JPH_GPU_VN_STRIDE;
    CUdeviceptr d_data = 0, d_recs = 0, d_out = 0;
    CUdeviceptr d_vlc0 = 0, d_vlc1 = 0, d_uvlc0 = 0, d_uvlc1 = 0;
    CUdeviceptr d_scratch = 0, d_buf = 0, d_vn = 0, d_magsgn = 0, d_sigprop = 0;
    exr_result rc;
    int ni;

    if (!c || !plan || !out_coeffs) return EXR_ERROR_INVALID_ARGUMENT;
    cuCtxSetCurrent(c->cuctx);
    n = plan->num_records;
    if (n == 0) return EXR_SUCCESS;

    rc = exr_jph_ht_tables(&vlc0, &vlc1, &uvlc0, &uvlc1);
    if (!EXR_OK(rc)) return rc;

    hrecs = (JphGpuRecH *)exr_malloc(&c->alloc, n * sizeof(*hrecs));
    if (!hrecs) return EXR_ERROR_OUT_OF_MEMORY;
    for (i = 0; i < n; ++i) {
        const exr_jph_cb_record *r = &plan->records[i];
        uint32_t ms_words = (uint32_t)(r->data_size / 8u + 3u);
        uint32_t sp_words = (uint32_t)(r->length1 / 8u + 3u);
        hrecs[i].width = r->width;
        hrecs[i].height = r->height;
        hrecs[i].missing_msbs = r->missing_msbs;
        hrecs[i].active_passes = r->active_passes;
        hrecs[i].length0 = r->length0;
        hrecs[i].length1 = r->length1;
        hrecs[i].kmax = r->kmax;
        hrecs[i].data_offset = (uint64_t)r->data_offset;
        hrecs[i].eligible = r->i32_eligible ? 1u : 0u;
        if (r->i32_eligible) {
            hrecs[i].out_stride = (r->width + 7u) & ~7u;
            hrecs[i].out_offset = (uint64_t)tile_offsets[i];
            if (ms_words > magsgn_stride) magsgn_stride = ms_words;
            if (sp_words > sigprop_stride) sigprop_stride = sp_words;
            ++nelig;
        } else {
            hrecs[i].out_stride = 0u;
            hrecs[i].out_offset = 0u;
        }
    }
    if (nelig == 0) { exr_free(&c->alloc, hrecs); return EXR_SUCCESS; }

    rc = d_alloc(&d_data, plan->data_size ? plan->data_size : 1u); if (!EXR_OK(rc)) goto done;
    rc = d_alloc(&d_recs, n * sizeof(JphGpuRecH)); if (!EXR_OK(rc)) goto done;
    rc = d_alloc(&d_out, out_count * sizeof(int32_t)); if (!EXR_OK(rc)) goto done;
    rc = d_alloc(&d_vlc0, 1024u * sizeof(uint16_t)); if (!EXR_OK(rc)) goto done;
    rc = d_alloc(&d_vlc1, 1024u * sizeof(uint16_t)); if (!EXR_OK(rc)) goto done;
    rc = d_alloc(&d_uvlc0, 320u * sizeof(uint16_t)); if (!EXR_OK(rc)) goto done;
    rc = d_alloc(&d_uvlc1, 256u * sizeof(uint16_t)); if (!EXR_OK(rc)) goto done;
    rc = d_alloc(&d_scratch, n * scratch_stride * sizeof(uint16_t)); if (!EXR_OK(rc)) goto done;
    rc = d_alloc(&d_buf, n * buf_stride * sizeof(uint32_t)); if (!EXR_OK(rc)) goto done;
    rc = d_alloc(&d_vn, n * vn_stride * sizeof(uint32_t)); if (!EXR_OK(rc)) goto done;
    rc = d_alloc(&d_magsgn, n * magsgn_stride * sizeof(uint64_t)); if (!EXR_OK(rc)) goto done;
    rc = d_alloc(&d_sigprop, n * sigprop_stride * sizeof(uint64_t)); if (!EXR_OK(rc)) goto done;

    /* Match the CPU reference's calloc'd scratch/buf/v_n (zero borders). */
    cuMemsetD8Async(d_scratch, 0, n * scratch_stride * sizeof(uint16_t), c->stream);
    cuMemsetD8Async(d_buf, 0, n * buf_stride * sizeof(uint32_t), c->stream);
    cuMemsetD8Async(d_vn, 0, n * vn_stride * sizeof(uint32_t), c->stream);

    rc = h2d(c, d_data, plan->data, plan->data_size); if (!EXR_OK(rc)) goto done;
    rc = h2d(c, d_recs, hrecs, n * sizeof(JphGpuRecH)); if (!EXR_OK(rc)) goto done;
    rc = h2d(c, d_vlc0, vlc0, 1024u * sizeof(uint16_t)); if (!EXR_OK(rc)) goto done;
    rc = h2d(c, d_vlc1, vlc1, 1024u * sizeof(uint16_t)); if (!EXR_OK(rc)) goto done;
    rc = h2d(c, d_uvlc0, uvlc0, 320u * sizeof(uint16_t)); if (!EXR_OK(rc)) goto done;
    rc = h2d(c, d_uvlc1, uvlc1, 256u * sizeof(uint16_t)); if (!EXR_OK(rc)) goto done;

    ni = (int)n;
    {
        void *params[18];
        params[0] = &d_data; params[1] = &d_recs; params[2] = &ni;
        params[3] = &d_vlc0; params[4] = &d_vlc1; params[5] = &d_uvlc0;
        params[6] = &d_uvlc1; params[7] = &d_out;
        params[8] = &d_scratch; params[9] = &scratch_stride;
        params[10] = &d_buf; params[11] = &buf_stride;
        params[12] = &d_vn; params[13] = &vn_stride;
        params[14] = &d_magsgn; params[15] = &magsgn_stride;
        params[16] = &d_sigprop; params[17] = &sigprop_stride;
        rc = launch1d(c, c->k_jph_decode, (long)n, params);
        if (!EXR_OK(rc)) goto done;
    }
    rc = d2h(c, out_coeffs, d_out, out_count * sizeof(int32_t));
    if (!EXR_OK(rc)) goto done;
    rc = sync(c);

done:
    d_free(d_data); d_free(d_recs); d_free(d_out);
    d_free(d_vlc0); d_free(d_vlc1); d_free(d_uvlc0); d_free(d_uvlc1);
    d_free(d_scratch); d_free(d_buf); d_free(d_vn);
    d_free(d_magsgn); d_free(d_sigprop);
    exr_free(&c->alloc, hrecs);
    return rc;
}

/* Host mirror of the kernel's JphGpuEncRec / JphGpuEncOut (match layout). */
typedef struct {
    uint32_t width, height, kmax, eligible;
    uint64_t coeff_offset;
    uint64_t out_offset;
} JphGpuEncRecH;
typedef struct { uint32_t missing, len0, size, pad; } JphGpuEncOutH;

#define JPH_GPU_ENC_MS_STRIDE 16384u
#define JPH_GPU_ENC_MEL_STRIDE 256u
#define JPH_GPU_ENC_VLC_STRIDE 3072u

exr_result exr_gpu_jph_encode_plan(exr_gpu_context *c,
                                   const struct exr_jph_enc_plan *plan,
                                   unsigned char *out_bytes,
                                   unsigned int out_stride,
                                   unsigned int *out_missing,
                                   unsigned int *out_len0,
                                   unsigned int *out_size) {
    size_t n, i;
    JphGpuEncRecH *hrecs = NULL;
    JphGpuEncOutH *houts = NULL;
    const uint16_t *venc0, *venc1;
    const uint8_t *uenc;
    uint32_t ms_stride = JPH_GPU_ENC_MS_STRIDE, mel_stride = JPH_GPU_ENC_MEL_STRIDE;
    uint32_t vlc_stride = JPH_GPU_ENC_VLC_STRIDE;
    CUdeviceptr d_coeffs = 0, d_recs = 0, d_outbytes = 0, d_outs = 0;
    CUdeviceptr d_venc0 = 0, d_venc1 = 0, d_uenc = 0;
    CUdeviceptr d_ms = 0, d_mel = 0, d_vlc = 0;
    exr_result rc;
    int ni;

    if (!c || !plan || !out_bytes) return EXR_ERROR_INVALID_ARGUMENT;
    if (out_stride < 2u) return EXR_ERROR_INVALID_ARGUMENT;
    cuCtxSetCurrent(c->cuctx);
    n = plan->num_records;
    if (n == 0) return EXR_SUCCESS;

    rc = exr_jph_ht_enc_tables(&venc0, &venc1, &uenc);
    if (!EXR_OK(rc)) return rc;

    hrecs = (JphGpuEncRecH *)exr_malloc(&c->alloc, n * sizeof(*hrecs));
    houts = (JphGpuEncOutH *)exr_malloc(&c->alloc, n * sizeof(*houts));
    if (!hrecs || !houts) { rc = EXR_ERROR_OUT_OF_MEMORY; goto done; }
    for (i = 0; i < n; ++i) {
        const exr_jph_enc_record *r = &plan->records[i];
        hrecs[i].width = r->width;
        hrecs[i].height = r->height;
        hrecs[i].kmax = r->kmax;
        hrecs[i].eligible =
            (r->plane_is_i32 && r->kmax >= 1u && r->kmax <= 30u) ? 1u : 0u;
        hrecs[i].coeff_offset = (uint64_t)r->coeff_offset;
        hrecs[i].out_offset = 0u;
    }

    rc = d_alloc(&d_coeffs, (plan->coeff_count ? plan->coeff_count : 1u) * sizeof(int32_t)); if (!EXR_OK(rc)) goto done;
    rc = d_alloc(&d_recs, n * sizeof(JphGpuEncRecH)); if (!EXR_OK(rc)) goto done;
    rc = d_alloc(&d_outbytes, n * out_stride); if (!EXR_OK(rc)) goto done;
    rc = d_alloc(&d_outs, n * sizeof(JphGpuEncOutH)); if (!EXR_OK(rc)) goto done;
    rc = d_alloc(&d_venc0, 2048u * sizeof(uint16_t)); if (!EXR_OK(rc)) goto done;
    rc = d_alloc(&d_venc1, 2048u * sizeof(uint16_t)); if (!EXR_OK(rc)) goto done;
    rc = d_alloc(&d_uenc, 75u * 6u); if (!EXR_OK(rc)) goto done;
    rc = d_alloc(&d_ms, n * ms_stride); if (!EXR_OK(rc)) goto done;
    rc = d_alloc(&d_mel, n * mel_stride); if (!EXR_OK(rc)) goto done;
    rc = d_alloc(&d_vlc, n * vlc_stride); if (!EXR_OK(rc)) goto done;

    rc = h2d(c, d_coeffs, plan->coeffs, plan->coeff_count * sizeof(int32_t)); if (!EXR_OK(rc)) goto done;
    rc = h2d(c, d_recs, hrecs, n * sizeof(JphGpuEncRecH)); if (!EXR_OK(rc)) goto done;
    rc = h2d(c, d_venc0, venc0, 2048u * sizeof(uint16_t)); if (!EXR_OK(rc)) goto done;
    rc = h2d(c, d_venc1, venc1, 2048u * sizeof(uint16_t)); if (!EXR_OK(rc)) goto done;
    rc = h2d(c, d_uenc, uenc, 75u * 6u); if (!EXR_OK(rc)) goto done;

    ni = (int)n;
    {
        void *params[16];
        params[0] = &d_coeffs; params[1] = &d_recs; params[2] = &ni;
        params[3] = &d_venc0; params[4] = &d_venc1; params[5] = &d_uenc;
        params[6] = &d_outbytes; params[7] = &out_stride; params[8] = &d_outs;
        params[9] = &d_ms; params[10] = &ms_stride;
        params[11] = &d_mel; params[12] = &mel_stride;
        params[13] = &d_vlc; params[14] = &vlc_stride;
        params[15] = NULL;
        rc = launch1d(c, c->k_jph_encode, (long)n, params);
        if (!EXR_OK(rc)) goto done;
    }
    rc = d2h(c, out_bytes, d_outbytes, n * out_stride); if (!EXR_OK(rc)) goto done;
    rc = d2h(c, houts, d_outs, n * sizeof(JphGpuEncOutH)); if (!EXR_OK(rc)) goto done;
    rc = sync(c);
    if (!EXR_OK(rc)) goto done;
    for (i = 0; i < n; ++i) {
        if (out_missing) out_missing[i] = houts[i].missing;
        if (out_len0) out_len0[i] = houts[i].len0;
        if (out_size) out_size[i] = houts[i].size;
    }

done:
    d_free(d_coeffs); d_free(d_recs); d_free(d_outbytes); d_free(d_outs);
    d_free(d_venc0); d_free(d_venc1); d_free(d_uenc);
    d_free(d_ms); d_free(d_mel); d_free(d_vlc);
    exr_free(&c->alloc, hrecs);
    exr_free(&c->alloc, houts);
    return rc;
}

/* GPU byte predictor decode over [0,n): 3-pass segmented prefix sum (mod 256). */
static exr_result gpu_predictor_decode(exr_gpu_context *c, CUdeviceptr d_in,
                                       int n, CUdeviceptr d_out) {
    int tile = 4096;
    int nblk = (n + tile - 1) / tile;
    CUdeviceptr d_part = 0, d_off = 0;
    exr_result rc;
    void *p1[4], *p2[3], *p3[5];
    if (n <= 0) return EXR_SUCCESS;
    rc = d_alloc(&d_part, (size_t)nblk * sizeof(int));
    if (!EXR_OK(rc)) return rc;
    rc = d_alloc(&d_off, (size_t)nblk * sizeof(int));
    if (!EXR_OK(rc)) { d_free(d_part); return rc; }

    p1[0] = &d_in; p1[1] = &n; p1[2] = &tile; p1[3] = &d_part;
    if (cuLaunchKernel(c->k_pred_partials, (unsigned)nblk, 1, 1, 1, 1, 1, 0,
                       c->stream, p1, NULL) != CUDA_SUCCESS) { rc = EXR_ERROR_IO; goto done; }
    p2[0] = &d_part; p2[1] = &nblk; p2[2] = &d_off;
    if (cuLaunchKernel(c->k_pred_offsets, 1, 1, 1, 1, 1, 1, 0, c->stream, p2,
                       NULL) != CUDA_SUCCESS) { rc = EXR_ERROR_IO; goto done; }
    p3[0] = &d_in; p3[1] = &n; p3[2] = &tile; p3[3] = &d_off; p3[4] = &d_out;
    if (cuLaunchKernel(c->k_pred_apply, (unsigned)nblk, 1, 1, 1, 1, 1, 0,
                       c->stream, p3, NULL) != CUDA_SUCCESS) { rc = EXR_ERROR_IO; goto done; }
    rc = EXR_SUCCESS;
done:
    d_free(d_part);
    d_free(d_off);
    return rc;
}

/* ===========================================================================
 * Hybrid decode
 * ========================================================================= */

static int part_gpu_eligible(const exr_header *h) {
    int c;
    if (h->part_type != EXR_PART_SCANLINE) return 0;
    for (c = 0; c < h->num_channels; ++c) {
        if (h->channels[c].x_sampling != 1 || h->channels[c].y_sampling != 1)
            return 0;
    }
    return 1;
}

/* GPU HTJ2K block-decode hook handed to exr_jph_decompress_gpu (user = ctx). */
static exr_result gpu_jph_decode_hook(void *user, const exr_jph_cb_plan *plan,
                                      const size_t *tile_offsets,
                                      size_t coeff_count, int32_t *coeffs) {
    return exr_gpu_jph_decode_plan((exr_gpu_context *)user, plan, tile_offsets,
                                   coeff_count, coeffs);
}

/* GPU HTJ2K block-encode hook handed to the writer (user = ctx). */
static exr_result gpu_jph_encode_hook(void *user, const struct exr_jph_enc_plan *plan,
                                      unsigned char *out_bytes,
                                      unsigned int out_stride,
                                      unsigned int *out_missing,
                                      unsigned int *out_len0,
                                      unsigned int *out_size) {
    return exr_gpu_jph_encode_plan((exr_gpu_context *)user, plan, out_bytes,
                                   out_stride, out_missing, out_len0, out_size);
}

/* Decode all scanline blocks of one eligible part into out->images on the GPU. */
static exr_result gpu_decode_part(exr_gpu_context *c, exr_reader *r, int32_t part,
                                  exr_part *out) {
    const exr_header *h = &out->header;
    int nch = h->num_channels;
    int w = out->width, hgt = out->height;
    int dwy0 = h->data_window.min_y;
    uint32_t nblocks = 0, bi_idx;
    size_t scan_stride = 0;
    size_t *ch_off = NULL, *ch_row = NULL;
    CUdeviceptr d_a = 0, d_b = 0;        /* reusable block buffers */
    CUdeviceptr d_plane = 0;             /* per-channel scratch */
    uint8_t *host_canon = NULL;
    size_t maxblock = 0, maxplane = 0;
    exr_result rc;
    int cc;

    ch_off = (size_t *)exr_calloc(&c->alloc, (size_t)nch, sizeof(size_t));
    ch_row = (size_t *)exr_calloc(&c->alloc, (size_t)nch, sizeof(size_t));
    if (!ch_off || !ch_row) { rc = EXR_ERROR_OUT_OF_MEMORY; goto done; }
    for (cc = 0; cc < nch; ++cc) {
        size_t ps = exr_pixel_size(h->channels[cc].pixel_type);
        ch_off[cc] = scan_stride;     /* byte offset of channel cc within a row */
        ch_row[cc] = (size_t)w * ps;  /* bytes per channel row */
        scan_stride += (size_t)w * ps;
        if (ch_row[cc] > maxplane) maxplane = ch_row[cc];
    }
    maxplane *= (size_t)hgt;

    rc = exr_reader_num_blocks(r, part, &nblocks);
    if (!EXR_OK(rc)) goto done;

    for (bi_idx = 0; bi_idx < nblocks; ++bi_idx) {
        exr_block_info bi;
        const uint8_t *cdata;
        size_t csize, unc;
        exr_codec_ctx ctx;
        int need_recon;
        int n_i;
        void *params[8];

        rc = exr_reader_block_raw(r, part, bi_idx, &bi, &cdata, &csize, &ctx);
        if (rc != EXR_SUCCESS) goto done; /* memory reader never WOULD_BLOCKs */
        unc = bi.uncompressed_size;
        n_i = (int)unc;

        /* (re)size device + host scratch to the largest block seen. */
        if (unc > maxblock) {
            d_free(d_a); d_free(d_b); exr_free(&c->alloc, host_canon);
            d_a = d_b = 0; host_canon = NULL;
            rc = d_alloc(&d_a, unc); if (!EXR_OK(rc)) goto done;
            rc = d_alloc(&d_b, unc); if (!EXR_OK(rc)) goto done;
            host_canon = (uint8_t *)exr_malloc(&c->alloc, unc ? unc : 1);
            if (!host_canon) { rc = EXR_ERROR_OUT_OF_MEMORY; goto done; }
            maxblock = unc;
        }
        if (maxplane > 0 && d_plane == 0) {
            rc = d_alloc(&d_plane, maxplane); if (!EXR_OK(rc)) goto done;
        }

        need_recon = (csize != unc) &&
                     (ctx.compression == EXR_COMPRESSION_ZIP ||
                      ctx.compression == EXR_COMPRESSION_ZIPS ||
                      ctx.compression == EXR_COMPRESSION_RLE);

        if (need_recon) {
            /* CPU entropy-only -> host_canon (pre-reconstruction bytes). */
            if (ctx.compression == EXR_COMPRESSION_RLE)
                rc = exr_rle_expand_only(cdata, csize, host_canon, unc);
            else
                rc = exr_zip_inflate_only(cdata, csize, host_canon, unc);
            if (!EXR_OK(rc)) goto done;
            rc = h2d(c, d_a, host_canon, unc); if (!EXR_OK(rc)) goto done;
            /* GPU predictor decode (d_a -> d_b), then deinterleave (d_b -> d_a) */
            rc = gpu_predictor_decode(c, d_a, n_i, d_b); if (!EXR_OK(rc)) goto done;
            { void *pp[3]; pp[0] = &d_b; pp[1] = &n_i; pp[2] = &d_a;
              rc = launch1d(c, c->k_deinterleave, n_i, pp); if (!EXR_OK(rc)) goto done; }
            /* canonical block now in d_a */
        } else if (ctx.compression == EXR_COMPRESSION_HTJ2K256 ||
                   ctx.compression == EXR_COMPRESSION_HTJ2K32) {
            /* HTJ2K: GPU HT block decode + CPU scatter/inverse-transform/store
             * produces the canonical block. Falls back to the CPU codec for
             * chunks with non-i32-eligible code-blocks (float/uint, hi-bitdepth). */
            rc = exr_jph_decompress_gpu(&ctx, cdata, csize, host_canon, unc,
                                        gpu_jph_decode_hook, c);
            if (rc == EXR_ERROR_UNSUPPORTED)
                rc = exr_decompress_block(&ctx, cdata, csize, host_canon, unc);
            if (!EXR_OK(rc)) goto done;
            rc = h2d(c, d_a, host_canon, unc); if (!EXR_OK(rc)) goto done;
        } else {
            /* canonical bytes via CPU (verbatim/NONE = copy; others decompress) */
            if (csize == unc || ctx.compression == EXR_COMPRESSION_NONE) {
                memcpy(host_canon, cdata, unc);
            } else {
                rc = exr_decompress_block(&ctx, cdata, csize, host_canon, unc);
                if (!EXR_OK(rc)) goto done;
            }
            rc = h2d(c, d_a, host_canon, unc); if (!EXR_OK(rc)) goto done;
        }

        /* GPU channel split: canonical (d_a) -> planar scratch -> D2H images[c] */
        for (cc = 0; cc < nch; ++cc) {
            int dStride = (int)ch_row[cc];
            int sStride = (int)scan_stride;
            int rowLen = (int)ch_row[cc];
            int rows = bi.height;
            size_t row_start = (size_t)(bi.y0 - dwy0);
            CUdeviceptr d_src = d_a + ch_off[cc];
            uint8_t *dst = (uint8_t *)out->images[cc] + row_start * ch_row[cc];
            params[0] = &d_plane; params[1] = &dStride; params[2] = &d_src;
            params[3] = &sStride; params[4] = &rowLen; params[5] = &rows;
            rc = launch1d(c, c->k_strided_copy, (long)rowLen * rows, params);
            if (!EXR_OK(rc)) goto done;
            rc = d2h(c, dst, d_plane, (size_t)rowLen * rows);
            if (!EXR_OK(rc)) goto done;
            rc = sync(c); if (!EXR_OK(rc)) goto done; /* d_plane reused next ch */
        }
    }
    rc = sync(c);
done:
    d_free(d_a); d_free(d_b); d_free(d_plane);
    exr_free(&c->alloc, host_canon);
    exr_free(&c->alloc, ch_off);
    exr_free(&c->alloc, ch_row);
    return rc;
}

static void copy_header(exr_header *dst, const exr_header *src) {
    *dst = *src;
    dst->channels = NULL;
    dst->attrs = NULL; /* GPU fast path does not round-trip custom attrs */
}

exr_result exr_gpu_load_from_memory(exr_gpu_context *c, const void *data,
                                    size_t size, const exr_allocator *alloc,
                                    exr_image *out) {
    exr_reader *r = NULL;
    exr_result rc;
    int np, p, eligible = 1;
    const exr_allocator *A;

    if (!out) return EXR_ERROR_INVALID_ARGUMENT;
    if (!c || !exr_gpu_available())
        return exr_load_from_memory(data, size, alloc, out);
    A = alloc ? alloc : exr_default_allocator();

    cuCtxSetCurrent(c->cuctx);
    rc = exr_reader_open_memory(data, size, alloc, &r);
    if (!EXR_OK(rc)) return rc;
    rc = exr_reader_parse_header(r);
    if (!EXR_OK(rc)) { exr_reader_close(r); return rc; }
    np = exr_reader_num_parts(r);
    for (p = 0; p < np; ++p) {
        if (!part_gpu_eligible(exr_reader_part_header(r, p))) { eligible = 0; break; }
    }
    if (!eligible || np <= 0) {
        exr_reader_close(r);
        return exr_load_from_memory(data, size, alloc, out);
    }

    memset(out, 0, sizeof(*out));
    if (A) out->alloc = *A;
    out->num_parts = np;
    out->parts = (exr_part *)exr_calloc(A, (size_t)np, sizeof(exr_part));
    if (!out->parts) { rc = EXR_ERROR_OUT_OF_MEMORY; goto fail; }

    for (p = 0; p < np; ++p) {
        const exr_header *h = exr_reader_part_header(r, p);
        exr_part *op = &out->parts[p];
        int nch = h->num_channels, cc;
        copy_header(&op->header, h);
        op->width = h->data_window.max_x - h->data_window.min_x + 1;
        op->height = h->data_window.max_y - h->data_window.min_y + 1;
        op->header.num_channels = nch;
        op->header.channels =
            (exr_channel *)exr_calloc(A, (size_t)nch, sizeof(exr_channel));
        op->images = (void **)exr_calloc(A, (size_t)nch, sizeof(void *));
        if (!op->header.channels || !op->images) {
            rc = EXR_ERROR_OUT_OF_MEMORY; goto fail;
        }
        for (cc = 0; cc < nch; ++cc) {
            size_t ps = exr_pixel_size(h->channels[cc].pixel_type);
            op->header.channels[cc] = h->channels[cc];
            op->images[cc] =
                exr_malloc(A, (size_t)op->width * op->height * ps + 1);
            if (!op->images[cc]) { rc = EXR_ERROR_OUT_OF_MEMORY; goto fail; }
        }
        rc = gpu_decode_part(c, r, p, op);
        if (!EXR_OK(rc)) goto fail;
    }
    exr_reader_close(r);
    return EXR_SUCCESS;
fail:
    exr_reader_close(r);
    exr_image_free(out);
    return rc;
}

exr_result exr_gpu_load_from_file(exr_gpu_context *c, const char *path,
                                  const exr_allocator *alloc, exr_image *out) {
    /* Read the whole file into memory then dispatch (the reader's zero-copy
     * memory path gives us stable compressed-chunk pointers for entropy-only). */
    FILE *f;
    long sz;
    void *buf;
    exr_result rc;
    const exr_allocator *A;
    if (!path || !out) return EXR_ERROR_INVALID_ARGUMENT;
    if (!c || !exr_gpu_available()) return exr_load_from_file(path, alloc, out);
    A = alloc ? alloc : exr_default_allocator();
    f = fopen(path, "rb");
    if (!f) return EXR_ERROR_IO;
    if (fseek(f, 0, SEEK_END) != 0) { fclose(f); return EXR_ERROR_IO; }
    sz = ftell(f);
    if (sz < 0) { fclose(f); return EXR_ERROR_IO; }
    rewind(f);
    buf = exr_malloc(A, (size_t)sz ? (size_t)sz : 1);
    if (!buf) { fclose(f); return EXR_ERROR_OUT_OF_MEMORY; }
    if (fread(buf, 1, (size_t)sz, f) != (size_t)sz) {
        fclose(f); exr_free(A, buf); return EXR_ERROR_IO;
    }
    fclose(f);
    rc = exr_gpu_load_from_memory(c, buf, (size_t)sz, alloc, out);
    exr_free(A, buf);
    return rc;
}

/* ===========================================================================
 * Hybrid encode (GPU gathers canonical scanline blocks; CPU entropy + assembly)
 * ========================================================================= */

static int image_gpu_eligible(const exr_image *img) {
    int p;
    if (img->num_parts <= 0) return 0;
    for (p = 0; p < img->num_parts; ++p) {
        if (img->parts[p].is_deep) return 0;
        if (!part_gpu_eligible(&img->parts[p].header)) return 0;
    }
    return 1;
}

/* GPU gather: planar channels (sorted order `order`) -> canonical scanline block
 * in device buffer d_block, then D2H to host_block. */
static exr_result gpu_gather_block(exr_gpu_context *c, const exr_part *pt,
                                   const int *order, int y0, int nlines,
                                   CUdeviceptr d_block, uint8_t *host_block,
                                   size_t blk_size,
                                   CUdeviceptr *d_chan, size_t *chan_bytes) {
    const exr_header *h = &pt->header;
    int w = pt->width, oi;
    int dwy0 = h->data_window.min_y;
    size_t scan_stride = 0, off;
    exr_result rc;
    /* scanline stride = sum of channel row bytes (sorted order). */
    for (oi = 0; oi < h->num_channels; ++oi) {
        int cc = order ? order[oi] : oi;
        scan_stride += (size_t)w * exr_pixel_size(h->channels[cc].pixel_type);
    }
    off = 0;
    for (oi = 0; oi < h->num_channels; ++oi) {
        int cc = order ? order[oi] : oi;
        size_t ps = exr_pixel_size(h->channels[cc].pixel_type);
        int rowLen = (int)((size_t)w * ps);
        int rows = nlines;
        int sStride = rowLen, dStride = (int)scan_stride;
        size_t row_start = (size_t)(y0 - dwy0);
        CUdeviceptr d_dst = d_block + off;
        CUdeviceptr d_src = d_chan[cc] + row_start * (size_t)rowLen;
        void *params[6];
        params[0] = &d_dst; params[1] = &dStride; params[2] = &d_src;
        params[3] = &sStride; params[4] = &rowLen; params[5] = &rows;
        rc = launch1d(c, c->k_strided_copy, (long)rowLen * rows, params);
        if (!EXR_OK(rc)) return rc;
        off += (size_t)rowLen; /* within-scanline channel offset (rows interleave
                                * via dStride = scan_stride) */
    }
    (void)chan_bytes;
    rc = d2h(c, host_block, d_block, blk_size);
    if (!EXR_OK(rc)) return rc;
    return sync(c);
}

/* ---- growable in-memory sink for the streaming writer ------------------- */
typedef struct {
    uint8_t *buf;
    size_t len, cap, pos;
    const exr_allocator *a;
    int err;
} mem_sink;

static exr_result mem_sink_write(void *user, const void *data, size_t n) {
    mem_sink *s = (mem_sink *)user;
    if (s->pos + n > s->cap) {
        size_t nc = s->cap ? s->cap * 2 : 65536;
        uint8_t *nb;
        while (nc < s->pos + n) nc *= 2;
        nb = (uint8_t *)exr_malloc(s->a, nc);
        if (!nb) { s->err = 1; return EXR_ERROR_OUT_OF_MEMORY; }
        if (s->buf) { memcpy(nb, s->buf, s->len); exr_free(s->a, s->buf); }
        s->buf = nb; s->cap = nc;
    }
    memcpy(s->buf + s->pos, data, n);
    s->pos += n;
    if (s->pos > s->len) s->len = s->pos;
    return EXR_SUCCESS;
}
static exr_result mem_sink_seek(void *user, uint64_t off) {
    mem_sink *s = (mem_sink *)user;
    s->pos = (size_t)off;
    return EXR_SUCCESS;
}

/* shared per-part GPU gather + canonical write loop (file/memory streams) */
static exr_result gpu_encode_parts(exr_gpu_context *c, exr_writer *w,
                                   const exr_image *img, exr_compression comp) {
    int np = img->num_parts, p;
    exr_result rc = EXR_SUCCESS;
    for (p = 0; p < np; ++p) {
        const exr_part *pt = &img->parts[p];
        const exr_header *h = &pt->header;
        const int *order = exr_writer_sorted_order(w, p);
        int nch = h->num_channels, cc;
        int lpb = exr_lines_per_block(comp);
        int ymin = h->data_window.min_y, ymax = h->data_window.max_y, y0;
        CUdeviceptr *d_chan = (CUdeviceptr *)exr_calloc(&c->alloc, (size_t)nch,
                                                        sizeof(CUdeviceptr));
        CUdeviceptr d_block = 0;
        uint8_t *host_block = NULL;
        size_t maxblk = 0;
        if (!d_chan) return EXR_ERROR_OUT_OF_MEMORY;
        for (cc = 0; cc < nch; ++cc) {
            size_t ps = exr_pixel_size(h->channels[cc].pixel_type);
            size_t bytes = (size_t)pt->width * pt->height * ps;
            rc = d_alloc(&d_chan[cc], bytes); if (!EXR_OK(rc)) goto cleanp;
            rc = h2d(c, d_chan[cc], pt->images[cc], bytes); if (!EXR_OK(rc)) goto cleanp;
        }
        rc = sync(c); if (!EXR_OK(rc)) goto cleanp;
        for (y0 = ymin; y0 <= ymax; y0 += lpb) {
            int nlines = (y0 + lpb - 1 > ymax) ? (ymax - y0 + 1) : lpb;
            size_t blk_size;
            rc = exr_block_uncompressed_size(h->channels, nch, h->data_window.min_x,
                                             y0, pt->width, nlines, &blk_size);
            if (!EXR_OK(rc)) goto cleanp;
            if (blk_size > maxblk) {
                d_free(d_block); exr_free(&c->alloc, host_block);
                d_block = 0; host_block = NULL;
                rc = d_alloc(&d_block, blk_size); if (!EXR_OK(rc)) goto cleanp;
                host_block = (uint8_t *)exr_malloc(&c->alloc, blk_size ? blk_size : 1);
                if (!host_block) { rc = EXR_ERROR_OUT_OF_MEMORY; goto cleanp; }
                maxblk = blk_size;
            }
            rc = gpu_gather_block(c, pt, order, y0, nlines, d_block, host_block,
                                  blk_size, d_chan, NULL);
            if (!EXR_OK(rc)) goto cleanp;
            rc = exr_writer_write_scanline_block_canon(w, p, y0, host_block, blk_size);
            if (!EXR_OK(rc)) goto cleanp;
        }
    cleanp:
        d_free(d_block);
        exr_free(&c->alloc, host_block);
        if (d_chan) { for (cc = 0; cc < nch; ++cc) d_free(d_chan[cc]);
                      exr_free(&c->alloc, d_chan); }
        if (!EXR_OK(rc)) return rc;
    }
    return EXR_SUCCESS;
}

exr_result exr_gpu_save_to_memory(exr_gpu_context *c, void **out_data,
                                  size_t *out_size, const exr_allocator *alloc,
                                  const exr_image *img, exr_compression comp) {
    exr_writer *w = NULL;
    exr_result rc;
    int np, p;
    mem_sink ms;
    exr_data_sink sink;
    const exr_allocator *A;

    if (!out_data || !out_size || !img) return EXR_ERROR_INVALID_ARGUMENT;
    if (!c || !exr_gpu_available() || !image_gpu_eligible(img))
        return exr_save_to_memory(out_data, out_size, alloc, img, comp);

    A = alloc ? alloc : exr_default_allocator();
    cuCtxSetCurrent(c->cuctx);
    np = img->num_parts;
    memset(&ms, 0, sizeof(ms));
    ms.a = A;
    sink.user = &ms;
    sink.write = mem_sink_write;
    sink.seek = mem_sink_seek;
    sink.close = NULL;

    rc = exr_writer_create(alloc, &w);
    if (!EXR_OK(rc)) return rc;
    for (p = 0; p < np; ++p) {
        rc = exr_writer_add_part(w, &img->parts[p].header, NULL);
        if (!EXR_OK(rc)) goto done;
    }
    rc = exr_writer_begin_stream(w, &sink, comp);
    if (!EXR_OK(rc)) goto done;
    exr_writer_set_gpu_jph_encoder(w, gpu_jph_encode_hook, c);
    rc = gpu_encode_parts(c, w, img, comp);
    if (!EXR_OK(rc)) goto done;
    rc = exr_writer_end_stream(w);
    if (!EXR_OK(rc)) goto done;
    if (ms.err) { rc = EXR_ERROR_OUT_OF_MEMORY; goto done; }
    *out_data = ms.buf;
    *out_size = ms.len;
    ms.buf = NULL;
done:
    exr_writer_destroy(w);
    if (ms.buf) exr_free(A, ms.buf);
    return rc;
}

exr_result exr_gpu_save_to_file(exr_gpu_context *c, const char *path,
                                const exr_image *img, exr_compression comp) {
    exr_writer *w = NULL;
    exr_result rc;
    int np, p;

    if (!path || !img) return EXR_ERROR_INVALID_ARGUMENT;
    if (!c || !exr_gpu_available() || !image_gpu_eligible(img))
        return exr_save_to_file(path, img, comp);

    cuCtxSetCurrent(c->cuctx);
    np = img->num_parts;
    rc = exr_writer_create(NULL, &w);
    if (!EXR_OK(rc)) return rc;
    for (p = 0; p < np; ++p) {
        rc = exr_writer_add_part(w, &img->parts[p].header, NULL);
        if (!EXR_OK(rc)) { exr_writer_destroy(w); return rc; }
    }
    rc = exr_writer_begin_stream_file(w, path, comp);
    if (!EXR_OK(rc)) { exr_writer_destroy(w); return rc; }
    exr_writer_set_gpu_jph_encoder(w, gpu_jph_encode_hook, c);
    rc = gpu_encode_parts(c, w, img, comp);
    if (EXR_OK(rc)) rc = exr_writer_end_stream(w);
    exr_writer_destroy(w);
    return rc;
}

/* ===========================================================================
 * Processing: simple H2D -> launch -> D2H wrappers
 * ========================================================================= */

/* Element-wise float in/out helper: upload n floats, run `fn` over n, download. */
static exr_result run_unary_f32(exr_gpu_context *c, CUfunction fn, float *dst,
                                const float *src, long n, int extra_int,
                                int extra_int2) {
    CUdeviceptr d_in = 0, d_out = 0;
    exr_result rc;
    void *params[5];
    int n32 = (int)n;
    rc = d_alloc(&d_in, (size_t)n * sizeof(float)); if (!EXR_OK(rc)) goto done;
    rc = d_alloc(&d_out, (size_t)n * sizeof(float)); if (!EXR_OK(rc)) goto done;
    rc = h2d(c, d_in, src, (size_t)n * sizeof(float)); if (!EXR_OK(rc)) goto done;
    params[0] = &d_out; params[1] = &d_in; params[2] = &n; /* note: long */
    params[3] = &extra_int; params[4] = &extra_int2;
    (void)n32;
    rc = launch1d(c, fn, n, params); if (!EXR_OK(rc)) goto done;
    rc = d2h(c, dst, d_out, (size_t)n * sizeof(float)); if (!EXR_OK(rc)) goto done;
    rc = sync(c);
done:
    d_free(d_in); d_free(d_out);
    return rc;
}

exr_result exr_gpu_encode_transfer(exr_gpu_context *c, float *dst,
                                   const float *src, size_t n, exr_transfer tf) {
    int tfi = (int)tf, dec = 0;
    if (!c || !exr_gpu_available()) return exr_encode_transfer(dst, src, n, tf);
    cuCtxSetCurrent(c->cuctx);
    return run_unary_f32(c, c->k_transfer, dst, src, (long)n, tfi, dec);
}
exr_result exr_gpu_decode_transfer(exr_gpu_context *c, float *dst,
                                   const float *src, size_t n, exr_transfer tf) {
    int tfi = (int)tf, dec = 1;
    if (!c || !exr_gpu_available()) return exr_decode_transfer(dst, src, n, tf);
    cuCtxSetCurrent(c->cuctx);
    return run_unary_f32(c, c->k_transfer, dst, src, (long)n, tfi, dec);
}

exr_result exr_gpu_color_apply_matrix(exr_gpu_context *c, float *dst,
                                      const float *src, size_t pixels, int ch,
                                      const float m[9]) {
    CUdeviceptr d_in = 0, d_out = 0;
    exr_result rc;
    long n = (long)pixels;
    float mm[9];
    void *params[13];
    int i;
    if (!c || !exr_gpu_available())
        return exr_color_apply_matrix(dst, src, pixels, ch, m);
    cuCtxSetCurrent(c->cuctx);
    for (i = 0; i < 9; ++i) mm[i] = m[i];
    rc = d_alloc(&d_in, pixels * (size_t)ch * sizeof(float)); if (!EXR_OK(rc)) goto done;
    rc = d_alloc(&d_out, pixels * (size_t)ch * sizeof(float)); if (!EXR_OK(rc)) goto done;
    rc = h2d(c, d_in, src, pixels * (size_t)ch * sizeof(float)); if (!EXR_OK(rc)) goto done;
    params[0] = &d_out; params[1] = &d_in; params[2] = &n; params[3] = &ch;
    for (i = 0; i < 9; ++i) params[4 + i] = &mm[i];
    rc = launch1d(c, c->k_color_matrix, n, params); if (!EXR_OK(rc)) goto done;
    rc = d2h(c, dst, d_out, pixels * (size_t)ch * sizeof(float)); if (!EXR_OK(rc)) goto done;
    rc = sync(c);
done:
    d_free(d_in); d_free(d_out);
    return rc;
}

exr_result exr_gpu_tonemap_float(exr_gpu_context *c, float *dst, const float *src,
                                 size_t pixels, int ch, exr_tonemap_op op,
                                 const exr_tonemap_params *p) {
    CUdeviceptr d_in = 0, d_out = 0;
    exr_result rc;
    long n = (long)pixels;
    int opi = (int)op;
    float exposure = (p && p->exposure != 0.0f) ? p->exposure : 1.0f;
    float white = p ? p->white_point : 0.0f;
    void *params[7];
    if (!c || !exr_gpu_available())
        return exr_tonemap_float(dst, src, pixels, ch, op, p);
    cuCtxSetCurrent(c->cuctx);
    rc = d_alloc(&d_in, pixels * (size_t)ch * sizeof(float)); if (!EXR_OK(rc)) goto done;
    rc = d_alloc(&d_out, pixels * (size_t)ch * sizeof(float)); if (!EXR_OK(rc)) goto done;
    rc = h2d(c, d_in, src, pixels * (size_t)ch * sizeof(float)); if (!EXR_OK(rc)) goto done;
    params[0] = &d_out; params[1] = &d_in; params[2] = &n; params[3] = &ch;
    params[4] = &opi; params[5] = &exposure; params[6] = &white;
    rc = launch1d(c, c->k_tonemap, n, params); if (!EXR_OK(rc)) goto done;
    rc = d2h(c, dst, d_out, pixels * (size_t)ch * sizeof(float)); if (!EXR_OK(rc)) goto done;
    rc = sync(c);
done:
    d_free(d_in); d_free(d_out);
    return rc;
}

exr_result exr_gpu_lut3d_apply(exr_gpu_context *c, float *dst, const float *src,
                               size_t pixels, int ch, const exr_lut3d *lut,
                               exr_lut_interp interp) {
    CUdeviceptr d_in = 0, d_out = 0, d_lut = 0;
    exr_result rc;
    long n = (long)pixels;
    int sz;
    float dmn0, dmn1, dmn2, dmx0, dmx1, dmx2;
    void *params[12];
    if (!c || !exr_gpu_available())
        return exr_lut3d_apply(dst, src, pixels, ch, lut, interp);
    if (!lut || lut->size < 2) return EXR_ERROR_INVALID_ARGUMENT;
    cuCtxSetCurrent(c->cuctx);
    sz = lut->size;
    dmn0 = lut->domain_min[0]; dmn1 = lut->domain_min[1]; dmn2 = lut->domain_min[2];
    dmx0 = lut->domain_max[0]; dmx1 = lut->domain_max[1]; dmx2 = lut->domain_max[2];
    rc = d_alloc(&d_in, pixels * (size_t)ch * sizeof(float)); if (!EXR_OK(rc)) goto done;
    rc = d_alloc(&d_out, pixels * (size_t)ch * sizeof(float)); if (!EXR_OK(rc)) goto done;
    rc = d_alloc(&d_lut, (size_t)sz * sz * sz * 3 * sizeof(float)); if (!EXR_OK(rc)) goto done;
    rc = h2d(c, d_in, src, pixels * (size_t)ch * sizeof(float)); if (!EXR_OK(rc)) goto done;
    rc = h2d(c, d_lut, lut->data, (size_t)sz * sz * sz * 3 * sizeof(float)); if (!EXR_OK(rc)) goto done;
    params[0] = &d_out; params[1] = &d_in; params[2] = &n; params[3] = &ch;
    params[4] = &d_lut; params[5] = &sz;
    params[6] = &dmn0; params[7] = &dmn1; params[8] = &dmn2;
    params[9] = &dmx0; params[10] = &dmx1; params[11] = &dmx2;
    rc = launch1d(c, c->k_lut3d, n, params); if (!EXR_OK(rc)) goto done;
    rc = d2h(c, dst, d_out, pixels * (size_t)ch * sizeof(float)); if (!EXR_OK(rc)) goto done;
    rc = sync(c);
done:
    d_free(d_in); d_free(d_out); d_free(d_lut);
    return rc;
}

exr_result exr_gpu_convert_pixels(exr_gpu_context *c, void *dst,
                                  exr_pixel_type dt, const void *src,
                                  exr_pixel_type st, size_t n,
                                  exr_convert_mode mode) {
    CUdeviceptr d_in = 0, d_out = 0;
    exr_result rc;
    long nn = (long)n;
    int dti = (int)dt, sti = (int)st, norm = (mode == EXR_CONVERT_NORMALIZED);
    size_t sps = exr_pixel_size(st), dps = exr_pixel_size(dt);
    void *params[6];
    if (!c || !exr_gpu_available())
        return exr_convert_pixels(dst, dt, src, st, n, mode);
    cuCtxSetCurrent(c->cuctx);
    rc = d_alloc(&d_in, n * sps); if (!EXR_OK(rc)) goto done;
    rc = d_alloc(&d_out, n * dps); if (!EXR_OK(rc)) goto done;
    rc = h2d(c, d_in, src, n * sps); if (!EXR_OK(rc)) goto done;
    params[0] = &d_out; params[1] = &dti; params[2] = &d_in; params[3] = &sti;
    params[4] = &nn; params[5] = &norm;
    rc = launch1d(c, c->k_convert, nn, params); if (!EXR_OK(rc)) goto done;
    rc = d2h(c, dst, d_out, n * dps); if (!EXR_OK(rc)) goto done;
    rc = sync(c);
done:
    d_free(d_in); d_free(d_out);
    return rc;
}

/* ---- channel gather / scatter ------------------------------------------- */
exr_result exr_gpu_part_to_rgba_float(exr_gpu_context *c, const exr_allocator *a,
                                      const exr_part *part, float **out, int *ow,
                                      int *oh, int *oc) {
    const exr_header *h;
    int nch, outc, cc;
    long n;
    float *host;
    CUdeviceptr d_out = 0, d_in = 0;
    exr_result rc = EXR_SUCCESS;
    if (!c || !exr_gpu_available())
        return exr_part_to_rgba_float(a, part, out, ow, oh, oc);
    if (!part || !out) return EXR_ERROR_INVALID_ARGUMENT;
    h = &part->header;
    if (!part_gpu_eligible(h)) return exr_part_to_rgba_float(a, part, out, ow, oh, oc);
    a = a ? a : exr_default_allocator();
    cuCtxSetCurrent(c->cuctx);
    nch = h->num_channels;
    outc = nch < 4 ? nch : 4;
    n = (long)part->width * part->height;
    host = (float *)exr_malloc(a, (size_t)n * outc * sizeof(float) + 1);
    if (!host) return EXR_ERROR_OUT_OF_MEMORY;
    rc = d_alloc(&d_out, (size_t)n * outc * sizeof(float)); if (!EXR_OK(rc)) goto done;
    for (cc = 0; cc < outc; ++cc) {
        size_t ps = exr_pixel_size(h->channels[cc].pixel_type);
        int stype = (int)h->channels[cc].pixel_type;
        void *params[6];
        rc = d_alloc(&d_in, (size_t)n * ps); if (!EXR_OK(rc)) goto done;
        rc = h2d(c, d_in, part->images[cc], (size_t)n * ps); if (!EXR_OK(rc)) goto done;
        params[0] = &d_out; params[1] = &outc; params[2] = &cc; params[3] = &d_in;
        params[4] = &stype; params[5] = &n;
        rc = launch1d(c, c->k_gather_f32, n, params); if (!EXR_OK(rc)) { goto done; }
        rc = sync(c); if (!EXR_OK(rc)) goto done;
        d_free(d_in); d_in = 0;
    }
    rc = d2h(c, host, d_out, (size_t)n * outc * sizeof(float)); if (!EXR_OK(rc)) goto done;
    rc = sync(c);
done:
    d_free(d_out); d_free(d_in);
    if (EXR_OK(rc)) {
        *out = host;
        if (ow) *ow = part->width;
        if (oh) *oh = part->height;
        if (oc) *oc = outc;
    } else {
        exr_free(a, host);
    }
    return rc;
}

exr_result exr_gpu_rgba_float_to_part(exr_gpu_context *c, const exr_allocator *a,
                                      const float *rgba, int width, int height,
                                      int channels, exr_pixel_type dt,
                                      exr_part *out) {
    static const char *names[4] = {"R", "G", "B", "A"};
    long n = (long)width * height;
    int cc;
    size_t dps;
    CUdeviceptr d_in = 0, d_out = 0;
    exr_result rc;
    if (!c || !exr_gpu_available())
        return exr_rgba_float_to_part(a, rgba, width, height, channels, dt, out);
    if (!rgba || !out || channels < 1 || channels > 4)
        return EXR_ERROR_INVALID_ARGUMENT;
    a = a ? a : exr_default_allocator();
    cuCtxSetCurrent(c->cuctx);
    dps = exr_pixel_size(dt);

    memset(out, 0, sizeof(*out));
    out->width = width;
    out->height = height;
    out->header.num_channels = channels;
    out->header.compression = EXR_COMPRESSION_ZIP;
    out->header.part_type = EXR_PART_SCANLINE;
    out->header.line_order = EXR_LINEORDER_INCREASING_Y;
    out->header.data_window.min_x = 0; out->header.data_window.min_y = 0;
    out->header.data_window.max_x = width - 1;
    out->header.data_window.max_y = height - 1;
    out->header.display_window = out->header.data_window;
    out->header.pixel_aspect_ratio = 1.0f;
    out->header.screen_window_width = 1.0f;
    out->header.channels = (exr_channel *)exr_calloc(a, (size_t)channels,
                                                     sizeof(exr_channel));
    out->images = (void **)exr_calloc(a, (size_t)channels, sizeof(void *));
    if (!out->header.channels || !out->images) { rc = EXR_ERROR_OUT_OF_MEMORY; goto fail; }

    rc = d_alloc(&d_in, (size_t)n * channels * sizeof(float)); if (!EXR_OK(rc)) goto fail;
    rc = h2d(c, d_in, rgba, (size_t)n * channels * sizeof(float)); if (!EXR_OK(rc)) goto fail;
    rc = d_alloc(&d_out, (size_t)n * dps); if (!EXR_OK(rc)) goto fail;

    /* channels named R,G,B,A in input order; header.channels must be sorted by
     * name, so place each at its sorted slot. */
    for (cc = 0; cc < channels; ++cc) {
        /* compute sorted index of names[cc] among the first `channels` names */
        int sidx = 0, k;
        for (k = 0; k < channels; ++k)
            if (k != cc && strcmp(names[k], names[cc]) < 0) sidx++;
        snprintf(out->header.channels[sidx].name,
                 sizeof(out->header.channels[sidx].name), "%s", names[cc]);
        out->header.channels[sidx].pixel_type = dt;
        out->header.channels[sidx].x_sampling = 1;
        out->header.channels[sidx].y_sampling = 1;
        out->images[sidx] = exr_malloc(a, (size_t)n * dps + 1);
        if (!out->images[sidx]) { rc = EXR_ERROR_OUT_OF_MEMORY; goto fail; }
        {
            int dti = (int)dt, inCh = channels, csrc = cc;
            void *params[6];
            params[0] = &d_out; params[1] = &dti; params[2] = &d_in;
            params[3] = &inCh; params[4] = &csrc; params[5] = &n;
            rc = launch1d(c, c->k_scatter_f32, n, params); if (!EXR_OK(rc)) goto fail;
            rc = d2h(c, out->images[sidx], d_out, (size_t)n * dps); if (!EXR_OK(rc)) goto fail;
            rc = sync(c); if (!EXR_OK(rc)) goto fail;
        }
    }
    d_free(d_in); d_free(d_out);
    return EXR_SUCCESS;
fail:
    d_free(d_in); d_free(d_out);
    exr_part_free(a, out);
    return rc;
}

/* ---- resize ------------------------------------------------------------- */
static float rk_cubic(float a, float B, float C) {
    if (a < 1.0f)
        return ((12.0f - 9.0f * B - 6.0f * C) * a * a * a +
                (-18.0f + 12.0f * B + 6.0f * C) * a * a + (6.0f - 2.0f * B)) / 6.0f;
    if (a < 2.0f)
        return ((-B - 6.0f * C) * a * a * a + (6.0f * B + 30.0f * C) * a * a +
                (-12.0f * B - 48.0f * C) * a + (8.0f * B + 24.0f * C)) / 6.0f;
    return 0.0f;
}
static float rk_weight(exr_resize_filter f, float t) {
    float a = t < 0.0f ? -t : t;
    switch (f) {
        case EXR_RESIZE_BOX: return a <= 0.5f ? 1.0f : 0.0f;
        case EXR_RESIZE_TRIANGLE: return a < 1.0f ? 1.0f - a : 0.0f;
        case EXR_RESIZE_CATMULL_ROM: return rk_cubic(a, 0.0f, 0.5f);
        default: return rk_cubic(a, 1.0f / 3.0f, 1.0f / 3.0f);
    }
}
static float rk_radius(exr_resize_filter f) {
    if (f == EXR_RESIZE_BOX) return 0.5f;
    if (f == EXR_RESIZE_TRIANGLE) return 1.0f;
    return 2.0f;
}
static int rk_edge(int j, int n, exr_edge_mode m) {
    if (n == 1) return 0;
    if (j >= 0 && j < n) return j;
    if (m == EXR_EDGE_CLAMP) return j < 0 ? 0 : n - 1;
    if (m == EXR_EDGE_WRAP) { j %= n; if (j < 0) j += n; return j; }
    { int p = 2 * n - 2; j %= p; if (j < 0) j += p; return j < n ? j : p - j; }
}
static int rk_ifloor(float x) { int i = (int)x; return ((float)i > x) ? i - 1 : i; }
static int rk_iceil(float x) { int i = (int)x; return ((float)i < x) ? i + 1 : i; }

/* Build a dense [d * support] weight/index table for one axis (resampling s->d).
 * support is fixed at the maximum contributor count; shorter taps are zero. */
static exr_result build_axis(const exr_allocator *a, int s, int d,
                             exr_resize_filter filt, exr_edge_mode edge,
                             float **out_w, int **out_i, int *out_support) {
    float fscale = (d < s) ? (float)s / (float)d : 1.0f;
    float support = rk_radius(filt) * fscale;
    float invscale = 1.0f / fscale;
    int sup = (int)(2.0f * support) + 4; /* generous upper bound on tap count */
    float *W;
    int *I;
    int i, k;
    W = (float *)exr_calloc(a, (size_t)d * sup, sizeof(float));
    I = (int *)exr_calloc(a, (size_t)d * sup, sizeof(int));
    if (!W || !I) { exr_free(a, W); exr_free(a, I); return EXR_ERROR_OUT_OF_MEMORY; }
    for (i = 0; i < d; ++i) {
        float center = ((float)i + 0.5f) * ((float)s / (float)d) - 0.5f;
        int lo = rk_iceil(center - support), hi = rk_ifloor(center + support), t;
        float sum = 0.0f;
        int idx = 0;
        float *w = W + (size_t)i * sup;
        int *ix = I + (size_t)i * sup;
        if (hi < lo) hi = lo;
        for (t = lo; t <= hi && idx < sup; ++t) {
            int mi = rk_edge(t, s, edge);
            float wt = rk_weight(filt, (center - (float)t) * invscale);
            ix[idx] = mi; w[idx] = wt; sum += wt; idx++;
        }
        for (; idx < sup; ++idx) { ix[idx] = (lo >= 0 && lo < s) ? lo : 0; w[idx] = 0.0f; }
        if (sum == 0.0f) { w[0] = 1.0f; }
        else { float inv = 1.0f / sum; for (k = 0; k < sup; ++k) w[k] *= inv; }
    }
    *out_w = W; *out_i = I; *out_support = sup;
    return EXR_SUCCESS;
}

exr_result exr_gpu_resize_float(exr_gpu_context *c, const float *src, int sw,
                                int sh, size_t ss, float *dst, int dw, int dh,
                                size_t ds, int ch, exr_resize_filter filter,
                                exr_edge_mode edge, int alpha_channel) {
    const exr_allocator *a;
    float *tight_src = NULL, *tight_dst = NULL;
    float *wx = NULL, *wy = NULL;
    int *ix = NULL, *iy = NULL;
    int supx = 0, supy = 0;
    CUdeviceptr d_src = 0, d_tmp = 0, d_dst = 0;
    CUdeviceptr d_wx = 0, d_ix = 0, d_wy = 0, d_iy = 0;
    exr_result rc;
    long y;
    int sstride_e = sw * ch, dstride_e = dw * ch, tmpstride_e = dw * ch;

    if (!c || !exr_gpu_available())
        return exr_resize_float(NULL, src, sw, sh, ss, dst, dw, dh, ds, ch, filter,
                                edge, alpha_channel);
    if (!src || !dst || sw <= 0 || sh <= 0 || dw <= 0 || dh <= 0 || ch < 1 || ch > 4)
        return EXR_ERROR_INVALID_ARGUMENT;
    a = &c->alloc;
    cuCtxSetCurrent(c->cuctx);
    if (ss == 0) ss = (size_t)sw * ch;
    if (ds == 0) ds = (size_t)dw * ch;

    /* tight host copies (kernels assume tight element strides) */
    tight_src = (float *)exr_malloc(a, (size_t)sw * sh * ch * sizeof(float) + 1);
    tight_dst = (float *)exr_malloc(a, (size_t)dw * dh * ch * sizeof(float) + 1);
    if (!tight_src || !tight_dst) { rc = EXR_ERROR_OUT_OF_MEMORY; goto done; }
    for (y = 0; y < sh; ++y)
        memcpy(tight_src + y * sstride_e, src + (size_t)y * ss,
               (size_t)sstride_e * sizeof(float));

    rc = build_axis(a, sw, dw, filter, edge, &wx, &ix, &supx); if (!EXR_OK(rc)) goto done;
    rc = build_axis(a, sh, dh, filter, edge, &wy, &iy, &supy); if (!EXR_OK(rc)) goto done;

    rc = d_alloc(&d_src, (size_t)sw * sh * ch * sizeof(float)); if (!EXR_OK(rc)) goto done;
    rc = d_alloc(&d_tmp, (size_t)dw * sh * ch * sizeof(float)); if (!EXR_OK(rc)) goto done;
    rc = d_alloc(&d_dst, (size_t)dw * dh * ch * sizeof(float)); if (!EXR_OK(rc)) goto done;
    rc = d_alloc(&d_wx, (size_t)dw * supx * sizeof(float)); if (!EXR_OK(rc)) goto done;
    rc = d_alloc(&d_ix, (size_t)dw * supx * sizeof(int)); if (!EXR_OK(rc)) goto done;
    rc = d_alloc(&d_wy, (size_t)dh * supy * sizeof(float)); if (!EXR_OK(rc)) goto done;
    rc = d_alloc(&d_iy, (size_t)dh * supy * sizeof(int)); if (!EXR_OK(rc)) goto done;

    rc = h2d(c, d_src, tight_src, (size_t)sw * sh * ch * sizeof(float)); if (!EXR_OK(rc)) goto done;
    rc = h2d(c, d_wx, wx, (size_t)dw * supx * sizeof(float)); if (!EXR_OK(rc)) goto done;
    rc = h2d(c, d_ix, ix, (size_t)dw * supx * sizeof(int)); if (!EXR_OK(rc)) goto done;
    rc = h2d(c, d_wy, wy, (size_t)dh * supy * sizeof(float)); if (!EXR_OK(rc)) goto done;
    rc = h2d(c, d_iy, iy, (size_t)dh * supy * sizeof(int)); if (!EXR_OK(rc)) goto done;

    if (alpha_channel >= 0 && alpha_channel < ch) {
        long n = (long)sw * sh; int chh = ch, ac = alpha_channel, undo = 0;
        void *pp[5]; pp[0] = &d_src; pp[1] = &n; pp[2] = &chh; pp[3] = &ac; pp[4] = &undo;
        rc = launch1d(c, c->k_premult, n, pp); if (!EXR_OK(rc)) goto done;
    }
    /* horizontal: d_src(sw x sh) -> d_tmp(dw x sh) */
    { void *p[11]; p[0] = &d_src; p[1] = &sw; p[2] = &sh; p[3] = &sstride_e;
      p[4] = &d_tmp; p[5] = &dw; p[6] = &tmpstride_e; p[7] = &ch; p[8] = &d_wx;
      p[9] = &d_ix; p[10] = &supx;
      rc = launch1d(c, c->k_resize_h, (long)dw * sh, p); if (!EXR_OK(rc)) goto done; }
    /* vertical: d_tmp(dw x sh) -> d_dst(dw x dh) */
    { void *p[12]; p[0] = &d_tmp; p[1] = &dw; p[2] = &sh; p[3] = &tmpstride_e;
      p[4] = &d_dst; p[5] = &dw; p[6] = &dh; p[7] = &dstride_e; p[8] = &ch;
      p[9] = &d_wy; p[10] = &d_iy; p[11] = &supy;
      rc = launch1d(c, c->k_resize_v, (long)dw * dh, p); if (!EXR_OK(rc)) goto done; }
    if (alpha_channel >= 0 && alpha_channel < ch) {
        long n = (long)dw * dh; int chh = ch, ac = alpha_channel, undo = 1;
        void *pp[5]; pp[0] = &d_dst; pp[1] = &n; pp[2] = &chh; pp[3] = &ac; pp[4] = &undo;
        rc = launch1d(c, c->k_premult, n, pp); if (!EXR_OK(rc)) goto done;
    }

    rc = d2h(c, tight_dst, d_dst, (size_t)dw * dh * ch * sizeof(float)); if (!EXR_OK(rc)) goto done;
    rc = sync(c); if (!EXR_OK(rc)) goto done;
    for (y = 0; y < dh; ++y)
        memcpy(dst + (size_t)y * ds, tight_dst + y * dstride_e,
               (size_t)dstride_e * sizeof(float));
done:
    d_free(d_src); d_free(d_tmp); d_free(d_dst);
    d_free(d_wx); d_free(d_ix); d_free(d_wy); d_free(d_iy);
    exr_free(a, tight_src); exr_free(a, tight_dst);
    exr_free(a, wx); exr_free(a, ix); exr_free(a, wy); exr_free(a, iy);
    return rc;
}

#endif /* EXR_USE_CUDA */
