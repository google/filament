/*
 * TinyEXR - internal shared declarations (not installed).
 *
 * Copyright (c) 2014-2026 Syoyo Fujita and TinyEXR authors
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef TINYEXR_INTERNAL_H_
#define TINYEXR_INTERNAL_H_

#include "exr.h"

#include <stddef.h>
#include <stdint.h>

/*
 * Freestanding builds (EXR_FREESTANDING) must not pull hosted <string.h>. We
 * declare the handful of mem/str symbols we use (and that the compiler may
 * emit) and define them in src/exr_freestanding.c. Hosted builds use the libc
 * versions as before; no call sites change either way.
 */
#ifdef EXR_FREESTANDING
void *memcpy(void *dst, const void *src, size_t n);
void *memmove(void *dst, const void *src, size_t n);
void *memset(void *dst, int c, size_t n);
int memcmp(const void *a, const void *b, size_t n);
size_t strlen(const char *s);
int strcmp(const char *a, const char *b);
#else
#include <string.h>
#endif

/* ============================================================================
 * EXR format constants
 * ========================================================================== */

#define EXR_MAGIC 20000630 /* 0x01312f76 */
#define EXR_VERSION_NUMBER 2

/* Version field flag bits (byte 1..3 of the 4-byte version word). */
#define EXR_VERSION_FLAG_TILED (1 << 9)
#define EXR_VERSION_FLAG_LONG_NAMES (1 << 10)
#define EXR_VERSION_FLAG_NON_IMAGE (1 << 11) /* deep data present */
#define EXR_VERSION_FLAG_MULTIPART (1 << 12)

/* Hard safety limits to bound allocations from hostile files. */
#define EXR_MAX_PARTS 1048576
#define EXR_MAX_CHANNELS 1048576
#define EXR_MAX_ATTRIBUTES 1048576
#define EXR_MAX_ATTR_SIZE (256u * 1024u * 1024u)
#define EXR_MAX_DIMENSION (1 << 20) /* 1,048,576 px per axis */

/* ============================================================================
 * Checked integer arithmetic (size_t domain)
 * ========================================================================== */

static inline int exr_mul_ovf(size_t a, size_t b, size_t *out) {
#if defined(__GNUC__) || defined(__clang__)
    return __builtin_mul_overflow(a, b, out);
#else
    if (a != 0 && b > SIZE_MAX / a) return 1;
    *out = a * b;
    return 0;
#endif
}

static inline int exr_add_ovf(size_t a, size_t b, size_t *out) {
#if defined(__GNUC__) || defined(__clang__)
    return __builtin_add_overflow(a, b, out);
#else
    if (b > SIZE_MAX - a) return 1;
    *out = a + b;
    return 0;
#endif
}

/* Function attribute that suppresses UBSan's signed-integer-overflow check.
 * The reversible 5/3 wavelet lifting steps are defined to wrap modulo 2^64
 * (two's complement); this is exact for real coefficient ranges and only trips
 * the sanitizer on the synthetic random-int64 parity inputs. The attribute
 * changes no generated code, so annotated transforms stay bit-identical; other
 * UBSan checks (bounds, null, etc.) remain active inside the function. */
#if defined(__clang__) || (defined(__GNUC__) && __GNUC__ >= 8)
#define EXR_NO_SANITIZE_SIO __attribute__((no_sanitize("signed-integer-overflow")))
#else
#define EXR_NO_SANITIZE_SIO
#endif

/* ============================================================================
 * Little-endian readers / writers (EXR is always little-endian on disk)
 * ========================================================================== */

static inline uint16_t exr_rd_u16(const uint8_t *p) {
    return (uint16_t)(p[0] | ((uint16_t)p[1] << 8));
}
static inline uint32_t exr_rd_u32(const uint8_t *p) {
    return (uint32_t)p[0] | ((uint32_t)p[1] << 8) | ((uint32_t)p[2] << 16) |
           ((uint32_t)p[3] << 24);
}
static inline int32_t exr_rd_i32(const uint8_t *p) {
    return (int32_t)exr_rd_u32(p);
}
static inline uint64_t exr_rd_u64(const uint8_t *p) {
    return (uint64_t)exr_rd_u32(p) | ((uint64_t)exr_rd_u32(p + 4) << 32);
}
static inline float exr_rd_f32(const uint8_t *p) {
    float f;
    uint32_t u = exr_rd_u32(p);
    memcpy(&f, &u, sizeof(f));
    return f;
}

static inline void exr_wr_u16(uint8_t *p, uint16_t v) {
    p[0] = (uint8_t)v;
    p[1] = (uint8_t)(v >> 8);
}
static inline void exr_wr_u32(uint8_t *p, uint32_t v) {
    p[0] = (uint8_t)v;
    p[1] = (uint8_t)(v >> 8);
    p[2] = (uint8_t)(v >> 16);
    p[3] = (uint8_t)(v >> 24);
}
static inline void exr_wr_i32(uint8_t *p, int32_t v) { exr_wr_u32(p, (uint32_t)v); }
static inline void exr_wr_u64(uint8_t *p, uint64_t v) {
    exr_wr_u32(p, (uint32_t)v);
    exr_wr_u32(p + 4, (uint32_t)(v >> 32));
}
static inline void exr_wr_f32(uint8_t *p, float f) {
    uint32_t u;
    memcpy(&u, &f, sizeof(u));
    exr_wr_u32(p, u);
}

/* ============================================================================
 * Allocator helpers
 * ========================================================================== */

const exr_allocator *exr_default_allocator(void);
void *exr_malloc(const exr_allocator *a, size_t size);
void *exr_calloc(const exr_allocator *a, size_t count, size_t size);
void exr_free(const exr_allocator *a, void *ptr);
char *exr_strdup(const exr_allocator *a, const char *s);

/* ============================================================================
 * SIMD dispatch
 * ========================================================================== */

/* Runtime-detected CPU features (bit flags mirror exr_simd_caps). */
uint32_t exr_cpu_caps(void);

/* Function-pointer table populated once by exr_simd_init() (idempotent). */
typedef struct {
    void (*half_to_float)(const uint16_t *src, float *dst, size_t count);
    void (*float_to_half)(const float *src, uint16_t *dst, size_t count);
    void (*interleave)(const uint8_t *src, uint8_t *dst, size_t n); /* de-split */
    void (*predictor_decode)(uint8_t *p, size_t n); /* delta -> prefix sum */
    void (*predictor_encode)(uint8_t *p, size_t n); /* prefix sum -> delta */
    /* Util-module hot kernels (src/exr_util_simd_*). `scale` folds the
     * raw-vs-normalized factor; the float->int kernels clamp then round to
     * nearest (ties to even) to match the hardware convert. */
    void (*u8_to_f32)(float *dst, const uint8_t *src, size_t n, float scale);
    void (*u16_to_f32)(float *dst, const uint16_t *src, size_t n, float scale);
    void (*f32_to_u8)(uint8_t *dst, const float *src, size_t n, float scale);
    void (*f32_to_u16)(uint16_t *dst, const float *src, size_t n, float scale);
    void (*axpy)(float *acc, const float *x, float w, size_t n); /* acc += w*x */
    /* Apply row-major 3x3 to interleaved RGB(A); alpha (4th ch) passes through. */
    void (*mat3)(float *dst, const float *src, size_t px, int ch, const float *m);
} exr_simd_vtbl;

extern exr_simd_vtbl exr_simd;
void exr_simd_init(void);
/* Force a kernel tier for benchmarking: 0=scalar, 1=sse2/neon, 2=avx2/f16c. */
void exr_simd_force(int level);

/* Benchmark-only: override the cached CPU caps so codec-internal dispatch that
 * reads exr_cpu_caps() directly (e.g. the JPH/HTJ2K path) selects a specific
 * tier. level: -1 restore real detection, 0 scalar, 1 sse4.1, 2 avx2. */
void exr_cpu_caps_force(int level);

/* Simple parallel-for over [0, njobs): runs fn(ctx, job) for each job, using up
 * to `nthreads` workers (the calling thread participates). Always defined; when
 * the library is built without EXR_USE_THREADS it runs serially. Jobs must be
 * independent and report results through ctx (no aggregated return value).
 * Defined in exr_thread.c. */
typedef void (*exr_par_fn)(void *ctx, int job);
void exr_parallel_for(int nthreads, int njobs, exr_par_fn fn, void *ctx);

/* Scalar kernels (always built). */
void exr_half_to_float_scalar(const uint16_t *src, float *dst, size_t count);
void exr_float_to_half_scalar(const float *src, uint16_t *dst, size_t count);
void exr_interleave_scalar(const uint8_t *src, uint8_t *dst, size_t n);
void exr_predictor_decode_scalar(uint8_t *p, size_t n);
void exr_predictor_encode_scalar(uint8_t *p, size_t n);

#if defined(__x86_64__) || defined(_M_X64) || defined(__i386__) || defined(_M_IX86)
#define EXR_X86 1
void exr_interleave_sse2(const uint8_t *src, uint8_t *dst, size_t n);
void exr_interleave_avx2(const uint8_t *src, uint8_t *dst, size_t n);
/* SSE2 predictor: prefix-sum (decode) / delta (encode), bit-identical to the
 * scalar reference. */
void exr_predictor_decode_sse2(uint8_t *p, size_t n);
void exr_predictor_encode_sse2(uint8_t *p, size_t n);
void exr_half_to_float_f16c(const uint16_t *src, float *dst, size_t count);
void exr_float_to_half_f16c(const float *src, uint16_t *dst, size_t count);

/* JPH (HTJ2K) SIMD kernels (src/exr_jph_simd.c). Each has a scalar reference of
 * the same name with a `_scalar` suffix that is the source of truth; the SIMD
 * variants must be bit-identical. Dispatched at runtime via exr_cpu_caps(). */
void jph_nlt_type3_i64_sse2(int64_t *data, size_t count, int64_t bias);
void jph_nlt_type3_i64_avx2(int64_t *data, size_t count, int64_t bias);
/* int32 NLT type-3 (all-HALF / bit_depth<=31 path): if v<0, v = ~v - biasm1
 * (== -v-bias, which always fits int32 for bit_depth<=31). 8/4-lane masked. */
void jph_nlt_type3_i32_sse2(int32_t *data, size_t count, int32_t biasm1);
void jph_nlt_type3_i32_avx2(int32_t *data, size_t count, int32_t biasm1);
/* Pack n int32 plane samples (all-HALF fast path) to little-endian uint16 by
 * truncation (low 16 bits), matching the scalar (uint16_t)v store. */
void jph_pack_i32_to_half_sse41(uint8_t *dst, const int32_t *src, size_t n);
void jph_pack_i32_to_half_avx2(uint8_t *dst, const int32_t *src, size_t n);
/* AVX2 inverse reversible 5/3 1D lifting (int32, int64 intermediates, 4 lanes).
 * Bit-identical to exr_jph_inverse_53_i32; returns EXR_ERROR_CORRUPT iff a
 * reconstructed sample exceeds int32 (matching the scalar). ev/od are caller
 * scratch of >= low_count / >= high_count int64 each. */
exr_result jph_inverse_53_i32_avx2(const int32_t *low, size_t low_count,
                                   const int32_t *high, size_t high_count,
                                   int32_t *out, size_t out_count,
                                   int64_t *ev, int64_t *od);
/* Internal bounded all-HALF decode variants: same math as the checked kernels,
 * but omit int32 overflow tests when the JPH profile's component precision keeps
 * reconstructed coefficients safely inside int32. */
exr_result jph_inverse_53_i32_bounded_avx2(const int32_t *low, size_t low_count,
                                           const int32_t *high,
                                           size_t high_count, int32_t *out,
                                           size_t out_count, int64_t *ev,
                                           int64_t *od);
/* Forward reversible 5/3 1D lifting (int64), bit-identical to the scalar
 * jph_forward_53_i64; ev/od are caller scratch of >= ceil(n/2) int64 each. */
exr_result jph_forward_53_i64_avx2(const int64_t *src, size_t n, int64_t *low,
                                   size_t low_count, int64_t *high,
                                   size_t high_count, int64_t *ev, int64_t *od);
/* AVX2 vertical (column) inverse reversible 5/3 (int32), row-wise across all
 * columns; bit-identical to exr_jph_inverse_53_vert_i32. temp: lh low-rows then
 * hh high-rows (stride rw) -> rh interleaved rows in data (stride width). */
exr_result jph_inverse_53_vert_i32_avx2(const int32_t *temp, size_t rw,
                                        size_t lh, size_t hh,
                                        int32_t *data, size_t width);
exr_result jph_inverse_53_vert_i32_bounded_avx2(const int32_t *temp, size_t rw,
                                               size_t lh, size_t hh,
                                               int32_t *data, size_t width);
/* AVX2 inverse reversible 5/3 1D (int64, float/32-bit decode path); bit-identical
 * to jph_inverse_53_i64. ev/od are caller scratch (>= low/high counts). */
exr_result jph_inverse_53_i64_avx2(const int64_t *low, size_t low_count,
                                   const int64_t *high, size_t high_count,
                                   int64_t *out, size_t out_count,
                                   int64_t *ev, int64_t *od);
/* AVX2 vertical (column) inverse 5/3 (int64), row-wise; bit-identical to
 * jph_inverse_53_vert_i64. temp: lh low-rows, hh high-rows -> rh rows in data. */
exr_result jph_inverse_53_vert_i64_avx2(const int64_t *temp, size_t rw,
                                        size_t lh, size_t hh,
                                        int64_t *data, size_t width);
/* AVX2 sign-magnitude codeblock word -> signed int64 coefficient; bit-identical
 * to the scalar extraction loop in jph_decode_block. */
void jph_extract_signmag_i32_to_i64_avx2(int64_t *out, const uint32_t *buf,
                                         size_t n, unsigned shift);
/* Same conversion for the all-HALF/int32 decode path; stores signed int32
 * coefficients directly, avoiding an int64 codeblock round trip. */
void jph_extract_signmag_i32_to_i32_avx2(int32_t *out, const uint32_t *buf,
                                         size_t n, unsigned shift);
/* AVX2 cleanup-pass MagSgn reconstruction for the <=15-bit path. Decodes four
 * HT quads (8 columns x 2 rows) from interleaved scratch inf/u words and writes
 * OpenJPH sign-magnitude codeblock words. bottom_vn receives left/right bottom
 * v_n values for each quad: L0,R0,L1,R1,L2,R2,L3,R3. */
void jph_decode_four_quad16_avx2(uint32_t *row0, uint32_t *row1,
                                 uint16_t bottom_vn[8],
                                 const uint16_t *inf_uq,
                                 const uint32_t u_q[4],
                                 const uint64_t *bits, uint64_t real_bits,
                                 uint64_t *cursor, unsigned p16);
typedef struct {
    const uint8_t *data;
    uint8_t tmp[48];
    uint32_t bits;
    uint32_t unstuff;
    int size;
} JphFrwdAvx2;
void jph_frwd_init_ff_avx2(JphFrwdAvx2 *msp, const uint8_t *data, int size);
void jph_decode_four_quad16_frwd_avx2(uint32_t *row0, uint32_t *row1,
                                      uint16_t bottom_vn[8],
                                      const uint16_t *inf_uq,
                                      const uint32_t u_q[4],
                                      JphFrwdAvx2 *magsgn,
                                      unsigned p16);
void jph_decode_two_quad32_frwd_avx2(uint32_t *row0, uint32_t *row1,
                                     uint32_t bottom_vn[4],
                                     const uint16_t *inf_uq,
                                     const uint32_t u_q[2],
                                     JphFrwdAvx2 *magsgn, unsigned p);
/* AVX2 vertical (column) forward reversible 5/3 (int64), row-wise across all
 * columns; bit-identical to jph_forward_53_vert_i64. data's interleaved rows
 * (stride width) -> subband layout in temp (stride rw): lh low-rows, hh high. */
exr_result jph_forward_53_vert_i64_avx2(const int64_t *data, size_t width,
                                        size_t rw, size_t lh, size_t hh,
                                        int64_t *temp);
/* AVX2 forward reversible 5/3 1D lifting (int32, int64 intermediates, 8 lanes).
 * Bit-identical to jph_forward_53_i32; deinterleaves int32 src -> int64 ev/od
 * scratch, then predict (high) and update (low) with floor-div-by-2^s. Outputs
 * int32 low/high arrays. ev/od are caller scratch of >= ceil(n/2) int64 each. */
exr_result jph_forward_53_i32_avx2(const int32_t *src, size_t n, int32_t *low,
                                    size_t low_count, int32_t *high,
                                    size_t high_count, int64_t *ev, int64_t *od);
/* AVX2 vertical (column) forward reversible 5/3 (int32), row-wise across all
 * columns; bit-identical to jph_forward_53_vert_i32. data's interleaved rows
 * (stride width) -> subband layout in temp (stride rw): lh low-rows, hh high. */
exr_result jph_forward_53_vert_i32_avx2(const int32_t *data, size_t width,
                                        size_t rw, size_t lh, size_t hh,
                                        int32_t *temp);
/* SSE2 int32 quad sample preparation for the HT encode block.
 * Processes 4 samples of a 2x2 quad at once via SSE2 load/shuffle/compute.
 * Updates rho (|=), e_qmax (max), e_q[4], s[4], and max_val in one pass. */
void jph_encode_prepare_quad_i32_sse2(const int64_t *plane_data,
                                       uint32_t stride, uint32_t x0,
                                       uint32_t y0, uint32_t x, uint32_t y,
                                       uint32_t shift, uint32_t p,
                                       int *rho, int *e_qmax, int e_q[4],
                                       uint64_t s[4], uint64_t *max_val);
void jph_encode_prepare_quad_from32_sse2(const int32_t *plane_data,
                                          uint32_t stride, uint32_t x0,
                                          uint32_t y0, uint32_t x, uint32_t y,
                                          uint32_t shift, uint32_t p,
                                          int *rho, int *e_qmax, int e_q[4],
                                          uint64_t s[4], uint64_t *max_val);
/* AVX2 8-quad (16x2) sample preparation: computes e_q/s/rho/e_qmax for 8 quads
 * from a zero-padded contiguous tile (row0[16] then row1[16]), folding the max
 * |coefficient| into *max_val. Outputs are sample-major: index [k*8+q] is sample
 * k (0..3) of quad q (0..7). Bit-identical to jph_prep_quad_body_sse2 per quad. */
void jph_enc_proc_pixel_8q_avx2(const int32_t *tile, uint32_t shift, uint32_t p,
                                int32_t eq[32], int32_t sarr[32],
                                int32_t rho8[8], int32_t eqmax8[8],
                                uint64_t *max_val);
/* AVX2 8-quad context kernel: vectorizes the per-quad bookkeeping (c_q, eps,
 * kappa, U_q, u_q) and the E/CX line-state updates, bit-identical to the scalar
 * encoder. e_line/cx_line are a flat int32 line-state indexed by quad position
 * (zero-init at codeblock start, sized n_vec*8 + slack). initial!=0 -> y=0 row
 * (proc_cq1, kappa==1); else main rows (proc_cq2, max_e-derived kappa). */
void jph_enc_context_8q_avx2(int initial, const int32_t eq[32],
                             const int32_t rho8[8], const int32_t eqmax8[8],
                             int32_t *e_line, int32_t *cx_line, uint32_t xv,
                             int *prev_cq, int *prev_e, int *prev_cx,
                             int32_t cq8[8], int32_t eps8[8], int32_t uq8[8],
                             int32_t Uq8[8]);
/* AVX2 8-quad MagSgn-prep kernel: computes m_n and the masked magnitude
 * cwd_s = s & ((1<<m_n)-1) for all 8 quads (sample-major [k*8+q]); the serial
 * pair assembly + bit-append stay with the caller. i32 path only (m_n<=30). */
void jph_enc_ms_prep_8q_avx2(const int32_t rho8[8], const int32_t Uq8[8],
                             const int32_t tuple8[8], const int32_t sarr[32],
                             int32_t m_n[32], int32_t cwd_s[32]);
/* Half-pixel deinterleave: bytes → int32 (SSE2/AVX2), sign-extending LE uint16.
 * Returns the number of pixels processed by the SIMD loop. */
size_t jph_deinterleave_half_sse2(const uint8_t *src, int32_t *dst,
                                   size_t count);
size_t jph_deinterleave_half_avx2(const uint8_t *src, int32_t *dst,
                                   size_t count);
/* Reversible Color Transform (int32, all-HALF path). Each processes vectors
 * whose three inputs are all within +/-2^28 (where the int32 math is exact and
 * cannot overflow int32), in place; returns the element count processed. The
 * caller finishes the remainder (and the precise int32 overflow check) scalar. */
size_t jph_inverse_rct_i32_sse2(int32_t *c0, int32_t *c1, int32_t *c2,
                                size_t count);
size_t jph_inverse_rct_i32_avx2(int32_t *c0, int32_t *c1, int32_t *c2,
                                size_t count);
size_t jph_forward_rct_i32_sse2(int32_t *c0, int32_t *c1, int32_t *c2,
                                size_t count);
size_t jph_forward_rct_i32_avx2(int32_t *c0, int32_t *c1, int32_t *c2,
                                size_t count);
#endif

/* Scalar references for the JPH NLT type-3 involution (v<0 -> -v-bias). */
void jph_nlt_type3_i64_scalar(int64_t *data, size_t count, int64_t bias);
void jph_nlt_type3_i32_scalar(int32_t *data, size_t count, int32_t biasm1);
void jph_pack_i32_to_half_scalar(uint8_t *dst, const int32_t *src, size_t n);
#if defined(__ARM_NEON) || defined(__ARM_NEON__)
#define EXR_NEON 1
/* NEON half<->float via FCVTL/FCVTN (mirrors the x86 F16C kernels). */
void exr_half_to_float_neon(const uint16_t *src, float *dst, size_t count);
void exr_float_to_half_neon(const float *src, uint16_t *dst, size_t count);
void exr_interleave_neon(const uint8_t *src, uint8_t *dst, size_t n);
/* NEON predictor: prefix-sum (decode) / delta (encode), bit-identical to the
 * scalar reference. */
void exr_predictor_decode_neon(uint8_t *p, size_t n);
void exr_predictor_encode_neon(uint8_t *p, size_t n);
/* NEON JPH (HTJ2K) kernels (src/exr_jph_simd_neon.c), bit-identical to the
 * `_scalar` references; the NEON path is selected at compile time. */
void jph_nlt_type3_i64_neon(int64_t *data, size_t count, int64_t bias);
void jph_nlt_type3_i32_neon(int32_t *data, size_t count, int32_t biasm1);
void jph_pack_i32_to_half_neon(uint8_t *dst, const int32_t *src, size_t n);
void jph_extract_signmag_i32_to_i64_neon(int64_t *out, const uint32_t *buf,
                                         size_t n, unsigned shift);
exr_result jph_inverse_53_i32_neon(const int32_t *low, size_t low_count,
                                   const int32_t *high, size_t high_count,
                                   int32_t *out, size_t out_count,
                                   int64_t *ev, int64_t *od);
exr_result jph_inverse_53_vert_i32_neon(const int32_t *temp, size_t rw,
                                        size_t lh, size_t hh,
                                        int32_t *data, size_t width);
exr_result jph_inverse_53_i64_neon(const int64_t *low, size_t low_count,
                                   const int64_t *high, size_t high_count,
                                   int64_t *out, size_t out_count,
                                   int64_t *ev, int64_t *od);
exr_result jph_inverse_53_vert_i64_neon(const int64_t *temp, size_t rw,
                                        size_t lh, size_t hh,
                                        int64_t *data, size_t width);
exr_result jph_forward_53_i64_neon(const int64_t *src, size_t n, int64_t *low,
                                   size_t low_count, int64_t *high,
                                   size_t high_count, int64_t *ev, int64_t *od);
exr_result jph_forward_53_vert_i64_neon(const int64_t *data, size_t width,
                                        size_t rw, size_t lh, size_t hh,
                                        int64_t *temp);
#endif

/* ============================================================================
 * Util-module kernels (scalar reference = source of truth; SIMD must match).
 * Scalar refs are defined in their natural TU (convert/resize/color); the SIMD
 * variants live in src/exr_util_simd_x86.c / src/exr_util_simd_neon.c.
 * ========================================================================== */

void exr_u8_to_f32_scalar(float *dst, const uint8_t *src, size_t n, float scale);
void exr_u16_to_f32_scalar(float *dst, const uint16_t *src, size_t n, float scale);
void exr_f32_to_u8_scalar(uint8_t *dst, const float *src, size_t n, float scale);
void exr_f32_to_u16_scalar(uint16_t *dst, const float *src, size_t n, float scale);
void exr_axpy_scalar(float *acc, const float *x, float w, size_t n);
void exr_mat3_scalar(float *dst, const float *src, size_t px, int ch,
                     const float *m);

/* Hand-rolled, libm-free transcendentals used by transfer functions (kept out
 * of the freestanding forbidden set: names are not the bare exp/log/pow words).
 * Accurate to ~1e-6 relative over the working range; verified against libm in
 * the test build. */
float exr_util_log2f(float x);
float exr_util_exp2f(float x);
float exr_util_powf(float x, float y);
float exr_util_sqrtf(float x);

#if defined(EXR_X86)
void exr_u8_to_f32_sse41(float *dst, const uint8_t *src, size_t n, float scale);
void exr_u16_to_f32_sse41(float *dst, const uint16_t *src, size_t n, float scale);
void exr_f32_to_u8_sse41(uint8_t *dst, const float *src, size_t n, float scale);
void exr_f32_to_u16_sse41(uint16_t *dst, const float *src, size_t n, float scale);
void exr_axpy_avx2(float *acc, const float *x, float w, size_t n);
void exr_mat3_avx2(float *dst, const float *src, size_t px, int ch,
                   const float *m);
#endif
#if defined(EXR_NEON)
void exr_u8_to_f32_neon(float *dst, const uint8_t *src, size_t n, float scale);
void exr_u16_to_f32_neon(float *dst, const uint16_t *src, size_t n, float scale);
void exr_f32_to_u8_neon(uint8_t *dst, const float *src, size_t n, float scale);
void exr_f32_to_u16_neon(uint16_t *dst, const float *src, size_t n, float scale);
void exr_axpy_neon(float *acc, const float *x, float w, size_t n);
void exr_mat3_neon(float *dst, const float *src, size_t px, int ch,
                   const float *m);
#endif

/* ============================================================================
 * Pixel helpers
 * ========================================================================== */

static inline size_t exr_pixel_size(exr_pixel_type t) {
    return (t == EXR_PIXEL_HALF) ? 2u : 4u;
}

/* Floor division and OpenEXR sample-count over an inclusive range [lo,hi]. */
static inline int64_t exr_floordiv(int64_t a, int64_t b) {
    int64_t q = a / b, r = a % b;
    if ((r != 0) && ((r < 0) != (b < 0))) q--;
    return q;
}
static inline int exr_num_samples(int lo, int hi, int s) {
    if (s <= 1) return hi - lo + 1;
    return (int)(exr_floordiv(hi, s) - exr_floordiv((int64_t)lo - 1, s));
}

/* Scanlines compressed together per chunk for a given compression. */
int exr_lines_per_block(exr_compression c);

/* ============================================================================
 * Attribute list (opaque exr_attr_list defined here)
 * ========================================================================== */

typedef struct exr_attr {
    char *name;      /* owned */
    char *type_name; /* owned */
    uint8_t *data;   /* owned raw bytes */
    uint32_t size;
} exr_attr;

struct exr_attr_list {
    exr_attr *items;
    uint32_t count;
    uint32_t capacity;
};

void exr_attr_list_free(const exr_allocator *a, exr_attr_list *list);
const exr_attr *exr_attr_find(const exr_attr_list *list, const char *name);

/* Append a copy of (name,type,data) to the list, growing it as needed. Used by
 * the header parser and by exr_header_set_attribute. */
exr_result exr_attr_list_append(const exr_allocator *a, exr_attr_list *list,
                                const char *name, size_t name_len,
                                const char *type, size_t type_len,
                                const uint8_t *data, uint32_t size);

/* Parse one channel-list ("chlist") attribute into a freshly allocated channel
 * array. Returns EXR_SUCCESS and sets *out_channels / *out_count. */
exr_result exr_parse_chlist(const exr_allocator *a, const uint8_t *data,
                            uint32_t size, exr_channel **out_channels,
                            int32_t *out_count);

/* ============================================================================
 * Header deep-copy / free
 * ========================================================================== */

exr_result exr_header_copy(const exr_allocator *a, exr_header *dst,
                           const exr_header *src);
void exr_header_free(const exr_allocator *a, exr_header *hdr);

/* The allocator a writer was created with (exr_stdio.c frees finalize buffers
 * with it). exr_writer is opaque outside exr_writer.c. */
const exr_allocator *exr_writer_allocator(const exr_writer *w);

/* ============================================================================
 * Internal per-part state held by the reader
 * ========================================================================== */

typedef struct exr_int_part {
    exr_header header; /* owns channels + attrs */

    uint64_t *offsets; /* chunk offset table (file byte offsets) */
    uint32_t num_chunks;

    /* geometry */
    int32_t width;  /* data window width  */
    int32_t height; /* data window height */

    /* tiling (valid when header.tiled) */
    int32_t num_x_levels;
    int32_t num_y_levels;
    /* per-level pixel sizes and tile counts; allocated when tiled. */
    int32_t *level_width;   /* [num_x_levels] */
    int32_t *level_height;  /* [num_y_levels] */
    int32_t *level_x_tiles; /* [num_x_levels] tiles across */
    int32_t *level_y_tiles; /* [num_y_levels] tiles down */
} exr_int_part;

/* ============================================================================
 * Reader (shared between exr_core.c and exr_reader.c)
 * ========================================================================== */

typedef enum exr_src_kind {
    EXR_SRC_MEMORY = 0,
    EXR_SRC_CALLBACK = 1
} exr_src_kind;

struct exr_reader {
    exr_allocator alloc;

    exr_src_kind kind;
    const uint8_t *mem; /* memory path: whole file (may be owned) */
    size_t mem_size;
    int free_mem; /* if set, exr_reader_close frees (void*)mem */
    exr_data_source src; /* callback path */

    int parsed;
    uint32_t version_flags;
    int is_tiled;
    int is_multipart;
    int is_deep;
    int long_names;

    exr_int_part *parts;
    int32_t num_parts;

    /* streaming suspend state (callback path). The whole file is buffered into
     * `mem` incrementally; `filled` bytes are valid so far. */
    size_t filled;
    int have_pending;
    exr_pending_read pending;
};

/* Ensure the streaming buffer holds the whole file; may return EXR_WOULD_BLOCK
 * with r->pending set. No-op for the memory path. */
exr_result exr_reader_ensure_buffered(exr_reader *r);

/*
 * Random-access read of [offset, offset+size) from the source.
 *   - memory path: returns a direct pointer (zero-copy) via *out_ptr, *out_ptr
 *     stays valid for the reader lifetime. scratch is ignored.
 *   - callback path: copies into scratch (caller-provided, >= size) and sets
 *     *out_ptr = scratch.
 * Returns EXR_WOULD_BLOCK if the callback path needs bytes (Phase 9).
 */
exr_result exr_reader_fetch(exr_reader *r, uint64_t offset, size_t size,
                            void *scratch, const uint8_t **out_ptr);

/* Mipmap pyramid generation for tiled writing (exr_mip.c). Channels stay in
 * the part's original order; img[level][channel] is a contiguous level image.
 * Only x/y sampling == 1 is supported. */
typedef struct {
    int num_levels;
    int *lw;     /* [num_levels] */
    int *lh;     /* [num_levels] */
    void ***img; /* [num_levels][num_channels] */
    int num_channels;
    exr_allocator alloc;
} exr_mip_pyramid;

exr_result exr_mip_generate(const exr_allocator *a, const exr_part *part,
                            int round_up, exr_mip_pyramid *out);
void exr_mip_free(exr_mip_pyramid *pyr);

/* Ripmap pyramid: img[lx*num_y_levels + ly][channel] is the level-0 image
 * downsampled by 2^lx in x and 2^ly in y. Sampling == 1 only. */
typedef struct {
    int num_x_levels;
    int num_y_levels;
    int *lw; /* [num_x_levels] */
    int *lh; /* [num_y_levels] */
    void ***img;
    int num_channels;
    exr_allocator alloc;
} exr_ripmap_pyramid;

exr_result exr_ripmap_generate(const exr_allocator *a, const exr_part *part,
                               int round_up, exr_ripmap_pyramid *out);
void exr_ripmap_free(exr_ripmap_pyramid *pyr);

/* Deep read (exr_deep.c). */
exr_result exr_read_deep_scanline_part(exr_reader *r, exr_int_part *p,
                                       int32_t part_idx, exr_part *out);
exr_result exr_read_deep_tiled_part(exr_reader *r, exr_int_part *p,
                                    int32_t part_idx, exr_part *out);

/* Deep block streaming decode (exr_deep.c): decode one chunk `idx` block-local.
 * bw/bh are the block pixel extent (from exr_reader_block_info); is_tiled selects
 * the chunk-header layout. _counts fills bw*bh per-pixel counts (block row-major);
 * _samples fills chan_dst[c] with sum(counts) contiguous samples per channel. */
exr_result exr_deep_decode_counts(exr_reader *r, exr_int_part *p,
                                  int32_t part_idx, uint32_t idx, int bw, int bh,
                                  int is_tiled, int32_t *counts);
exr_result exr_deep_decode_samples(exr_reader *r, exr_int_part *p,
                                   int32_t part_idx, uint32_t idx, int bw, int bh,
                                   int is_tiled, void *const *chan_dst);

/* Build a temporary deep part for tile level (lw x lh) by point-subsampling the
 * source: level pixel (x,y) takes the samples of source pixel
 * (x*W/lw, y*H/lh). out shares src's header (channels not owned); free the deep
 * arrays with exr_deep_level_free. src_prefix[pixel] is src's running offset. */
exr_result exr_deep_build_level(const exr_allocator *a, const exr_part *src,
                                const uint64_t *src_prefix, int lw, int lh,
                                exr_part *out);
void exr_deep_level_free(const exr_allocator *a, exr_part *level);

/* Deep tiled write: encode one tile's offset table + sample data. tile_w/tile_h
 * are the (clamped) tile pixel dimensions; x0/y0 the tile pixel origin. */
exr_result exr_deep_encode_tile(const exr_allocator *a, exr_compression comp,
                                const exr_part *part, const uint64_t *prefix,
                                int x0, int y0, int tile_w, int tile_h,
                                uint8_t **packed_off, size_t *packed_off_size,
                                uint64_t *unpacked_off_size, uint8_t **packed_samp,
                                size_t *packed_samp_size,
                                uint64_t *unpacked_samp_size);

/* Deep scanline write: encode one block's offset table + sample data, each
 * compressed per `comp`. `prefix[pixel]` is the running sample offset of each
 * pixel within part->deep_images[c]. Allocates *packed_off / *packed_samp. */
exr_result exr_deep_encode_block(const exr_allocator *a, exr_compression comp,
                                 const exr_part *part, const uint64_t *prefix,
                                 int y0, int nlines, uint8_t **packed_off,
                                 size_t *packed_off_size,
                                 uint64_t *unpacked_off_size,
                                 uint8_t **packed_samp, size_t *packed_samp_size,
                                 uint64_t *unpacked_samp_size);

/* ============================================================================
 * Codec layer (exr_codec.c and friends). Each decodes one chunk's raw
 * compressed bytes into the canonical uncompressed scanline/tile block.
 * `channels` is the part's sorted channel list; the block layout is, per
 * scanline then per channel, sample data of width/x_sampling samples.
 * ========================================================================== */

typedef struct exr_codec_ctx {
    const exr_allocator *alloc;
    exr_compression compression;
    const exr_channel *channels;
    int32_t num_channels;
    int32_t x; /* data window x origin (for sampling alignment) */
    int32_t y; /* first scanline of this block (data window coords) */
    int32_t width;
    int32_t num_lines; /* scanlines in this block */
} exr_codec_ctx;

/* Decompress `src_size` bytes from `src` into `dst` (capacity dst_size, which
 * must equal the exact uncompressed block size). */
exr_result exr_decompress_block(const exr_codec_ctx *ctx, const uint8_t *src,
                                size_t src_size, uint8_t *dst, size_t dst_size);

/* Fetch the raw compressed chunk bytes + codec ctx for block `idx` without
 * decompressing (used by the GPU backend for device-side reconstruction). The
 * returned pointer is valid until the next reader fetch. */
exr_result exr_reader_block_raw(exr_reader *r, int32_t part, uint32_t idx,
                                exr_block_info *out_bi, const uint8_t **out_cdata,
                                size_t *out_csize, exr_codec_ctx *out_ctx);

/* Streaming encode seam for the GPU backend: write one scanline block from a
 * pre-gathered canonical buffer (the device does the planar->canonical gather),
 * and query the writer's sorted channel order. Declared in exr_writer.c. */
exr_result exr_writer_write_scanline_block_canon(exr_writer *w, int32_t part,
                                                 int32_t y0, const uint8_t *block,
                                                 size_t block_size);
const int *exr_writer_sorted_order(exr_writer *w, int32_t part);

/* Install a GPU HTJ2K block-encode hook; exr_writer_write_scanline_block_canon
 * then routes HTJ2K parts through it (falling back to the CPU coder per-chunk
 * for non-i32-eligible blocks). Declared after exr_jph_gpu_block_encode_fn. */
struct exr_jph_enc_plan;
typedef exr_result (*exr_jph_gpu_block_encode_fn)(void *user,
                                                  const struct exr_jph_enc_plan *plan,
                                                  unsigned char *out_bytes,
                                                  unsigned int out_stride,
                                                  unsigned int *out_missing,
                                                  unsigned int *out_len0,
                                                  unsigned int *out_size);
void exr_writer_set_gpu_jph_encoder(exr_writer *w,
                                    exr_jph_gpu_block_encode_fn fn, void *user);

/* Uncompressed byte size of one scanline block / tile region. */
exr_result exr_block_uncompressed_size(const exr_channel *channels,
                                       int32_t num_channels, int32_t x,
                                       int32_t y, int32_t width,
                                       int32_t num_lines, size_t *out_size);

/* Individual codecs (decode). ZIP/RLE need scratch for the pre-reconstruction
 * buffer, hence the allocator. */
exr_result exr_rle_decompress(const exr_allocator *a, const uint8_t *src,
                              size_t src_size, uint8_t *dst, size_t dst_size);
exr_result exr_zip_decompress(const exr_allocator *a, const uint8_t *src,
                              size_t src_size, uint8_t *dst, size_t dst_size);

/* Entropy-decode only: produce the pre-reconstruction buffer (after inflate /
 * RLE-expand, BEFORE the byte predictor + even/odd deinterleave). `dst` must be
 * dst_size bytes. The GPU backend uses these so the reconstruction passes can
 * run as kernels; the CPU path keeps doing predictor+interleave on the host. */
exr_result exr_zip_inflate_only(const uint8_t *src, size_t src_size,
                                uint8_t *dst, size_t dst_size);
exr_result exr_rle_expand_only(const uint8_t *src, size_t src_size,
                               uint8_t *dst, size_t dst_size);
exr_result exr_pxr24_decompress(const exr_codec_ctx *ctx, const uint8_t *src,
                                size_t src_size, uint8_t *dst, size_t dst_size);
exr_result exr_piz_decompress(const exr_codec_ctx *ctx, const uint8_t *src,
                              size_t src_size, uint8_t *dst, size_t dst_size);
exr_result exr_b44_decompress(const exr_codec_ctx *ctx, const uint8_t *src,
                              size_t src_size, uint8_t *dst, size_t dst_size,
                              int optimize_flat);
exr_result exr_zstd_decompress(const exr_allocator *a, const uint8_t *src,
                               size_t src_size, uint8_t *dst, size_t dst_size);
exr_result exr_jph_decompress(const exr_codec_ctx *ctx, const uint8_t *src,
                              size_t src_size, uint8_t *dst, size_t dst_size);

/* ---- GPU HTJ2K seam ------------------------------------------------------
 * The HT block coder is bit-serial within a code-block but embarrassingly
 * parallel across code-blocks, so the GPU backend collects every code-block of
 * a chunk into a flat plan, decodes them all in one kernel launch (one thread
 * per block), then the CPU scatters + runs the inverse transforms. These
 * read-only seams expose the plan, the (runtime-built) HT tables, and a single
 * reference/fallback block decode, without leaking the file-local jph structs.
 */
typedef struct exr_jph_cb_record {
    uint32_t width, height;       /* code-block dims (<=128 x <=32) */
    uint32_t missing_msbs, active_passes;
    uint32_t length0, length1;    /* lcup, len2 */
    uint32_t kmax;
    uint32_t comp, res, band, x0, y0;
    uint32_t dst_row, dst_col;    /* origin in the component plane */
    uint32_t plane_w, plane_h;    /* component plane dims */
    size_t   data_offset;         /* byte offset into plan->data */
    size_t   data_size;
    int      i32_eligible;        /* missing_msbs<30 && kmax<=30 (else CPU) */
} exr_jph_cb_record;

typedef struct exr_jph_cb_plan {
    exr_jph_cb_record *records;
    size_t num_records;
    uint8_t *data;                /* contiguous copy of the tile codestream */
    size_t data_size;
    int num_components;
} exr_jph_cb_plan;

/* Parse an HTJ2K chunk and collect its code-block plan (no decode). */
exr_result exr_jph_collect_codeblocks(const exr_codec_ctx *ctx,
                                      const uint8_t *src, size_t src_size,
                                      exr_jph_cb_plan *out);
void exr_jph_cb_plan_free(const exr_allocator *a, exr_jph_cb_plan *plan);

/* Whole-image GPU decode: the backend supplies a batch block-decode callback
 * (decodes every i32-eligible record of `plan` into `coeffs` at tile_offsets[i],
 * stride (width+7)&~7). exr_jph_decompress_gpu collects the chunk's plan, calls
 * the hook, scatters the tiles into the component planes, then runs the inverse
 * transforms + store on the CPU. Returns EXR_ERROR_UNSUPPORTED (so the caller
 * falls back to exr_jph_decompress) when any code-block is not i32-eligible. */
typedef exr_result (*exr_jph_gpu_block_decode_fn)(void *user,
                                                  const exr_jph_cb_plan *plan,
                                                  const size_t *tile_offsets,
                                                  size_t coeff_count,
                                                  int32_t *coeffs);
exr_result exr_jph_decompress_gpu(const exr_codec_ctx *ctx, const uint8_t *src,
                                  size_t src_size, uint8_t *dst, size_t dst_size,
                                  exr_jph_gpu_block_decode_fn fn, void *user);

/* The runtime-built HT VLC tables (opaque to the backend; layout: four arrays
 * vlc_tbl0[1024], vlc_tbl1[1024], uvlc_tbl0[320], uvlc_tbl1[256], all uint16).
 * Returns pointers + element counts so the backend can upload them verbatim. */
exr_result exr_jph_ht_tables(const uint16_t **vlc0, const uint16_t **vlc1,
                             const uint16_t **uvlc0, const uint16_t **uvlc1);

/* Reference/fallback: decode one code-block (i32 path) into out[stride*h]. */
exr_result exr_jph_decode_one_block_i32(const exr_jph_cb_record *rec,
                                        const uint8_t *data, int32_t *out,
                                        uint32_t out_stride);

/* ---- GPU HTJ2K encode seam ----------------------------------------------
 * Symmetric to the decode plan: collect every code-block's (post-transform)
 * int32 coefficient tile + dims/kmax so the GPU can encode them all in one
 * launch, then the CPU assembles the codestream. */
typedef struct exr_jph_enc_record {
    uint32_t width, height, kmax;
    int      plane_is_i32;        /* all-HALF i32 plane (GPU-eligible path) */
    size_t   coeff_offset;        /* int32 element offset into plan->coeffs */
} exr_jph_enc_record;

typedef struct exr_jph_enc_plan {
    exr_jph_enc_record *records;
    size_t num_records;
    int32_t *coeffs;              /* concatenated cb_w*cb_h tiles, stride=cb_w */
    size_t coeff_count;
} exr_jph_enc_plan;

/* Run the forward transforms for one EXR block and collect its code-block
 * encode plan (no entropy coding / codestream emission). */
exr_result exr_jph_collect_encode_blocks(const exr_codec_ctx *ctx,
                                         const uint8_t *block, size_t n,
                                         exr_jph_enc_plan *out);
void exr_jph_enc_plan_free(const exr_allocator *a, exr_jph_enc_plan *plan);

/* Runtime-built HT encode tables: g_vlc_enc_tbl0/1 (uint16[2048] each) and the
 * uvlc table packed as uint8[75*6] = {pre,pre_len,suf,suf_len,ext,ext_len}. */
exr_result exr_jph_ht_enc_tables(const uint16_t **vlc0, const uint16_t **vlc1,
                                 const uint8_t **uvlc_packed);

/* Reference/fallback: encode one i32 code-block (cleanup pass) from its tile. */
exr_result exr_jph_encode_one_block_i32(const exr_jph_enc_record *rec,
                                        const int32_t *coeffs, uint8_t *out,
                                        size_t out_cap, uint32_t *out_missing,
                                        uint32_t *out_len0, size_t *out_size);

/* Whole-image GPU encode: the backend supplies a batch block-encode callback
 * (encodes every i32-eligible record of `plan`; block i's cleanup bytes land at
 * out_bytes + i*out_stride, with per-block missing/len0/size in the out arrays).
 * exr_jph_compress_gpu builds the planes + plan, calls the hook, then assembles
 * the codestream from the GPU block outputs on the CPU. Returns
 * EXR_ERROR_UNSUPPORTED (caller falls back to exr_jph_compress) when any
 * code-block is not i32-eligible. (exr_jph_gpu_block_encode_fn is declared
 * above, near exr_writer_set_gpu_jph_encoder.) */
exr_result exr_jph_compress_gpu(const exr_codec_ctx *ctx, const uint8_t *block,
                                size_t n, uint8_t **out_data, size_t *out_size,
                                exr_jph_gpu_block_encode_fn fn, void *user);

exr_result exr_jph_inverse_53_i32(const int32_t *low, size_t low_count,
                                  const int32_t *high, size_t high_count,
                                  int32_t *out, size_t out_count);
/* Row-wise vertical inverse 5/3 (int32); source of truth for the AVX2 variant. */
exr_result exr_jph_inverse_53_vert_i32(const int32_t *temp, size_t rw,
                                       size_t lh, size_t hh,
                                       int32_t *data, size_t width);
/* Row-wise vertical forward 5/3 (int64); source of truth for the AVX2 variant. */
exr_result jph_forward_53_vert_i64(const int64_t *data, size_t width,
                                   size_t rw, size_t lh, size_t hh,
                                   int64_t *temp);
/* int64 inverse 5/3 1D + row-wise vertical scalar refs (sources of truth for
 * the AVX2 variants). */
exr_result jph_inverse_53_i64(const int64_t *low, size_t low_count,
                              const int64_t *high, size_t high_count,
                              int64_t *out, size_t out_count);
exr_result jph_inverse_53_vert_i64(const int64_t *temp, size_t rw,
                                   size_t lh, size_t hh,
                                   int64_t *data, size_t width);
exr_result exr_jph_inverse_53_2d_i32(const exr_allocator *a, int32_t *data,
                                     size_t width, size_t height,
                                     unsigned levels);
exr_result exr_jph_inverse_rct_i32(int32_t *c0, int32_t *c1, int32_t *c2,
                                   size_t count);
exr_result exr_jph_apply_nlt_type3_i32(int32_t *data, size_t count,
                                       uint32_t bit_depth);

typedef struct exr_jph_bitreader {
    const uint8_t *p;
    const uint8_t *end;
    uint64_t bits;
    unsigned bit_count;
    int prev_ff;
} exr_jph_bitreader;

void exr_jph_bitreader_init(exr_jph_bitreader *br, const uint8_t *data,
                            size_t size);
exr_result exr_jph_bitreader_read(exr_jph_bitreader *br, unsigned nbits,
                                  uint32_t *out);
void exr_jph_bitreader_align(exr_jph_bitreader *br);
exr_result exr_jph_packet_read_pass_count(exr_jph_bitreader *br,
                                          uint32_t *raw_passes,
                                          uint32_t *active_passes,
                                          uint32_t *placeholder_groups);
exr_result exr_jph_packet_read_pass_lengths(exr_jph_bitreader *br,
                                            uint32_t active_passes,
                                            uint32_t placeholder_groups,
                                            uint32_t lengths[2]);

typedef struct exr_jph_ht_forward_reader {
    const uint8_t *p;
    const uint8_t *end;
    uint64_t bits;
    unsigned bit_count;
    uint8_t fill_byte;
    int prev_ff;
} exr_jph_ht_forward_reader;

typedef struct exr_jph_ht_reverse_reader {
    const uint8_t *start;
    const uint8_t *p;
    uint64_t bits;
    unsigned bit_count;
    int unstuff;
    int zero_fill;
} exr_jph_ht_reverse_reader;

typedef struct exr_jph_mel_reader {
    const uint8_t *p;
    const uint8_t *end;
    uint64_t bits;
    unsigned bit_count;
    int prev_ff;
    int k;
} exr_jph_mel_reader;

void exr_jph_ht_forward_init(exr_jph_ht_forward_reader *r,
                             const uint8_t *data, size_t size,
                             uint8_t fill_byte);
exr_result exr_jph_ht_forward_read(exr_jph_ht_forward_reader *r,
                                   unsigned nbits, uint32_t *out);
void exr_jph_ht_reverse_init(exr_jph_ht_reverse_reader *r,
                             const uint8_t *data, size_t size,
                             int initial_unstuff, int zero_fill);
exr_result exr_jph_ht_reverse_read(exr_jph_ht_reverse_reader *r,
                                   unsigned nbits, uint32_t *out);
void exr_jph_mel_init(exr_jph_mel_reader *r, const uint8_t *data,
                      size_t size);
exr_result exr_jph_mel_get_run(exr_jph_mel_reader *r, uint32_t *zero_run,
                               int *has_one);

#define EXR_JPH_TAGTREE_MAX_LEVELS 32
typedef struct exr_jph_tag_tree {
    uint32_t num_levels;
    uint32_t width[EXR_JPH_TAGTREE_MAX_LEVELS];
    uint32_t height[EXR_JPH_TAGTREE_MAX_LEVELS];
    size_t offset[EXR_JPH_TAGTREE_MAX_LEVELS];
    size_t node_count;
    uint32_t *value;
    uint8_t *known;
} exr_jph_tag_tree;

exr_result exr_jph_tag_tree_init(const exr_allocator *a,
                                 exr_jph_tag_tree *tree, uint32_t width,
                                 uint32_t height);
void exr_jph_tag_tree_free(const exr_allocator *a, exr_jph_tag_tree *tree);
exr_result exr_jph_tag_tree_decode(exr_jph_tag_tree *tree,
                                   exr_jph_bitreader *br, uint32_t leaf_x,
                                   uint32_t leaf_y, uint32_t threshold,
                                   uint32_t *out_value);

/* Raw zlib (DEFLATE) inflate used by ZIP/ZIPS/PXR24. Returns the number of
 * decoded bytes in *out_size; fails on truncation/corruption. */
exr_result exr_inflate_zlib(const uint8_t *src, size_t src_size, uint8_t *dst,
                            size_t dst_cap, size_t *out_size);

/* Apply EXR's two post-DEFLATE reconstruction passes (predictor + byte
 * interleave) in place over `n` bytes. */
void exr_predictor_decode(uint8_t *p, size_t n);
void exr_interleave_decode(const uint8_t *src, uint8_t *dst, size_t n);

/* Forward (encode-side) inverses: byte split, then delta predictor. */
void exr_interleave_encode(const uint8_t *src, uint8_t *dst, size_t n);
void exr_predictor_encode(uint8_t *p, size_t n);

/* Raw zlib (DEFLATE) compress. Allocates *out_data (caller frees with the same
 * allocator); *out_size is the exact compressed length. */
exr_result exr_deflate_zlib(const exr_allocator *a, const uint8_t *src,
                            size_t n, uint8_t **out_data, size_t *out_size);

/* Adler-32 (zlib trailer). */
uint32_t exr_adler32(const uint8_t *data, size_t n, uint32_t adler);

/* libdeflate backend for ZIP/ZIPS/PXR24 (compiled in for DEFLATE=auto|libdeflate
 * via EXR_USE_LIBDEFLATE). The in-tree encoder/inflate above remain the only
 * path for freestanding and WASM builds. ZIP/PXR24 reach whichever backend is
 * active through the exr_zlib_inflate / exr_zlib_deflate pointers below. */
#ifdef EXR_USE_LIBDEFLATE
exr_result exr_ld_deflate_zlib(const exr_allocator *a, const uint8_t *src,
                               size_t n, uint8_t **out_data, size_t *out_size);
exr_result exr_ld_inflate_zlib(const uint8_t *src, size_t src_size, uint8_t *dst,
                               size_t dst_cap, size_t *out_size);
#endif

/* Build-time default for the runtime backend when both are compiled in. The
 * Makefile sets this to 1 for DEFLATE=auto|libdeflate. Freestanding and WASM
 * builds never define EXR_USE_LIBDEFLATE, so only the in-tree path is linked
 * and the pointers below resolve to it regardless of this value. */
#ifndef EXR_ZLIB_DEFAULT_LIBDEFLATE
#define EXR_ZLIB_DEFAULT_LIBDEFLATE 0
#endif

/* Runtime-selectable zlib backend. ZIP/ZIPS/PXR24 call through these pointers
 * (defined in exr_codec.c, statically initialised to the build default). When
 * libdeflate is not compiled in they can only ever point at the in-tree codec.
 * Override with exr_zlib_set_backend() (public, exr.h) before decoding; the
 * pointers are read-only during decode, so do not flip them mid-run. */
typedef exr_result (*exr_zlib_inflate_fn)(const uint8_t *src, size_t src_size,
                                          uint8_t *dst, size_t dst_cap,
                                          size_t *out_size);
typedef exr_result (*exr_zlib_deflate_fn)(const exr_allocator *a,
                                          const uint8_t *src, size_t n,
                                          uint8_t **out_data, size_t *out_size);
/* _Atomic so a write in exr_zlib_set_backend() races defined-behaviour-ly with
 * reads in the codec functions: a concurrent reader sees old-or-new, never a
 * torn value. (The documented contract is still "set before decoding"; this is
 * belt-and-suspenders.) Pointer-sized atomics are lock-free and lower to plain
 * loads/stores, so this adds no libcall and stays freestanding-clean. */
extern _Atomic exr_zlib_inflate_fn exr_zlib_inflate;
extern _Atomic exr_zlib_deflate_fn exr_zlib_deflate;

/* fpnge-derived literal DEFLATE encoder with a PSHUFB Huffman-table lookup.
 * The PSHUFB-friendly table: symbols 0-15 and 240-255 are looked up by low
 * nibble, and 16-239 share one code length (so the code is computable from the
 * byte's nibbles). */
typedef struct {
    uint8_t nbits[286];
    uint16_t end_bits; /* end-of-block (symbol 256) code */
    uint8_t first16_nbits[16], first16_bits[16];
    uint8_t last16_nbits[16], last16_bits[16];
    uint8_t mid_lowbits[16];
    uint8_t mid_nbits;
} exr_fpnge_table;

/* Build the PSHUFB-friendly table from per-symbol frequencies[286]. */
int exr_fpnge_build_table(const exr_allocator *a, const uint64_t *collected,
                          exr_fpnge_table *t);

/* Per-byte Huffman lookup: fills nb[count] and the 16-bit code as blo|bhi<<8.
 * Scalar reference plus SSE4.1/AVX2 PSHUFB kernels (selected by exr_fpnge_deflate). */
void exr_fpnge_lookup_scalar(const exr_fpnge_table *t, const uint8_t *src,
                             size_t count, uint8_t *nb, uint8_t *blo, uint8_t *bhi);
#if defined(__x86_64__) || defined(_M_X64) || defined(__i386__) || defined(_M_IX86)
void exr_fpnge_lookup_sse41(const exr_fpnge_table *t, const uint8_t *src,
                            size_t count, uint8_t *nb, uint8_t *blo, uint8_t *bhi);
void exr_fpnge_lookup_avx2(const exr_fpnge_table *t, const uint8_t *src,
                           size_t count, uint8_t *nb, uint8_t *blo, uint8_t *bhi);
#endif

/* SIMD bit-pack stage: combine consecutive code pairs into 32-bit groups so the
 * scalar bit-writer runs at half the call count. combined[k] = code(2k) |
 * (code(2k+1) << nb(2k)); cnb[k] = nb(2k)+nb(2k+1). Returns count/2 pairs; the
 * caller emits a trailing odd byte. Bit-identical to per-byte emission (codes
 * <= 15 bits keep the float-exponent shift exact). */
size_t exr_fpnge_pack16_scalar(const uint8_t *nb, const uint8_t *blo,
                               const uint8_t *bhi, size_t count,
                               uint32_t *combined, uint32_t *cnb);
#if defined(__x86_64__) || defined(_M_X64) || defined(__i386__) || defined(_M_IX86)
size_t exr_fpnge_pack16_sse41(const uint8_t *nb, const uint8_t *blo,
                              const uint8_t *bhi, size_t count,
                              uint32_t *combined, uint32_t *cnb);
size_t exr_fpnge_pack16_avx2(const uint8_t *nb, const uint8_t *blo,
                             const uint8_t *bhi, size_t count,
                             uint32_t *combined, uint32_t *cnb);
#endif

/* Literal-only zlib stream over src[0..n). use_simd selects the PSHUFB lookup
 * when SSE4.1 is available. Allocates *out_data (caller frees). */
exr_result exr_fpnge_deflate(const exr_allocator *a, const uint8_t *src, size_t n,
                             uint8_t **out_data, size_t *out_size, int use_simd);

/* Codec encoders (compress one canonical block). Each allocates *out_data
 * (caller frees) holding the chunk payload; if compression does not shrink the
 * data the codec stores it verbatim and *out_size == src_size. */
exr_result exr_rle_compress(const exr_allocator *a, const uint8_t *src,
                            size_t n, uint8_t **out_data, size_t *out_size);
exr_result exr_zip_compress(const exr_allocator *a, const uint8_t *src,
                            size_t n, uint8_t **out_data, size_t *out_size);
exr_result exr_piz_compress(const exr_codec_ctx *ctx, const uint8_t *block,
                            size_t n, uint8_t **out_data, size_t *out_size);
exr_result exr_pxr24_compress(const exr_codec_ctx *ctx, const uint8_t *block,
                              size_t n, uint8_t **out_data, size_t *out_size);
exr_result exr_b44_compress(const exr_codec_ctx *ctx, const uint8_t *block,
                            size_t n, uint8_t **out_data, size_t *out_size,
                            int optimize_flat);
/* Test hook: force B44 perceptual-table init and expose the two tables so a
 * hosted test can verify them bit-for-bit against a libm reference. */
void exr_b44_debug_tables(const uint16_t **exp_tbl, const uint16_t **log_tbl);
exr_result exr_zstd_compress(const exr_allocator *a, const uint8_t *src,
                             size_t n, uint8_t **out_data, size_t *out_size);
exr_result exr_jph_forward_53_2d_i32(const exr_allocator *a,
                                      int32_t *data, size_t width,
                                      size_t height, unsigned levels);
exr_result exr_jph_forward_rct_i32(int32_t *c0, int32_t *c1, int32_t *c2,
                                   size_t count);
exr_result exr_jph_forward_nlt_type3_i32(int32_t *data, size_t count,
                                         uint32_t bit_depth);
exr_result exr_jph_compress(const exr_codec_ctx *ctx, const uint8_t *block,
                            size_t n, uint8_t **out_data, size_t *out_size);

/* Build the HTJ2K encode VLC/UVLC tables up front. Call once on a single thread
 * before compressing chunks concurrently: the lazy table init in exr_jph_compress
 * is not thread-safe (sets its ready flag before the table is built), so the
 * parallel encode path warms it here instead of racing in the workers. */
void exr_jph_warmup_encode_tables(void);

/* Encode dispatch: compress one canonical block per ctx->compression. */
exr_result exr_compress_block(const exr_codec_ctx *ctx, const uint8_t *block,
                              size_t n, uint8_t **out_data, size_t *out_size);

#endif /* TINYEXR_INTERNAL_H_ */
