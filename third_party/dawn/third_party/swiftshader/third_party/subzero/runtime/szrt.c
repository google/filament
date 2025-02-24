//===- subzero/runtime/szrt.c - Subzero runtime source ----------*- C++ -*-===//
//
//                        The Subzero Code Generator
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file implements wrappers for particular bitcode instructions
// that are too uncommon and complex for a particular target to bother
// implementing directly in Subzero target lowering.  This needs to be
// compiled by some non-Subzero compiler.
//
//===----------------------------------------------------------------------===//

#include <stdint.h>

uint32_t __Sz_fptoui_f32_i32(float value) { return (uint32_t)value; }

uint32_t __Sz_fptoui_f64_i32(double value) { return (uint32_t)value; }

uint64_t __Sz_fptoui_f32_i64(float Value) { return (uint64_t)Value; }

uint64_t __Sz_fptoui_f64_i64(double Value) { return (uint64_t)Value; }

int64_t __Sz_fptosi_f32_i64(float Value) { return (int64_t)Value; }

int64_t __Sz_fptosi_f64_i64(double Value) { return (int64_t)Value; }

float __Sz_uitofp_i32_f32(uint32_t Value) { return (float)Value; }

float __Sz_uitofp_i64_f32(uint64_t Value) { return (float)Value; }

double __Sz_uitofp_i32_f64(uint32_t Value) { return (double)Value; }

double __Sz_uitofp_i64_f64(uint64_t Value) { return (double)Value; }

float __Sz_sitofp_i64_f32(int64_t Value) { return (float)Value; }

double __Sz_sitofp_i64_f64(int64_t Value) { return (double)Value; }

// Other helper calls emitted by Subzero but not implemented here:
// Compiler-rt:
//   __udivsi3     - udiv i32
//   __divsi3      - sdiv i32
//   __umodsi3     - urem i32
//   __modsi3      - srem i32
//   __udivdi3     - udiv i64
//   __divdi3      - sdiv i64
//   __umoddi3     - urem i64
//   __moddi3      - srem i64
//   __popcountsi2 - call @llvm.ctpop.i32
//   __popcountdi2 - call @llvm.ctpop.i64
// libm:
//   fmodf - frem f32
//   fmod  - frem f64
// libc:
//   setjmp  - call @llvm.nacl.setjmp
//   longjmp - call @llvm.nacl.longjmp
//   memcpy  - call @llvm.memcpy.p0i8.p0i8.i32
//   memmove - call @llvm.memmove.p0i8.p0i8.i32
//   memset  - call @llvm.memset.p0i8.i32
// unsandboxed_irt:
//   __nacl_read_tp
//   __aeabi_read_tp [arm32 only]
// MIPS runtime library:
// __sync_fetch_and_add_8
// __sync_fetch_and_and_8
// __sync_fetch_and_or_8
// __sync_fetch_and_sub_8
// __sync_fetch_and_xor_8
// __sync_lock_test_and_set_8
// __sync_val_compare_and_swap_8
