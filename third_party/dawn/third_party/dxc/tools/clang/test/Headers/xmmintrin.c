// RUN: %clang_cc1 %s -ffreestanding -triple x86_64-apple-macosx10.9.0 -emit-llvm -o - | FileCheck %s
//
// RUN: rm -rf %t
// RUN: %clang_cc1 %s -ffreestanding -triple x86_64-apple-macosx10.9.0 -emit-llvm -o - \
// RUN:     -fmodules -fimplicit-module-maps -fmodules-cache-path=%t -isystem %S/Inputs/include \
// RUN:     | FileCheck %s
// REQUIRES: x86-registered-target
#include <xmmintrin.h>

// Make sure the last step of _mm_cvtps_pi16 converts <4 x i32> to <4 x i16> by
// checking that clang emits PACKSSDW instead of PACKSSWB.

// CHECK: define i64 @test_mm_cvtps_pi16
// CHECK: call x86_mmx @llvm.x86.mmx.packssdw

__m64 test_mm_cvtps_pi16(__m128 a) {
  return _mm_cvtps_pi16(a);
}

// Make sure that including <xmmintrin.h> also makes <emmintrin.h>'s content available.
// This is an ugly hack for GCC compatibility.
__m128 test_xmmintrin_provides_emmintrin(__m128d __a, __m128d __b) {
  return _mm_add_sd(__a, __b);
}

