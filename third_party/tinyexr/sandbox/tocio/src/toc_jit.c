/*
 * tocio - JIT backend (x86-64 SSE2/AVX + AArch64 NEON). Emits machine code for
 * an op chain into executable memory and runs it directly (no disk / dlopen).
 * Hosted-only (needs OS executable memory via mmap); excluded from the
 * freestanding core.
 *
 * The emitted function is `void fn(float *rgba, size_t npix)` over interleaved
 * RGBA. matrix/range are inlined as SIMD (bit-exact with the matching
 * interpreter tier); every other op calls toc_apply_op_pixel(op, px, ch) with
 * the op's address baked in. `channels` is baked at compile time.
 *
 * Copyright (c) 2014-2026 Syoyo Fujita and TinyEXR authors
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "toc_internal.h"

#if defined(__x86_64__) || defined(_M_X64)

#include <stddef.h>

#if defined(__unix__) || defined(__APPLE__)
#include <sys/mman.h>
#define TOC_HAVE_MMAP 1
#endif

#if defined(__GNUC__) || defined(__clang__)
#include <cpuid.h>
#endif

struct toc_jit {
    void *mem;     /* executable mapping */
    size_t mapsz;  /* rounded map size */
    toc_jit_fn fn;
    toc_allocator alloc;
};

/* ---- growable code buffer ------------------------------------------------ */
typedef struct {
    uint8_t *buf;
    size_t len, cap;
    const toc_allocator *a;
    int oom;
} codebuf;

static int cb_reserve(codebuf *c, size_t extra) {
    if (c->oom) return 0;
    if (c->len + extra <= c->cap) return 1;
    {
        size_t ncap = c->cap ? c->cap * 2 : 1024;
        uint8_t *n;
        while (ncap < c->len + extra) ncap *= 2;
        n = (uint8_t *)toc_malloc(c->a, ncap);
        if (!n) { c->oom = 1; return 0; }
        if (c->buf) { memcpy(n, c->buf, c->len); toc_free(c->a, c->buf); }
        c->buf = n;
        c->cap = ncap;
    }
    return 1;
}
static void e1(codebuf *c, unsigned b) {
    if (cb_reserve(c, 1)) c->buf[c->len++] = (uint8_t)b;
}
static void en(codebuf *c, const uint8_t *p, size_t n) {
    if (cb_reserve(c, n)) { memcpy(c->buf + c->len, p, n); c->len += n; }
}
static void e4(codebuf *c, uint32_t v) {
    uint8_t b[4] = {(uint8_t)v, (uint8_t)(v >> 8), (uint8_t)(v >> 16),
                    (uint8_t)(v >> 24)};
    en(c, b, 4);
}
static void e8(codebuf *c, uint64_t v) {
    uint8_t b[8];
    int i;
    for (i = 0; i < 8; ++i) b[i] = (uint8_t)(v >> (8 * i));
    en(c, b, 8);
}

/* ---- instruction emitters (SSE2; xmm regs limited to 0,1,5,6 = no REX) --- */
static void mov_rax_imm64(codebuf *c, uint64_t v) { e1(c, 0x48); e1(c, 0xB8); e8(c, v); }
static void mov_rdi_imm64(codebuf *c, uint64_t v) { e1(c, 0x48); e1(c, 0xBF); e8(c, v); }
static void mov_edx_imm32(codebuf *c, uint32_t v) { e1(c, 0xBA); e4(c, v); }
static void mov_rsi_r12(codebuf *c) { e1(c, 0x4C); e1(c, 0x89); e1(c, 0xE6); }
static void call_rax(codebuf *c) { e1(c, 0xFF); e1(c, 0xD0); }

/* movups xmm<reg>, [rax + disp32] */
static void ld_xmm_rax(codebuf *c, int reg, uint32_t disp) {
    e1(c, 0x0F); e1(c, 0x10);
    e1(c, 0x80u | ((unsigned)reg << 3)); /* mod=10, rm=000 (rax) */
    e4(c, disp);
}
/* movups xmm<reg>, [r12]  /  movups [r12], xmm<reg> */
static void ld_xmm_r12(codebuf *c, int reg) {
    e1(c, 0x41); e1(c, 0x0F); e1(c, 0x10);
    e1(c, 0x04u | ((unsigned)reg << 3)); e1(c, 0x24);
}
static void st_xmm_r12(codebuf *c, int reg) {
    e1(c, 0x41); e1(c, 0x0F); e1(c, 0x11);
    e1(c, 0x04u | ((unsigned)reg << 3)); e1(c, 0x24);
}
static void pshufd(codebuf *c, int dst, int src, unsigned imm) {
    e1(c, 0x66); e1(c, 0x0F); e1(c, 0x70);
    e1(c, 0xC0u | ((unsigned)dst << 3) | (unsigned)src); e1(c, imm);
}
static void sse_rr(codebuf *c, unsigned op2, int dst, int src) {
    e1(c, 0x0F); e1(c, op2);
    e1(c, 0xC0u | ((unsigned)dst << 3) | (unsigned)src);
}
/* 66-prefixed packed-integer reg,reg form. */
static void sse66_rr(codebuf *c, unsigned op2, int dst, int src) {
    e1(c, 0x66); e1(c, 0x0F); e1(c, op2);
    e1(c, 0xC0u | ((unsigned)dst << 3) | (unsigned)src);
}
#define mulps(c, d, s) sse_rr(c, 0x59, d, s)
#define addps(c, d, s) sse_rr(c, 0x58, d, s)
#define subps(c, d, s) sse_rr(c, 0x5C, d, s)
#define divps(c, d, s) sse_rr(c, 0x5E, d, s)
#define maxps(c, d, s) sse_rr(c, 0x5F, d, s)
#define minps(c, d, s) sse_rr(c, 0x5D, d, s)
#define movaps(c, d, s) sse_rr(c, 0x28, d, s)
#define cvtdq2ps(c, d, s) sse_rr(c, 0x5B, d, s)
#define cvtps2dq(c, d, s) sse66_rr(c, 0x5B, d, s)
#define pand(c, d, s) sse66_rr(c, 0xDB, d, s)
#define por(c, d, s) sse66_rr(c, 0xEB, d, s)
#define paddd(c, d, s) sse66_rr(c, 0xFE, d, s)
#define psubd(c, d, s) sse66_rr(c, 0xFA, d, s)
/* psrld/pslld xmm, imm8 (66 0F 72 /2 or /6) */
static void psrld_i(codebuf *c, int reg, unsigned imm) {
    e1(c, 0x66); e1(c, 0x0F); e1(c, 0x72);
    e1(c, 0xD0u | (unsigned)reg); e1(c, imm);
}
static void pslld_i(codebuf *c, int reg, unsigned imm) {
    e1(c, 0x66); e1(c, 0x0F); e1(c, 0x72);
    e1(c, 0xF0u | (unsigned)reg); e1(c, imm);
}
/* broadcast a 32-bit constant to all 4 lanes of xmm<reg> via eax. */
static void bcast_bits(codebuf *c, int reg, uint32_t bits) {
    e1(c, 0xB8); e4(c, bits);                 /* mov eax, imm32 */
    e1(c, 0x66); e1(c, 0x0F); e1(c, 0x6E);    /* movd xmm<reg>, eax */
    e1(c, 0xC0u | ((unsigned)reg << 3));
    e1(c, 0x66); e1(c, 0x0F); e1(c, 0x70);    /* pshufd xmm,xmm,0 */
    e1(c, 0xC0u | ((unsigned)reg << 3) | (unsigned)reg); e1(c, 0x00);
}
static void bcast_f(codebuf *c, int reg, float v) {
    uint32_t u;
    memcpy(&u, &v, 4);
    bcast_bits(c, reg, u);
}

static uint64_t addr_of(const void *p) { return (uint64_t)(uintptr_t)p; }

/* 4-wide log2 in place on xmm1; scratch xmm2,xmm3,xmm4,xmm7. Matches the
 * scalar toc_log2f formulation (atanh series). */
static void emit_log2_xmm1(codebuf *c) {
    movaps(c, 2, 1);                 /* xmm2 = bits copy for exponent */
    psrld_i(c, 2, 23);
    bcast_bits(c, 7, 0xffu); pand(c, 2, 7);
    bcast_bits(c, 7, 127u);  psubd(c, 2, 7);
    cvtdq2ps(c, 2, 2);               /* xmm2 = e_f */
    bcast_bits(c, 7, 0x7fffffu); pand(c, 1, 7);
    bcast_bits(c, 7, 0x3f800000u); por(c, 1, 7);  /* xmm1 = m in [1,2) */
    bcast_f(c, 7, 1.0f);
    movaps(c, 3, 1); subps(c, 3, 7); /* m-1 */
    addps(c, 1, 7);                  /* m+1 */
    divps(c, 3, 1);                  /* xmm3 = t */
    movaps(c, 4, 3); mulps(c, 4, 4); /* xmm4 = t2 */
    bcast_f(c, 1, 1.0f / 9.0f);
    mulps(c, 1, 4); bcast_f(c, 7, 1.0f / 7.0f); addps(c, 1, 7);
    mulps(c, 1, 4); bcast_f(c, 7, 1.0f / 5.0f); addps(c, 1, 7);
    mulps(c, 1, 4); bcast_f(c, 7, 1.0f / 3.0f); addps(c, 1, 7);
    mulps(c, 1, 4); bcast_f(c, 7, 1.0f);        addps(c, 1, 7); /* poly */
    bcast_f(c, 7, 2.0f); mulps(c, 3, 7);        /* 2t */
    mulps(c, 1, 3);                              /* ln = poly*2t */
    bcast_f(c, 7, 1.4426950408889634f); mulps(c, 1, 7);
    addps(c, 1, 2);                  /* xmm1 = log2 */
}

/* 4-wide exp2 in place on xmm1; scratch xmm2,xmm3,xmm4,xmm5,xmm7. */
static void emit_exp2_xmm1(codebuf *c) {
    bcast_f(c, 7, 127.0f);  minps(c, 1, 7);
    bcast_f(c, 7, -126.0f); maxps(c, 1, 7);
    cvtps2dq(c, 2, 1);               /* xmm2 = k_int (round nearest) */
    cvtdq2ps(c, 3, 2);               /* xmm3 = k_f */
    movaps(c, 4, 1); subps(c, 4, 3); /* xmm4 = f */
    bcast_f(c, 7, 0.6931471805599453f); mulps(c, 4, 7); /* g = f*ln2 */
    bcast_f(c, 5, 1.0f / 720.0f);
    mulps(c, 5, 4); bcast_f(c, 7, 1.0f / 120.0f); addps(c, 5, 7);
    mulps(c, 5, 4); bcast_f(c, 7, 1.0f / 24.0f);  addps(c, 5, 7);
    mulps(c, 5, 4); bcast_f(c, 7, 1.0f / 6.0f);   addps(c, 5, 7);
    mulps(c, 5, 4); bcast_f(c, 7, 0.5f);          addps(c, 5, 7);
    mulps(c, 5, 4); bcast_f(c, 7, 1.0f);          addps(c, 5, 7);
    mulps(c, 5, 4); bcast_f(c, 7, 1.0f);          addps(c, 5, 7); /* p */
    bcast_bits(c, 7, 127u); paddd(c, 2, 7); pslld_i(c, 2, 23); /* 2^k bits */
    mulps(c, 5, 2);                  /* p * 2^k */
    movaps(c, 1, 5);                 /* xmm1 = exp2 */
}

/* dispatch one op via the SSE (1-pixel) path */
static void emit_op_sse(codebuf *c, const toc_op *op, int channels);

/* EXPONENT (ch==4): v = pow(max(0,v), e[4]) per lane. NOTE bcast_* clobbers
 * eax (= low 32 of rax), so the op pointer is reloaded after the log2 pass. */
static void emit_exponent(codebuf *c, const toc_op *op) {
    uint32_t E = (uint32_t)offsetof(toc_op, u.exponent.e);
    ld_xmm_r12(c, 0);
    bcast_f(c, 7, 0.0f); maxps(c, 0, 7);             /* max(v,0) */
    bcast_f(c, 7, 1.17549435e-38f); maxps(c, 0, 7);  /* >= FLT_MIN for log2 */
    movaps(c, 1, 0);
    emit_log2_xmm1(c);
    mov_rax_imm64(c, addr_of(op));                   /* reload (bcast hit eax) */
    ld_xmm_rax(c, 2, E); mulps(c, 1, 2);             /* y = e*log2(v) */
    emit_exp2_xmm1(c);
    movaps(c, 0, 1);
    st_xmm_r12(c, 0);
}

static void emit_matrix(codebuf *c, const toc_op *op) {
    uint32_t M = (uint32_t)offsetof(toc_op, u.matrix.m);
    uint32_t O = (uint32_t)offsetof(toc_op, u.matrix.off);
    mov_rax_imm64(c, addr_of(op));
    ld_xmm_r12(c, 0);            /* xmm0 = v(r,g,b,a) */
    pshufd(c, 1, 0, 0x00);       /* xmm1 = r */
    ld_xmm_rax(c, 5, M + 0);  mulps(c, 5, 1);
    pshufd(c, 1, 0, 0x55);       /* g */
    ld_xmm_rax(c, 6, M + 16); mulps(c, 6, 1); addps(c, 5, 6);
    pshufd(c, 1, 0, 0xAA);       /* b */
    ld_xmm_rax(c, 6, M + 32); mulps(c, 6, 1); addps(c, 5, 6);
    pshufd(c, 1, 0, 0xFF);       /* a */
    ld_xmm_rax(c, 6, M + 48); mulps(c, 6, 1); addps(c, 5, 6);
    ld_xmm_rax(c, 6, O);      addps(c, 5, 6);
    st_xmm_r12(c, 5);
}

/* ---- AVX (256-bit, 2 pixels/iter) encoders + kernels -------------------- */
/* vmovups ymm<r>,[r12] / [r12],ymm<r> (3-byte VEX, B bit for r12 base) */
static void vld_ymm_r12(codebuf *c, int r) {
    e1(c, 0xC4); e1(c, 0xC1); e1(c, 0x7C); e1(c, 0x10);
    e1(c, 0x04u | ((unsigned)r << 3)); e1(c, 0x24);
}
static void vst_ymm_r12(codebuf *c, int r) {
    e1(c, 0xC4); e1(c, 0xC1); e1(c, 0x7C); e1(c, 0x11);
    e1(c, 0x04u | ((unsigned)r << 3)); e1(c, 0x24);
}
/* vbroadcastf128 ymm<r>, [rax+disp32] */
static void vbcast128_rax(codebuf *c, int r, uint32_t disp) {
    e1(c, 0xC4); e1(c, 0xE2); e1(c, 0x7D); e1(c, 0x1A);
    e1(c, 0x80u | ((unsigned)r << 3)); e4(c, disp);
}
/* vpermilps ymm<d>, ymm<s>, imm8 */
static void vpermilps(codebuf *c, int d, int s, unsigned imm) {
    e1(c, 0xC4); e1(c, 0xE3); e1(c, 0x7D); e1(c, 0x04);
    e1(c, 0xC0u | ((unsigned)d << 3) | (unsigned)s); e1(c, imm);
}
/* 3-operand 256-bit float op (C5 form): dst = op(src1, src2). */
static void vex_rrr(codebuf *c, unsigned op2, int d, int s1, int s2) {
    e1(c, 0xC5);
    e1(c, 0x80u | (((~(unsigned)s1) & 0xfu) << 3) | 0x04u); /* vvvv=~s1, L=1 */
    e1(c, op2);
    e1(c, 0xC0u | ((unsigned)d << 3) | (unsigned)s2);
}
#define vmulps(c, d, a, b) vex_rrr(c, 0x59, d, a, b)
#define vaddps(c, d, a, b) vex_rrr(c, 0x58, d, a, b)
#define vmaxps(c, d, a, b) vex_rrr(c, 0x5F, d, a, b)
#define vminps(c, d, a, b) vex_rrr(c, 0x5D, d, a, b)
static void vzeroupper(codebuf *c) { e1(c, 0xC5); e1(c, 0xF8); e1(c, 0x77); }

static void emit_matrix_avx2(codebuf *c, const toc_op *op) {
    uint32_t M = (uint32_t)offsetof(toc_op, u.matrix.m);
    uint32_t O = (uint32_t)offsetof(toc_op, u.matrix.off);
    mov_rax_imm64(c, addr_of(op));
    vld_ymm_r12(c, 0);                 /* ymm0 = 2 pixels */
    vpermilps(c, 1, 0, 0x00); vbcast128_rax(c, 5, M + 0);  vmulps(c, 5, 5, 1);
    vpermilps(c, 1, 0, 0x55); vbcast128_rax(c, 6, M + 16); vmulps(c, 6, 6, 1); vaddps(c, 5, 5, 6);
    vpermilps(c, 1, 0, 0xAA); vbcast128_rax(c, 6, M + 32); vmulps(c, 6, 6, 1); vaddps(c, 5, 5, 6);
    vpermilps(c, 1, 0, 0xFF); vbcast128_rax(c, 6, M + 48); vmulps(c, 6, 6, 1); vaddps(c, 5, 5, 6);
    vbcast128_rax(c, 6, O); vaddps(c, 5, 5, 6);
    vst_ymm_r12(c, 5);
}

static void emit_range_avx2(codebuf *c, const toc_op *op) {
    uint32_t S = (uint32_t)offsetof(toc_op, u.range.scale);
    uint32_t OF = (uint32_t)offsetof(toc_op, u.range.offset);
    uint32_t MN = (uint32_t)offsetof(toc_op, u.range.min);
    uint32_t MX = (uint32_t)offsetof(toc_op, u.range.max);
    mov_rax_imm64(c, addr_of(op));
    vld_ymm_r12(c, 0);
    vbcast128_rax(c, 1, S);  vmulps(c, 0, 0, 1);
    vbcast128_rax(c, 1, OF); vaddps(c, 0, 0, 1);
    if (op->u.range.clamp_lo) { vbcast128_rax(c, 1, MN); vmaxps(c, 0, 0, 1); }
    if (op->u.range.clamp_hi) { vbcast128_rax(c, 1, MX); vminps(c, 0, 0, 1); }
    vst_ymm_r12(c, 0);
}

#if defined(__GNUC__) || defined(__clang__)
static int host_has_avx(void) {
    unsigned a, b, cc, d, eax, edx;
    if (!__get_cpuid(1, &a, &b, &cc, &d)) return 0;
    if (!(cc & (1u << 27)) || !(cc & (1u << 28))) return 0; /* OSXSAVE + AVX */
    __asm__ volatile("xgetbv" : "=a"(eax), "=d"(edx) : "c"(0));
    return ((eax & 0x6u) == 0x6u) ? 1 : 0; /* XMM+YMM enabled */
}
#else
static int host_has_avx(void) { return 0; }
#endif

static void emit_range(codebuf *c, const toc_op *op) {
    uint32_t S = (uint32_t)offsetof(toc_op, u.range.scale);
    uint32_t OF = (uint32_t)offsetof(toc_op, u.range.offset);
    uint32_t MN = (uint32_t)offsetof(toc_op, u.range.min);
    uint32_t MX = (uint32_t)offsetof(toc_op, u.range.max);
    mov_rax_imm64(c, addr_of(op));
    ld_xmm_r12(c, 0);
    ld_xmm_rax(c, 1, S);  mulps(c, 0, 1);
    ld_xmm_rax(c, 1, OF); addps(c, 0, 1);
    if (op->u.range.clamp_lo) { ld_xmm_rax(c, 1, MN); maxps(c, 0, 1); }
    if (op->u.range.clamp_hi) { ld_xmm_rax(c, 1, MX); minps(c, 0, 1); }
    st_xmm_r12(c, 0);
}

static void patch_rel32(codebuf *c, size_t site, size_t target) {
    int32_t r = (int32_t)((int64_t)target - (int64_t)(site + 4));
    if (c->oom) return;
    c->buf[site] = (uint8_t)r; c->buf[site + 1] = (uint8_t)(r >> 8);
    c->buf[site + 2] = (uint8_t)(r >> 16); c->buf[site + 3] = (uint8_t)(r >> 24);
}

/* call toc_apply_op_pixel(op, r12, ch) */
static void emit_helper_call(codebuf *c, const toc_op *op, int ch) {
    mov_rdi_imm64(c, addr_of(op));
    mov_rsi_r12(c);
    mov_edx_imm32(c, (uint32_t)ch);
    mov_rax_imm64(c, addr_of((const void *)&toc_apply_op_pixel));
    call_rax(c);
}

static void emit_op_sse(codebuf *c, const toc_op *op, int channels) {
    if (channels == 4 && op->kind == TOC_OP_MATRIX) emit_matrix(c, op);
    else if (channels == 4 && op->kind == TOC_OP_RANGE) emit_range(c, op);
    else if (channels == 4 && op->kind == TOC_OP_EXPONENT) emit_exponent(c, op);
    else if (op->kind != TOC_OP_NOOP) emit_helper_call(c, op, channels);
}

toc_result toc_jit_compile(const toc_op_list *ops, int channels,
                           const toc_allocator *a, toc_jit **out) {
    codebuf c;
    size_t k;
    int avx_pure;
    toc_jit *j;
#if !defined(TOC_HAVE_MMAP)
    (void)ops; (void)channels; (void)a; (void)out;
    return TOC_ERROR_UNSUPPORTED;
#else
    size_t pgsz = 4096, mapsz;
    void *mem;
    if (!ops || !out || (channels != 3 && channels != 4))
        return TOC_ERROR_INVALID_ARGUMENT;
    if (!a) a = toc_default_allocator();
    *out = NULL;
    memset(&c, 0, sizeof(c));
    c.a = a;

    /* prologue: save callee-saved, load args, align stack for calls.
     * SysV entry rsp%16==8; push r12,r13 then sub 8 -> rsp%16==0. */
    e1(&c, 0x41); e1(&c, 0x54);            /* push r12 */
    e1(&c, 0x41); e1(&c, 0x55);            /* push r13 */
    e1(&c, 0x48); e1(&c, 0x83); e1(&c, 0xEC); e1(&c, 0x08); /* sub rsp,8 */
    e1(&c, 0x49); e1(&c, 0x89); e1(&c, 0xFC); /* mov r12, rdi (px = rgba) */
    e1(&c, 0x49); e1(&c, 0x89); e1(&c, 0xF5); /* mov r13, rsi (counter = npix) */

    /* Use the AVX 2-pixel path only for pure matrix/range pipelines (the common
     * colorspace-conversion case): it avoids mixing AVX/SSE inline and keeps the
     * inline-pow SSE path for everything else. */
    avx_pure = (channels == 4 && host_has_avx());
    for (k = 0; avx_pure && k < ops->count; ++k) {
        toc_op_kind kk = ops->ops[k].kind;
        if (kk != TOC_OP_MATRIX && kk != TOC_OP_RANGE && kk != TOC_OP_NOOP)
            avx_pure = 0;
    }

    if (avx_pure) {
        size_t main_pos, jb_at, jmp_at, jz_at;
        main_pos = c.len;
        e1(&c, 0x49); e1(&c, 0x83); e1(&c, 0xFD); e1(&c, 0x02); /* cmp r13,2 */
        e1(&c, 0x0F); e1(&c, 0x82); jb_at = c.len; e4(&c, 0);   /* jb -> tail */
        for (k = 0; k < ops->count; ++k) {
            const toc_op *op = &ops->ops[k];
            if (op->kind == TOC_OP_MATRIX) emit_matrix_avx2(&c, op);
            else if (op->kind == TOC_OP_RANGE) emit_range_avx2(&c, op);
        }
        e1(&c, 0x49); e1(&c, 0x83); e1(&c, 0xC4); e1(&c, 0x20); /* add r12,32 */
        e1(&c, 0x49); e1(&c, 0x83); e1(&c, 0xED); e1(&c, 0x02); /* sub r13,2 */
        e1(&c, 0xE9); jmp_at = c.len; e4(&c, 0);                /* jmp -> main */
        patch_rel32(&c, jmp_at, main_pos);
        patch_rel32(&c, jb_at, c.len);                          /* tail: */
        e1(&c, 0x4D); e1(&c, 0x85); e1(&c, 0xED);               /* test r13,r13 */
        e1(&c, 0x0F); e1(&c, 0x84); jz_at = c.len; e4(&c, 0);   /* jz -> done */
        for (k = 0; k < ops->count; ++k) emit_op_sse(&c, &ops->ops[k], channels);
        patch_rel32(&c, jz_at, c.len);                          /* done: */
        vzeroupper(&c);
    } else {
        size_t loop_pos, jz_at, jmp_at;
        loop_pos = c.len;
        e1(&c, 0x4D); e1(&c, 0x85); e1(&c, 0xED);               /* test r13,r13 */
        e1(&c, 0x0F); e1(&c, 0x84); jz_at = c.len; e4(&c, 0);   /* jz -> done */
        for (k = 0; k < ops->count; ++k) emit_op_sse(&c, &ops->ops[k], channels);
        e1(&c, 0x49); e1(&c, 0x83); e1(&c, 0xC4);               /* add r12,imm8 */
        e1(&c, (unsigned)(channels * 4));
        e1(&c, 0x49); e1(&c, 0xFF); e1(&c, 0xCD);               /* dec r13 */
        e1(&c, 0xE9); jmp_at = c.len; e4(&c, 0);                /* jmp -> loop */
        patch_rel32(&c, jmp_at, loop_pos);
        patch_rel32(&c, jz_at, c.len);                          /* done: */
    }

    e1(&c, 0x48); e1(&c, 0x83); e1(&c, 0xC4); e1(&c, 0x08); /* add rsp,8 */
    e1(&c, 0x41); e1(&c, 0x5D);            /* pop r13 */
    e1(&c, 0x41); e1(&c, 0x5C);            /* pop r12 */
    e1(&c, 0xC3);                          /* ret */

    if (c.oom) { toc_free(a, c.buf); return TOC_ERROR_OUT_OF_MEMORY; }

    /* copy into an executable mapping (W^X) */
    mapsz = (c.len + pgsz - 1) & ~(pgsz - 1);
    mem = mmap(NULL, mapsz, PROT_READ | PROT_WRITE,
               MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (mem == MAP_FAILED) { toc_free(a, c.buf); return TOC_ERROR_UNSUPPORTED; }
    memcpy(mem, c.buf, c.len);
    toc_free(a, c.buf);
    if (mprotect(mem, mapsz, PROT_READ | PROT_EXEC) != 0) {
        munmap(mem, mapsz);
        return TOC_ERROR_UNSUPPORTED;
    }
    j = (toc_jit *)toc_malloc(a, sizeof(*j));
    if (!j) { munmap(mem, mapsz); return TOC_ERROR_OUT_OF_MEMORY; }
    j->mem = mem;
    j->mapsz = mapsz;
    j->alloc = *a;
    memcpy(&j->fn, &mem, sizeof(void *)); /* avoid object<->fn ptr warning */
    *out = j;
    return TOC_SUCCESS;
#endif
}

toc_jit_fn toc_jit_func(const toc_jit *j) { return j ? j->fn : NULL; }

void toc_jit_destroy(toc_jit *j) {
    toc_allocator a;
    if (!j) return;
    a = j->alloc;
#if defined(TOC_HAVE_MMAP)
    if (j->mem) munmap(j->mem, j->mapsz);
#endif
    toc_free(&a, j);
}

#elif defined(__aarch64__) || defined(_M_ARM64)

/* ============================================================================
 * AArch64 (ARM64) JIT backend. Emits A64 machine code for an op chain into
 * executable memory and runs it directly. Mirrors the x86-64 backend: the
 * emitted `void fn(float *rgba, size_t npix)` runs a per-pixel loop with
 * `channels` baked in. For channels==4, MATRIX and RANGE are inlined as NEON
 * (bit-exact with the NEON interpreter tier: same dup+fmul+fadd order, no FMA
 * contraction); every other op (and all of channels==3, to avoid a 16-byte
 * vector touching past a 12-byte pixel) is a `blr` to toc_apply_op_pixel(op,
 * px, ch) with the op address baked as a movz/movk imm64.
 *
 * ABI (AAPCS64): args x0=rgba, x1=npix. rgba/counter live in callee-saved
 * x19/x20 so they survive helper calls; x9 is a scratch for the op/helper
 * address. The stack stays 16-byte aligned for the calls.
 * ========================================================================== */

#include <stddef.h>

#if defined(__unix__) || defined(__APPLE__)
#include <sys/mman.h>
#define TOC_HAVE_MMAP 1
#endif
#if defined(__APPLE__)
#include <libkern/OSCacheControl.h> /* sys_icache_invalidate */
#include <pthread.h>                /* pthread_jit_write_protect_np */
#ifndef MAP_ANONYMOUS
#define MAP_ANONYMOUS MAP_ANON
#endif
#endif

struct toc_jit {
    void *mem;
    size_t mapsz;
    toc_jit_fn fn;
    toc_allocator alloc;
};

/* ---- growable code buffer (32-bit instruction words) --------------------- */
typedef struct {
    uint8_t *buf;
    size_t len, cap;
    const toc_allocator *a;
    int oom;
} codebuf;

static int cb_reserve(codebuf *c, size_t extra) {
    if (c->oom) return 0;
    if (c->len + extra <= c->cap) return 1;
    {
        size_t ncap = c->cap ? c->cap * 2 : 1024;
        uint8_t *n;
        while (ncap < c->len + extra) ncap *= 2;
        n = (uint8_t *)toc_malloc(c->a, ncap);
        if (!n) { c->oom = 1; return 0; }
        if (c->buf) { memcpy(n, c->buf, c->len); toc_free(c->a, c->buf); }
        c->buf = n;
        c->cap = ncap;
    }
    return 1;
}
/* emit one little-endian 32-bit A64 instruction word */
static void e32(codebuf *c, uint32_t w) {
    if (cb_reserve(c, 4)) {
        c->buf[c->len++] = (uint8_t)w;
        c->buf[c->len++] = (uint8_t)(w >> 8);
        c->buf[c->len++] = (uint8_t)(w >> 16);
        c->buf[c->len++] = (uint8_t)(w >> 24);
    }
}

static uint64_t addr_of(const void *p) { return (uint64_t)(uintptr_t)p; }

/* ---- instruction encoders ------------------------------------------------ */
/* mov Xd, Xm  (ORR Xd, XZR, Xm) */
static void emit_mov_reg(codebuf *c, int xd, int xm) {
    e32(c, 0xAA0003E0u | ((unsigned)xm << 16) | (unsigned)xd);
}
static void emit_movz_x(codebuf *c, int xd, unsigned imm16, unsigned hw) {
    e32(c, 0xD2800000u | (hw << 21) | (imm16 << 5) | (unsigned)xd);
}
static void emit_movk_x(codebuf *c, int xd, unsigned imm16, unsigned hw) {
    e32(c, 0xF2800000u | (hw << 21) | (imm16 << 5) | (unsigned)xd);
}
/* load a full 64-bit immediate into Xd via movz + 3x movk */
static void emit_load_imm64(codebuf *c, int xd, uint64_t v) {
    emit_movz_x(c, xd, (unsigned)(v & 0xffffu), 0);
    emit_movk_x(c, xd, (unsigned)((v >> 16) & 0xffffu), 1);
    emit_movk_x(c, xd, (unsigned)((v >> 32) & 0xffffu), 2);
    emit_movk_x(c, xd, (unsigned)((v >> 48) & 0xffffu), 3);
}
/* mov Wd, #imm16 (movz, 32-bit) */
static void emit_movz_w(codebuf *c, int wd, unsigned imm16) {
    e32(c, 0x52800000u | (imm16 << 5) | (unsigned)wd);
}
/* LDUR/STUR Qt, [Xn, #simm9]  (unscaled byte offset; any field offset fits) */
static void emit_ldur_q(codebuf *c, int qt, int xn, int off) {
    e32(c, 0x3CC00000u | (((unsigned)off & 0x1ffu) << 12) | ((unsigned)xn << 5) |
               (unsigned)qt);
}
static void emit_stur_q(codebuf *c, int qt, int xn, int off) {
    e32(c, 0x3C800000u | (((unsigned)off & 0x1ffu) << 12) | ((unsigned)xn << 5) |
               (unsigned)qt);
}
/* DUP Vd.4S, Vn.S[idx] */
static void emit_dup_s(codebuf *c, int vd, int vn, int idx) {
    unsigned imm5 = ((unsigned)idx << 3) | 4u;
    e32(c, 0x4E000400u | (imm5 << 16) | ((unsigned)vn << 5) | (unsigned)vd);
}
/* three-same FP ops on .4S */
static void emit_fmul(codebuf *c, int vd, int vn, int vm) {
    e32(c, 0x6E20DC00u | ((unsigned)vm << 16) | ((unsigned)vn << 5) | (unsigned)vd);
}
static void emit_fadd(codebuf *c, int vd, int vn, int vm) {
    e32(c, 0x4E20D400u | ((unsigned)vm << 16) | ((unsigned)vn << 5) | (unsigned)vd);
}
static void emit_fmax(codebuf *c, int vd, int vn, int vm) {
    e32(c, 0x4E20F400u | ((unsigned)vm << 16) | ((unsigned)vn << 5) | (unsigned)vd);
}
static void emit_fmin(codebuf *c, int vd, int vn, int vm) {
    e32(c, 0x4EA0F400u | ((unsigned)vm << 16) | ((unsigned)vn << 5) | (unsigned)vd);
}
static void emit_add_imm(codebuf *c, int xd, int xn, unsigned imm12) {
    e32(c, 0x91000000u | ((imm12 & 0xfffu) << 10) | ((unsigned)xn << 5) | (unsigned)xd);
}
static void emit_sub_imm(codebuf *c, int xd, int xn, unsigned imm12) {
    e32(c, 0xD1000000u | ((imm12 & 0xfffu) << 10) | ((unsigned)xn << 5) | (unsigned)xd);
}
static void emit_blr(codebuf *c, int xn) { e32(c, 0xD63F0000u | ((unsigned)xn << 5)); }

/* ---- additional NEON encoders for the inline-pow (log2/exp2) path --------- */
static void emit_fsub(codebuf *c, int vd, int vn, int vm) {
    e32(c, 0x4EA0D400u | ((unsigned)vm << 16) | ((unsigned)vn << 5) | (unsigned)vd);
}
static void emit_fdiv(codebuf *c, int vd, int vn, int vm) {
    e32(c, 0x6E20FC00u | ((unsigned)vm << 16) | ((unsigned)vn << 5) | (unsigned)vd);
}
static void emit_add_i32(codebuf *c, int vd, int vn, int vm) { /* ADD Vd.4S */
    e32(c, 0x4EA08400u | ((unsigned)vm << 16) | ((unsigned)vn << 5) | (unsigned)vd);
}
static void emit_sub_i32(codebuf *c, int vd, int vn, int vm) { /* SUB Vd.4S */
    e32(c, 0x6EA08400u | ((unsigned)vm << 16) | ((unsigned)vn << 5) | (unsigned)vd);
}
static void emit_and16(codebuf *c, int vd, int vn, int vm) { /* AND Vd.16B */
    e32(c, 0x4E201C00u | ((unsigned)vm << 16) | ((unsigned)vn << 5) | (unsigned)vd);
}
static void emit_orr16(codebuf *c, int vd, int vn, int vm) { /* ORR Vd.16B */
    e32(c, 0x4EA01C00u | ((unsigned)vm << 16) | ((unsigned)vn << 5) | (unsigned)vd);
}
static void emit_mov_v(codebuf *c, int vd, int vn) { emit_orr16(c, vd, vn, vn); }
static void emit_ushr_i32(codebuf *c, int vd, int vn, int sh) { /* USHR Vd.4S,#sh */
    e32(c, 0x6F000400u | (((unsigned)(64 - sh)) << 16) | ((unsigned)vn << 5) |
               (unsigned)vd);
}
static void emit_shl_i32(codebuf *c, int vd, int vn, int sh) { /* SHL Vd.4S,#sh */
    e32(c, 0x4F005400u | (((unsigned)(32 + sh)) << 16) | ((unsigned)vn << 5) |
               (unsigned)vd);
}
static void emit_scvtf(codebuf *c, int vd, int vn) { /* SCVTF Vd.4S (int->float) */
    e32(c, 0x4E21D800u | ((unsigned)vn << 5) | (unsigned)vd);
}
static void emit_fcvtns(codebuf *c, int vd, int vn) { /* FCVTNS Vd.4S (round nearest) */
    e32(c, 0x4E21A800u | ((unsigned)vn << 5) | (unsigned)vd);
}
static void emit_movk_w(codebuf *c, int wd, unsigned imm16, unsigned hw) {
    e32(c, 0x72800000u | (hw << 21) | (imm16 << 5) | (unsigned)wd);
}
static void emit_dup_w(codebuf *c, int vd, int wn) { /* DUP Vd.4S, Wn */
    e32(c, 0x4E040C00u | ((unsigned)wn << 5) | (unsigned)vd);
}
static void emit_ins_lane3(codebuf *c, int vd, int vn) { /* INS Vd.S[3], Vn.S[3] */
    e32(c, 0x6E1C6400u | ((unsigned)vn << 5) | (unsigned)vd);
}
static void emit_fcmgt(codebuf *c, int vd, int vn, int vm) { /* Vd.4S = Vn > Vm */
    e32(c, 0x6EA0E400u | ((unsigned)vm << 16) | ((unsigned)vn << 5) | (unsigned)vd);
}
static void emit_fcmge0(codebuf *c, int vd, int vn) { /* Vd.4S = Vn >= 0.0 */
    e32(c, 0x6EA0C800u | ((unsigned)vn << 5) | (unsigned)vd);
}
/* BSL Vd.16B, Vn.16B, Vm.16B: Vd = (Vn & Vd) | (Vm & ~Vd) -- Vd is the mask in
 * and the result out, so result = mask ? Vn : Vm. */
static void emit_bsl(codebuf *c, int vd, int vn, int vm) {
    e32(c, 0x6E601C00u | ((unsigned)vm << 16) | ((unsigned)vn << 5) | (unsigned)vd);
}
static void emit_faddp_v(codebuf *c, int vd, int vn, int vm) { /* FADDP Vd.4S */
    e32(c, 0x6E20D400u | ((unsigned)vm << 16) | ((unsigned)vn << 5) | (unsigned)vd);
}
static void emit_faddp_s(codebuf *c, int sd, int vn) { /* FADDP Sd, Vn.2S */
    e32(c, 0x7E30D800u | ((unsigned)vn << 5) | (unsigned)sd);
}
static void emit_ins_s_w(codebuf *c, int vd, int idx, int wn) { /* INS Vd.S[idx], Wn */
    e32(c, 0x4E001C00u | ((((unsigned)idx << 3) | 4u) << 16) | ((unsigned)wn << 5) |
               (unsigned)vd);
}
static void emit_set_lane_f(codebuf *c, int vd, int idx, float v) {
    uint32_t u;
    memcpy(&u, &v, 4);
    emit_movz_w(c, 10, u & 0xffffu);
    emit_movk_w(c, 10, (u >> 16) & 0xffffu, 1);
    emit_ins_s_w(c, vd, idx, 10);
}
/* broadcast a 32-bit constant to all lanes of Vd via the scratch w10 (so x9,
 * holding the op address, is preserved across the polynomial unlike the x86
 * path which reloads it after clobbering eax). */
static void emit_bcast_bits(codebuf *c, int vd, uint32_t bits) {
    emit_movz_w(c, 10, bits & 0xffffu);
    emit_movk_w(c, 10, (bits >> 16) & 0xffffu, 1);
    emit_dup_w(c, vd, 10);
}
static void emit_bcast_f(codebuf *c, int vd, float v) {
    uint32_t u;
    memcpy(&u, &v, 4);
    emit_bcast_bits(c, vd, u);
}

/* The inline-pow polynomial constants are hoisted into v8..v28 ONCE before the
 * per-pixel loop (emit_pow_consts) and referenced from there, so the hot loop
 * does no movz/movk/dup per pixel. v8..v15 are AAPCS64 callee-saved (low 64b),
 * so the compile path that uses pow saves/restores d8..d15. Working scratch for
 * the kernels stays in v0..v5; matrix/range only touch v0..v6, so the constants
 * survive across other ops in the same loop body. */
enum {
    K_ZERO = 8, K_FLTMIN, K_0xFF, K_127I, K_MANT, K_EXP1, K_1F, K_1_9, K_1_7,
    K_1_5, K_1_3, K_2F, K_LOG2E, K_127F, K_N126F, K_LN2, K_1_720, K_1_120,
    K_1_24, K_1_6, K_HALF /* = v28 */
};
static void emit_pow_consts(codebuf *c) {
    emit_bcast_f(c, K_ZERO, 0.0f);
    emit_bcast_f(c, K_FLTMIN, 1.17549435e-38f);
    emit_bcast_bits(c, K_0xFF, 0xffu);
    emit_bcast_bits(c, K_127I, 127u);
    emit_bcast_bits(c, K_MANT, 0x7fffffu);
    emit_bcast_bits(c, K_EXP1, 0x3f800000u);
    emit_bcast_f(c, K_1F, 1.0f);
    emit_bcast_f(c, K_1_9, 1.0f / 9.0f);
    emit_bcast_f(c, K_1_7, 1.0f / 7.0f);
    emit_bcast_f(c, K_1_5, 1.0f / 5.0f);
    emit_bcast_f(c, K_1_3, 1.0f / 3.0f);
    emit_bcast_f(c, K_2F, 2.0f);
    emit_bcast_f(c, K_LOG2E, 1.4426950408889634f);
    emit_bcast_f(c, K_127F, 127.0f);
    emit_bcast_f(c, K_N126F, -126.0f);
    emit_bcast_f(c, K_LN2, 0.6931471805599453f);
    emit_bcast_f(c, K_1_720, 1.0f / 720.0f);
    emit_bcast_f(c, K_1_120, 1.0f / 120.0f);
    emit_bcast_f(c, K_1_24, 1.0f / 24.0f);
    emit_bcast_f(c, K_1_6, 1.0f / 6.0f);
    emit_bcast_f(c, K_HALF, 0.5f);
}

/* 4-wide log2 in place on v1; scratch v2,v3,v4. Mirrors the scalar toc_log2f
 * (atanh series); uses the hoisted constants, so JIT output matches the
 * interpreter to the test's 1e-6 with no per-pixel constant setup. */
static void emit_log2_neon(codebuf *c) {
    emit_mov_v(c, 2, 1);                                 /* v2 = bits copy */
    emit_ushr_i32(c, 2, 2, 23);
    emit_and16(c, 2, 2, K_0xFF);
    emit_sub_i32(c, 2, 2, K_127I);
    emit_scvtf(c, 2, 2);                                 /* v2 = e_f */
    emit_and16(c, 1, 1, K_MANT);
    emit_orr16(c, 1, 1, K_EXP1);                         /* v1 = m in [1,2) */
    emit_mov_v(c, 3, 1); emit_fsub(c, 3, 3, K_1F);       /* m-1 */
    emit_fadd(c, 1, 1, K_1F);                            /* m+1 */
    emit_fdiv(c, 3, 3, 1);                               /* v3 = t */
    emit_mov_v(c, 4, 3); emit_fmul(c, 4, 4, 4);          /* v4 = t2 */
    emit_mov_v(c, 1, K_1_9);
    emit_fmul(c, 1, 1, 4); emit_fadd(c, 1, 1, K_1_7);
    emit_fmul(c, 1, 1, 4); emit_fadd(c, 1, 1, K_1_5);
    emit_fmul(c, 1, 1, 4); emit_fadd(c, 1, 1, K_1_3);
    emit_fmul(c, 1, 1, 4); emit_fadd(c, 1, 1, K_1F);
    emit_fmul(c, 3, 3, K_2F);                            /* 2t */
    emit_fmul(c, 1, 1, 3);                               /* ln = poly*2t */
    emit_fmul(c, 1, 1, K_LOG2E);
    emit_fadd(c, 1, 1, 2);                               /* v1 = log2 */
}

/* 4-wide exp2 in place on v1; scratch v2,v3,v4,v5. Hoisted constants. */
static void emit_exp2_neon(codebuf *c) {
    emit_fmin(c, 1, 1, K_127F);
    emit_fmax(c, 1, 1, K_N126F);
    emit_fcvtns(c, 2, 1);                                /* v2 = k_int (nearest) */
    emit_scvtf(c, 3, 2);                                 /* v3 = k_f */
    emit_mov_v(c, 4, 1); emit_fsub(c, 4, 4, 3);          /* v4 = f */
    emit_fmul(c, 4, 4, K_LN2);                           /* g = f*ln2 */
    emit_mov_v(c, 5, K_1_720);
    emit_fmul(c, 5, 5, 4); emit_fadd(c, 5, 5, K_1_120);
    emit_fmul(c, 5, 5, 4); emit_fadd(c, 5, 5, K_1_24);
    emit_fmul(c, 5, 5, 4); emit_fadd(c, 5, 5, K_1_6);
    emit_fmul(c, 5, 5, 4); emit_fadd(c, 5, 5, K_HALF);
    emit_fmul(c, 5, 5, 4); emit_fadd(c, 5, 5, K_1F);
    emit_fmul(c, 5, 5, 4); emit_fadd(c, 5, 5, K_1F);     /* p */
    emit_add_i32(c, 2, 2, K_127I); emit_shl_i32(c, 2, 2, 23); /* 2^k bits */
    emit_fmul(c, 5, 5, 2);                               /* p * 2^k */
    emit_mov_v(c, 1, 5);                                 /* v1 = exp2 */
}

/* EXPONENT (ch==4): v = pow(max(0,v), e[4]) per lane, inline. x9 := &op; only
 * the per-op exponent vector is loaded from memory per pixel (the polynomial
 * constants are hoisted). */
static void emit_exponent_arm(codebuf *c, const toc_op *op) {
    int E = (int)offsetof(toc_op, u.exponent.e);
    emit_load_imm64(c, 9, addr_of(op));
    emit_ldur_q(c, 0, 19, 0);
    emit_fmax(c, 0, 0, K_ZERO);                          /* max(v,0) */
    emit_fmax(c, 0, 0, K_FLTMIN);                        /* >= FLT_MIN for log2 */
    emit_mov_v(c, 1, 0);
    emit_log2_neon(c);
    emit_ldur_q(c, 2, 9, E); emit_fmul(c, 1, 1, 2);     /* y = e*log2(v) */
    emit_exp2_neon(c);
    emit_mov_v(c, 0, 1);
    emit_stur_q(c, 0, 19, 0);
}

/* LOG (ch==4): log-affine transform on RGB (alpha preserved), inline. Reuses the
 * hoisted log2/exp2. The per-op log2(base) is materialized into v6 per pixel
 * (cheap on this ~40-instr op). The forward clamp a>0?a:FLT_MIN is approximated
 * as fmax(a,FLT_MIN) -- bit-identical for normal inputs, within the test's 1e-6
 * for the rest. v0 keeps the original pixel for the alpha lane restore. */
static void emit_log_arm(codebuf *c, const toc_op *op) {
    int LS = (int)offsetof(toc_op, u.log.lin_slope);
    int LO = (int)offsetof(toc_op, u.log.lin_offset);
    int GS = (int)offsetof(toc_op, u.log.log_slope);
    int GO = (int)offsetof(toc_op, u.log.log_offset);
    float lb = toc_log2f(op->u.log.base);
    emit_load_imm64(c, 9, addr_of(op));
    emit_ldur_q(c, 0, 19, 0);
    if (!op->u.log.inverse) {
        emit_ldur_q(c, 2, 9, LS); emit_fmul(c, 1, 0, 2);  /* a = x*lin_slope */
        emit_ldur_q(c, 2, 9, LO); emit_fadd(c, 1, 1, 2);  /* + lin_offset */
        emit_fmax(c, 1, 1, K_FLTMIN);                     /* ~ a>0?a:FLT_MIN */
        emit_log2_neon(c);                                /* v1 = log2(a) */
        emit_bcast_f(c, 6, lb); emit_fdiv(c, 1, 1, 6);    /* / log2(base) */
        emit_ldur_q(c, 2, 9, GS); emit_fmul(c, 1, 1, 2);  /* * log_slope */
        emit_ldur_q(c, 2, 9, GO); emit_fadd(c, 1, 1, 2);  /* + log_offset */
    } else {
        emit_ldur_q(c, 2, 9, GO); emit_fsub(c, 1, 0, 2);  /* x - log_offset */
        emit_ldur_q(c, 2, 9, GS); emit_fdiv(c, 1, 1, 2);  /* / log_slope = e */
        emit_bcast_f(c, 6, lb); emit_fmul(c, 1, 1, 6);    /* e * log2(base) */
        emit_exp2_neon(c);                                /* p = base^e */
        emit_ldur_q(c, 2, 9, LO); emit_fsub(c, 1, 1, 2);  /* p - lin_offset */
        emit_ldur_q(c, 2, 9, LS); emit_fdiv(c, 1, 1, 2);  /* / lin_slope */
    }
    emit_ins_lane3(c, 1, 0);                              /* restore alpha */
    emit_stur_q(c, 1, 19, 0);
}

/* EXP_LINEAR (MonCurve, ch==4): power above the breakpoint, linear below, on
 * RGB (alpha preserved). The per-lane branch is a compare (fcmgt) + bit-select
 * (bsl); both branches are computed and then selected. v0 keeps the pixel. */
static void emit_explin_arm(codebuf *c, const toc_op *op) {
    int SC = (int)offsetof(toc_op, u.exp_linear.scale);
    int OF = (int)offsetof(toc_op, u.exp_linear.offset);
    int GM = (int)offsetof(toc_op, u.exp_linear.gamma);
    int BK = (int)offsetof(toc_op, u.exp_linear.breakpoint);
    int SL = (int)offsetof(toc_op, u.exp_linear.slope);
    emit_load_imm64(c, 9, addr_of(op));
    emit_ldur_q(c, 0, 19, 0);
    if (!op->u.exp_linear.inverse) {
        emit_ldur_q(c, 2, 9, SC); emit_fmul(c, 1, 0, 2);  /* base = x*scale */
        emit_ldur_q(c, 2, 9, OF); emit_fadd(c, 1, 1, 2);  /* + offset */
        emit_fmax(c, 1, 1, K_FLTMIN);
        emit_log2_neon(c);
        emit_ldur_q(c, 5, 9, GM); emit_fmul(c, 1, 1, 5);  /* gamma*log2 */
        emit_exp2_neon(c);                                /* v1 = pow */
        emit_ldur_q(c, 2, 9, SL); emit_fmul(c, 5, 0, 2);  /* lin = x*slope */
        emit_ldur_q(c, 2, 9, BK); emit_fcmgt(c, 6, 0, 2); /* mask = x>brk */
        emit_bsl(c, 6, 1, 5);                             /* mask ? pow : lin */
    } else {
        emit_mov_v(c, 1, 0); emit_fmax(c, 1, 1, K_FLTMIN);
        emit_log2_neon(c);
        emit_ldur_q(c, 5, 9, GM);
        emit_fdiv(c, 6, K_1F, 5);                         /* 1/gamma */
        emit_fmul(c, 1, 1, 6);
        emit_exp2_neon(c);                                /* v1 = pow(x,1/g) */
        emit_ldur_q(c, 2, 9, OF); emit_fsub(c, 1, 1, 2);  /* pow - offset */
        emit_ldur_q(c, 2, 9, SC); emit_fdiv(c, 1, 1, 2);  /* / scale */
        emit_ldur_q(c, 2, 9, SL); emit_fdiv(c, 5, 0, 2);  /* lin = x/slope */
        emit_ldur_q(c, 2, 9, BK); emit_ldur_q(c, 3, 9, SL);
        emit_fmul(c, 6, 2, 3);                            /* ybrk = brk*slope */
        emit_fcmgt(c, 6, 0, 6);                           /* mask = x>ybrk */
        emit_bsl(c, 6, 1, 5);                             /* mask ? hi : lin */
    }
    emit_ins_lane3(c, 6, 0);
    emit_stur_q(c, 6, 19, 0);
}

/* CDL (ASC, ch==4): (in*slope+offset)^power then saturation around luma, RGB
 * only. pow uses a per-lane x>=0 select; the luma dot product is built with a
 * materialized weight vector (lr,lg,lb,0) and ADDV. v7 keeps the pre-pow x; v0
 * the pixel. */
static void emit_cdl_arm(codebuf *c, const toc_op *op) {
    int SL = (int)offsetof(toc_op, u.cdl.slope);
    int OF = (int)offsetof(toc_op, u.cdl.offset);
    int PW = (int)offsetof(toc_op, u.cdl.power);
    int clamp = op->u.cdl.clamp;
    float lr = op->u.cdl.luma[0], lg = op->u.cdl.luma[1], lb = op->u.cdl.luma[2];
    float sat = op->u.cdl.saturation;
    if (lr == 0.0f && lg == 0.0f && lb == 0.0f) {
        lr = 0.2126f; lg = 0.7152f; lb = 0.0722f;
    }
    emit_load_imm64(c, 9, addr_of(op));
    emit_ldur_q(c, 0, 19, 0);
    emit_ldur_q(c, 2, 9, SL); emit_fmul(c, 1, 0, 2);      /* x = pixel*slope */
    emit_ldur_q(c, 2, 9, OF); emit_fadd(c, 1, 1, 2);      /* + offset */
    if (clamp) { emit_fmax(c, 1, 1, K_ZERO); emit_fmin(c, 1, 1, K_1F); }
    emit_mov_v(c, 7, 1);                                  /* save x */
    emit_fmax(c, 1, 1, K_FLTMIN);
    emit_log2_neon(c);
    emit_ldur_q(c, 6, 9, PW); emit_fmul(c, 1, 1, 6);      /* power*log2 */
    emit_exp2_neon(c);                                    /* v1 = pow(x,power) */
    emit_fcmge0(c, 6, 7);                                 /* mask = x>=0 */
    emit_bsl(c, 6, 1, 7);                                 /* vv = x>=0 ? pow : x */
    /* luma = lr*vv0 + lg*vv1 + lb*vv2  (weight vector in v2) */
    emit_bcast_f(c, 2, lr);
    emit_set_lane_f(c, 2, 1, lg);
    emit_set_lane_f(c, 2, 2, lb);
    emit_set_lane_f(c, 2, 3, 0.0f);
    emit_fmul(c, 3, 6, 2);                                /* prod = vv*weights */
    emit_faddp_v(c, 3, 3, 3); emit_faddp_s(c, 3, 3);      /* luma = sum lanes */
    emit_dup_s(c, 4, 3, 0);                               /* broadcast luma */
    emit_bcast_f(c, 2, sat);
    emit_fsub(c, 5, 6, 4); emit_fmul(c, 5, 5, 2); emit_fadd(c, 5, 4, 5);
    if (clamp) { emit_fmax(c, 5, 5, K_ZERO); emit_fmin(c, 5, 5, K_1F); }
    emit_ins_lane3(c, 5, 0);
    emit_stur_q(c, 5, 19, 0);
}

/* MATRIX (ch==4): v = (c0*r + c1*g) + c2*b + c3*a + off, in NEON, matching the
 * NEON interpreter's association exactly. x9 := &op. */
static void emit_matrix_arm(codebuf *c, const toc_op *op) {
    int M = (int)offsetof(toc_op, u.matrix.m);
    int O = (int)offsetof(toc_op, u.matrix.off);
    emit_load_imm64(c, 9, addr_of(op));
    emit_ldur_q(c, 0, 19, 0);                /* v0 = pixel [r,g,b,a] */
    emit_dup_s(c, 1, 0, 0); emit_dup_s(c, 2, 0, 1);
    emit_dup_s(c, 3, 0, 2); emit_dup_s(c, 4, 0, 3);
    emit_ldur_q(c, 5, 9, M + 0);  emit_fmul(c, 5, 5, 1);
    emit_ldur_q(c, 6, 9, M + 16); emit_fmul(c, 6, 6, 2); emit_fadd(c, 5, 5, 6);
    emit_ldur_q(c, 6, 9, M + 32); emit_fmul(c, 6, 6, 3); emit_fadd(c, 5, 5, 6);
    emit_ldur_q(c, 6, 9, M + 48); emit_fmul(c, 6, 6, 4); emit_fadd(c, 5, 5, 6);
    emit_ldur_q(c, 6, 9, O);      emit_fadd(c, 5, 5, 6);
    emit_stur_q(c, 5, 19, 0);
}

/* RANGE (ch==4): v = clamp(in*scale + offset, min, max) per channel. */
static void emit_range_arm(codebuf *c, const toc_op *op) {
    int S = (int)offsetof(toc_op, u.range.scale);
    int OF = (int)offsetof(toc_op, u.range.offset);
    int MN = (int)offsetof(toc_op, u.range.min);
    int MX = (int)offsetof(toc_op, u.range.max);
    emit_load_imm64(c, 9, addr_of(op));
    emit_ldur_q(c, 0, 19, 0);
    emit_ldur_q(c, 1, 9, S);  emit_fmul(c, 0, 0, 1);
    emit_ldur_q(c, 1, 9, OF); emit_fadd(c, 0, 0, 1);
    if (op->u.range.clamp_lo) { emit_ldur_q(c, 1, 9, MN); emit_fmax(c, 0, 0, 1); }
    if (op->u.range.clamp_hi) { emit_ldur_q(c, 1, 9, MX); emit_fmin(c, 0, 0, 1); }
    emit_stur_q(c, 0, 19, 0);
}

/* call toc_apply_op_pixel(op, px=x19, ch) */
static void emit_helper_call_arm(codebuf *c, const toc_op *op, int ch) {
    emit_load_imm64(c, 0, addr_of(op));   /* x0 = op */
    emit_mov_reg(c, 1, 19);               /* x1 = px */
    emit_movz_w(c, 2, (unsigned)ch);      /* w2 = ch */
    emit_load_imm64(c, 9, addr_of((const void *)&toc_apply_op_pixel));
    emit_blr(c, 9);
}

static void emit_op_arm(codebuf *c, const toc_op *op, int channels) {
    if (channels == 4 && op->kind == TOC_OP_MATRIX) emit_matrix_arm(c, op);
    else if (channels == 4 && op->kind == TOC_OP_RANGE) emit_range_arm(c, op);
    else if (channels == 4 && op->kind == TOC_OP_EXPONENT) emit_exponent_arm(c, op);
    else if (channels == 4 && op->kind == TOC_OP_LOG) emit_log_arm(c, op);
    else if (channels == 4 && op->kind == TOC_OP_EXP_LINEAR) emit_explin_arm(c, op);
    else if (channels == 4 && op->kind == TOC_OP_CDL && !op->u.cdl.inverse)
        emit_cdl_arm(c, op); /* inverse CDL falls back to the kernel call */
    else if (op->kind != TOC_OP_NOOP) emit_helper_call_arm(c, op, channels);
}

toc_result toc_jit_compile(const toc_op_list *ops, int channels,
                           const toc_allocator *a, toc_jit **out) {
    codebuf c;
    size_t k;
    toc_jit *j;
#if !defined(TOC_HAVE_MMAP)
    (void)ops; (void)channels; (void)a; (void)out;
    return TOC_ERROR_UNSUPPORTED;
#else
    size_t pgsz = 4096, mapsz, loop_pos, cbz_at;
    int has_pow = 0;
    void *mem;
    if (!ops || !out || (channels != 3 && channels != 4))
        return TOC_ERROR_INVALID_ARGUMENT;
    if (!a) a = toc_default_allocator();
    *out = NULL;
    memset(&c, 0, sizeof(c));
    c.a = a;

    /* A pipeline with an inline-pow (exponent) op hoists its polynomial
     * constants into v8..v28 before the loop; v8..v15 are callee-saved (d8..d15)
     * so that path uses a larger frame and saves/restores them. */
    for (k = 0; k < ops->count; ++k) {
        toc_op_kind kk = ops->ops[k].kind;
        if (channels == 4 && (kk == TOC_OP_EXPONENT || kk == TOC_OP_LOG ||
                              kk == TOC_OP_EXP_LINEAR || kk == TOC_OP_CDL))
            has_pow = 1;
    }

    if (has_pow) {
        /* 96B frame: x29/x30, x19/x20, d8..d15 (16-aligned). */
        e32(&c, 0xA9BA7BFDu);      /* stp x29, x30, [sp, #-96]! */
        e32(&c, 0xA90153F3u);      /* stp x19, x20, [sp, #16] */
        e32(&c, 0x6D0227E8u);      /* stp d8,  d9,  [sp, #32] */
        e32(&c, 0x6D032FEAu);      /* stp d10, d11, [sp, #48] */
        e32(&c, 0x6D0437ECu);      /* stp d12, d13, [sp, #64] */
        e32(&c, 0x6D053FEEu);      /* stp d14, d15, [sp, #80] */
        emit_mov_reg(&c, 19, 0);
        emit_mov_reg(&c, 20, 1);
        emit_pow_consts(&c);       /* load v8..v28 once */
    } else {
        /* prologue: fp/lr + callee-saved x19/x20 (32B frame, 16-aligned). */
        e32(&c, 0xA9BE7BFDu);      /* stp x29, x30, [sp, #-32]! */
        e32(&c, 0xA90153F3u);      /* stp x19, x20, [sp, #16] */
        emit_mov_reg(&c, 19, 0);   /* x19 = rgba */
        emit_mov_reg(&c, 20, 1);   /* x20 = npix */
    }

    /* per-pixel loop: while (x20) { ops on [x19]; x19+=ch*4; x20--; } */
    loop_pos = c.len;
    cbz_at = c.len;
    e32(&c, 0xB4000000u | 20u);    /* cbz x20, done  (imm19 patched below) */
    for (k = 0; k < ops->count; ++k) emit_op_arm(&c, &ops->ops[k], channels);
    emit_add_imm(&c, 19, 19, (unsigned)(channels * 4));
    emit_sub_imm(&c, 20, 20, 1);
    {
        int32_t off = (int32_t)((int64_t)loop_pos - (int64_t)c.len); /* b -> loop */
        e32(&c, 0x14000000u | ((uint32_t)(off >> 2) & 0x03ffffffu));
    }
    if (!c.oom) {                  /* patch cbz -> done (here) */
        int32_t off = (int32_t)((int64_t)c.len - (int64_t)cbz_at);
        uint32_t insn =
            0xB4000000u | (((uint32_t)(off >> 2) & 0x7ffffu) << 5) | 20u;
        c.buf[cbz_at] = (uint8_t)insn;
        c.buf[cbz_at + 1] = (uint8_t)(insn >> 8);
        c.buf[cbz_at + 2] = (uint8_t)(insn >> 16);
        c.buf[cbz_at + 3] = (uint8_t)(insn >> 24);
    }

    /* epilogue (matches the prologue frame) */
    if (has_pow) {
        e32(&c, 0x6D4227E8u);      /* ldp d8,  d9,  [sp, #32] */
        e32(&c, 0x6D432FEAu);      /* ldp d10, d11, [sp, #48] */
        e32(&c, 0x6D4437ECu);      /* ldp d12, d13, [sp, #64] */
        e32(&c, 0x6D453FEEu);      /* ldp d14, d15, [sp, #80] */
        e32(&c, 0xA94153F3u);      /* ldp x19, x20, [sp, #16] */
        e32(&c, 0xA8C67BFDu);      /* ldp x29, x30, [sp], #96 */
    } else {
        e32(&c, 0xA94153F3u);      /* ldp x19, x20, [sp, #16] */
        e32(&c, 0xA8C27BFDu);      /* ldp x29, x30, [sp], #32 */
    }
    e32(&c, 0xD65F03C0u);          /* ret */

    if (c.oom) { toc_free(a, c.buf); return TOC_ERROR_OUT_OF_MEMORY; }

    mapsz = (c.len + pgsz - 1) & ~(pgsz - 1);
#if defined(__APPLE__)
    /* Apple Silicon enforces W^X: map MAP_JIT, toggle write-protect per thread,
     * then flush the icache before executing. */
    mem = mmap(NULL, mapsz, PROT_READ | PROT_WRITE | PROT_EXEC,
               MAP_PRIVATE | MAP_ANONYMOUS | MAP_JIT, -1, 0);
    if (mem == MAP_FAILED) { toc_free(a, c.buf); return TOC_ERROR_UNSUPPORTED; }
    pthread_jit_write_protect_np(0);
    memcpy(mem, c.buf, c.len);
    pthread_jit_write_protect_np(1);
    sys_icache_invalidate(mem, c.len);
    toc_free(a, c.buf);
#else
    mem = mmap(NULL, mapsz, PROT_READ | PROT_WRITE,
               MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (mem == MAP_FAILED) { toc_free(a, c.buf); return TOC_ERROR_UNSUPPORTED; }
    memcpy(mem, c.buf, c.len);
    toc_free(a, c.buf);
    if (mprotect(mem, mapsz, PROT_READ | PROT_EXEC) != 0) {
        munmap(mem, mapsz);
        return TOC_ERROR_UNSUPPORTED;
    }
    __builtin___clear_cache((char *)mem, (char *)mem + c.len);
#endif
    j = (toc_jit *)toc_malloc(a, sizeof(*j));
    if (!j) { munmap(mem, mapsz); return TOC_ERROR_OUT_OF_MEMORY; }
    j->mem = mem;
    j->mapsz = mapsz;
    j->alloc = *a;
    memcpy(&j->fn, &mem, sizeof(void *)); /* avoid object<->fn ptr warning */
    *out = j;
    return TOC_SUCCESS;
#endif
}

toc_jit_fn toc_jit_func(const toc_jit *j) { return j ? j->fn : NULL; }

void toc_jit_destroy(toc_jit *j) {
    toc_allocator a;
    if (!j) return;
    a = j->alloc;
#if defined(TOC_HAVE_MMAP)
    if (j->mem) munmap(j->mem, j->mapsz);
#endif
    toc_free(&a, j);
}

#else /* other architectures: stub */

#include <stddef.h>
struct toc_jit { int unused; };
toc_result toc_jit_compile(const toc_op_list *ops, int channels,
                           const toc_allocator *a, toc_jit **out) {
    (void)ops; (void)channels; (void)a;
    if (out) *out = NULL;
    return TOC_ERROR_UNSUPPORTED;
}
toc_jit_fn toc_jit_func(const toc_jit *j) { (void)j; return NULL; }
void toc_jit_destroy(toc_jit *j) { (void)j; }

#endif
