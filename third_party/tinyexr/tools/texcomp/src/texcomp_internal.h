/*
 * TinyEXR texcomp - shared internal declarations across the per-codec
 * translation units (texcomp.c + texcomp_<codec>.c).
 *
 * Copyright (c) 2014-2026 Syoyo Fujita and TinyEXR authors
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef TINYEXR_TEXCOMP_INTERNAL_H_
#define TINYEXR_TEXCOMP_INTERNAL_H_

#include <stddef.h>
#include <stdint.h>

/* --- architecture / SIMD feature detection (mirrors texcomp.c) ---------- */
#if defined(__i386__) || defined(__x86_64__) || defined(_M_IX86) || defined(_M_X64)
#define TC_X86 1
#if defined(_MSC_VER)
#include <intrin.h>
#else
#include <cpuid.h>
#include <immintrin.h>
#endif
#endif

#if defined(__ARM_NEON) || defined(__ARM_NEON__)
#include <arm_neon.h>
#endif

#if defined(__GNUC__) || defined(__clang__)
#define TC_TARGET(x) __attribute__((target(x)))
#else
#define TC_TARGET(x)
#endif

#define TC_CPU_SSE2 1u
#define TC_CPU_SSE41 2u
#define TC_CPU_AVX2 4u
#define TC_CPU_NEON 8u
#define TC_CPU_SVE 16u

/* --- helpers shared across translation units ---------------------------- */
/* Defined in texcomp.c (common core). */
uint32_t tc_cpu_caps(void);
void tc_set_bits(uint8_t *dst, uint32_t *bitpos, uint32_t val, uint32_t nbits);
uint32_t tc_luma_u8(const uint8_t *p);
int32_t tc_clamp_i32(int32_t v, int32_t lo, int32_t hi);
uint8_t tc_clamp_u8_i32(int32_t v);
void tc_wr_u64(uint8_t *p, uint64_t v);
void tc_wr_u24(uint8_t *p, uint32_t v);
uint32_t tc_bswap32(uint32_t v);
uint64_t tc_bswap64(uint64_t v);
int tc_astc_valid_block(uint32_t bx, uint32_t by);
int tc_mul_ovf_size(size_t a, size_t b, size_t *out);

/* Shared 4-bit interpolation weights (defined in texcomp_bc7.c). */
extern const uint32_t tc_bc7_weights4[16];

/* Defined in texcomp_bc5.c / texcomp_bc1.c, reused by texcomp_bc3.c. */
void tc_encode_bc4_block(const uint8_t v[16], uint8_t out[8]);
void tc_encode_bc1_color_block(const uint8_t px[16][4], int dxt1, uint8_t out[8]);

/* EAC alpha block (defined in texcomp_eac.c), reused by texcomp_etc2.c. */
uint64_t tc_encode_eac_alpha(const uint8_t alpha[16]);

/* IEEE float -> binary16 bits (defined in texcomp_bc6h.c), reused by the
 * ASTC HDR encoder for FP16 constant-colour (void-extent) blocks. */
uint16_t tc_float_to_half_bits(float fv);

/* ASTC HDR endpoint codec (defined in texcomp_astc_hdr.c). LNS = the 16-bit
 * logarithmic domain ASTC HDR interpolates endpoints in. */
int tc_astc_float_to_lns16(float a);
uint16_t tc_astc_lns16_to_sf16(int p);
/* quant_color equivalent (defined in texcomp_astc.c): value as reconstructed
 * at a colour quant level; identity at level 20 (256). */
int tc_astc_hdr_color_roundtrip(uint32_t level, int value);
/* CEM 11 endpoint pack at colour quant `level` (20 == 256). */
void tc_astc_cem11_pack(const int lns0[3], const int lns1[3], int level,
                        uint8_t v[6]);
int tc_astc_cem11_unpack(const uint8_t v[6], int out0[3], int out1[3]);
/* HDR alpha endpoint codec (extra 2 values of CEM 14/15) + CEM 15 (HDR RGB +
 * HDR alpha, 8 values) wrappers. LNS domain like CEM 11. */
void tc_astc_hdr_alpha_pack(int alns0, int alns1, uint8_t out[2]);
void tc_astc_hdr_alpha_unpack(const uint8_t in[2], int *out0, int *out1);
void tc_astc_cem15_pack(const int lns0[3], const int lns1[3], int alns0,
                        int alns1, int level, uint8_t v[8]);
int tc_astc_cem15_unpack(const uint8_t v[8], int out0[4], int out1[4]);
/* CEM 7 (HDR RGB base+scale, 4 values) pack/unpack; LNS domain like CEM 11. */
void tc_astc_cem7_pack(const int e0[3], const int e1[3], int level,
                       uint8_t v[4]);
int tc_astc_cem7_unpack(const uint8_t v[4], int out0[3], int out1[3]);
/* Single-subset CEM 7 (base+scale) 4x4 block encoder. */
uint64_t tc_encode_astc_hdr_cem7_block(const int lns[16][3], uint8_t out[16]);
/* Per-texel CEM 11 4x4 block encoder (defined in texcomp_astc.c, which owns
 * the ASTC block/ISE machinery). `lns` is 16 texels of 16-bit LNS RGB; returns
 * the reconstruction SSE in the LNS domain (for mode selection). */
uint64_t tc_encode_astc_hdr_cem11_block(const int lns[16][3], uint8_t out[16]);
/* Single-subset CEM 15 (HDR RGB + HDR alpha) 4x4 block encoder; lns is RGBA. */
uint64_t tc_encode_astc_hdr_cem15_block(const int lns[16][4], uint8_t out[16]);
/* Two-subset CEM 11 block; UINT64_MAX if no usable partition. */
uint64_t tc_encode_astc_hdr_cem11_2subset_block(const int lns[16][3],
                                                uint8_t out[16]);

/* ---- ASTC block decoder (defined in texcomp_astc_decode.c) --------------- */
/* Decode one ASTC 2D block with footprint bx x by to RGBA8 (row-major, 4 bytes
 * per texel). bx,by are the block footprint (both <= 12). Returns 1 on success,
 * 0 on an invalid or unsupported encoding (HDR, 3D, reserved modes). */
int tc_astc_decode_block_rgba8(const uint8_t block[16], uint32_t bx,
                               uint32_t by, uint8_t out_rgba[16*4]);
/* Decode a full ASTC image; out_rgba must hold width*height*4 bytes. Returns 1
 * on success, 0 on any decode error in any block. */
int tc_astc_decode_image_rgba8(const uint8_t *blocks, uint32_t width,
                               uint32_t height, uint32_t bx, uint32_t by,
                               uint8_t *out_rgba);

#endif /* TINYEXR_TEXCOMP_INTERNAL_H_ */
