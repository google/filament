; RUN: opt < %s -instcombine -S | FileCheck %s

declare <4 x float> @llvm.x86.sse41.insertps(<4 x float>, <4 x float>, i8) nounwind readnone

; This should never happen, but make sure we don't crash handling a non-constant immediate byte.

define <4 x float> @insertps_non_const_imm(<4 x float> %v1, <4 x float> %v2, i8 %c) {
  %res = call <4 x float> @llvm.x86.sse41.insertps(<4 x float> %v1, <4 x float> %v2, i8 %c)
  ret <4 x float> %res

; CHECK-LABEL: @insertps_non_const_imm
; CHECK-NEXT:  call <4 x float> @llvm.x86.sse41.insertps(<4 x float> %v1, <4 x float> %v2, i8 %c)
; CHECK-NEXT:  ret <4 x float>
}

; If all zero mask bits are set, return a zero regardless of the other control bits.

define <4 x float> @insertps_0x0f(<4 x float> %v1, <4 x float> %v2) {
  %res = call <4 x float> @llvm.x86.sse41.insertps(<4 x float> %v1, <4 x float> %v2, i8 15)
  ret <4 x float> %res

; CHECK-LABEL: @insertps_0x0f
; CHECK-NEXT:  ret <4 x float> zeroinitializer
}
define <4 x float> @insertps_0xff(<4 x float> %v1, <4 x float> %v2) {
  %res = call <4 x float> @llvm.x86.sse41.insertps(<4 x float> %v1, <4 x float> %v2, i8 255)
  ret <4 x float> %res

; CHECK-LABEL: @insertps_0xff
; CHECK-NEXT:  ret <4 x float> zeroinitializer
}

; If some zero mask bits are set that do not override the insertion, we do not change anything.

define <4 x float> @insertps_0x0c(<4 x float> %v1, <4 x float> %v2) {
  %res = call <4 x float> @llvm.x86.sse41.insertps(<4 x float> %v1, <4 x float> %v2, i8 12)
  ret <4 x float> %res

; CHECK-LABEL: @insertps_0x0c
; CHECK-NEXT:  call <4 x float> @llvm.x86.sse41.insertps(<4 x float> %v1, <4 x float> %v2, i8 12)
; CHECK-NEXT:  ret <4 x float>
}

; ...unless both input vectors are the same operand.

define <4 x float> @insertps_0x15_single_input(<4 x float> %v1) {
  %res = call <4 x float> @llvm.x86.sse41.insertps(<4 x float> %v1, <4 x float> %v1, i8 21)
  ret <4 x float> %res

; CHECK-LABEL: @insertps_0x15_single_input
; CHECK-NEXT:  shufflevector <4 x float> %v1, <4 x float> <float 0.000000e+00, float undef, float 0.000000e+00, float undef>, <4 x i32> <i32 4, i32 0, i32 6, i32 3>
; CHECK-NEXT:  ret <4 x float>
}

; The zero mask overrides the insertion lane.

define <4 x float> @insertps_0x1a_single_input(<4 x float> %v1) {
  %res = call <4 x float> @llvm.x86.sse41.insertps(<4 x float> %v1, <4 x float> %v1, i8 26)
  ret <4 x float> %res

; CHECK-LABEL: @insertps_0x1a_single_input
; CHECK-NEXT:  shufflevector <4 x float> %v1, <4 x float> <float undef, float 0.000000e+00, float undef, float 0.000000e+00>, <4 x i32> <i32 0, i32 5, i32 2, i32 7>
; CHECK-NEXT:  ret <4 x float>
}

; The zero mask overrides the insertion lane, so the second input vector is not used.

define <4 x float> @insertps_0xc1(<4 x float> %v1, <4 x float> %v2) {
  %res = call <4 x float> @llvm.x86.sse41.insertps(<4 x float> %v1, <4 x float> %v2, i8 193)
  ret <4 x float> %res

; CHECK-LABEL: @insertps_0xc1
; CHECK-NEXT:  shufflevector <4 x float> %v1, <4 x float> <float 0.000000e+00, float undef, float undef, float undef>, <4 x i32> <i32 4, i32 1, i32 2, i32 3>
; CHECK-NEXT:  ret <4 x float>
}

; If no zero mask bits are set, convert to a shuffle.

define <4 x float> @insertps_0x00(<4 x float> %v1, <4 x float> %v2) {
  %res = call <4 x float> @llvm.x86.sse41.insertps(<4 x float> %v1, <4 x float> %v2, i8 0)
  ret <4 x float> %res

; CHECK-LABEL: @insertps_0x00
; CHECK-NEXT:  shufflevector <4 x float> %v1, <4 x float> %v2, <4 x i32> <i32 4, i32 1, i32 2, i32 3>
; CHECK-NEXT:  ret <4 x float>
}

define <4 x float> @insertps_0x10(<4 x float> %v1, <4 x float> %v2) {
  %res = call <4 x float> @llvm.x86.sse41.insertps(<4 x float> %v1, <4 x float> %v2, i8 16)
  ret <4 x float> %res

; CHECK-LABEL: @insertps_0x10
; CHECK-NEXT:  shufflevector <4 x float> %v1, <4 x float> %v2, <4 x i32> <i32 0, i32 4, i32 2, i32 3>
; CHECK-NEXT:  ret <4 x float>
}

define <4 x float> @insertps_0x20(<4 x float> %v1, <4 x float> %v2) {
  %res = call <4 x float> @llvm.x86.sse41.insertps(<4 x float> %v1, <4 x float> %v2, i8 32)
  ret <4 x float> %res

; CHECK-LABEL: @insertps_0x20
; CHECK-NEXT:  shufflevector <4 x float> %v1, <4 x float> %v2, <4 x i32> <i32 0, i32 1, i32 4, i32 3>
; CHECK-NEXT:  ret <4 x float>
}

define <4 x float> @insertps_0x30(<4 x float> %v1, <4 x float> %v2) {
  %res = call <4 x float> @llvm.x86.sse41.insertps(<4 x float> %v1, <4 x float> %v2, i8 48)
  ret <4 x float> %res

; CHECK-LABEL: @insertps_0x30
; CHECK-NEXT:  shufflevector <4 x float> %v1, <4 x float> %v2, <4 x i32> <i32 0, i32 1, i32 2, i32 4>
; CHECK-NEXT:  ret <4 x float>
}

define <4 x float> @insertps_0xc0(<4 x float> %v1, <4 x float> %v2) {
  %res = call <4 x float> @llvm.x86.sse41.insertps(<4 x float> %v1, <4 x float> %v2, i8 192)
  ret <4 x float> %res

; CHECK-LABEL: @insertps_0xc0
; CHECK-NEXT:  shufflevector <4 x float> %v1, <4 x float> %v2, <4 x i32> <i32 7, i32 1, i32 2, i32 3>
; CHECK-NEXT:  ret <4 x float>
}

define <4 x float> @insertps_0xd0(<4 x float> %v1, <4 x float> %v2) {
  %res = call <4 x float> @llvm.x86.sse41.insertps(<4 x float> %v1, <4 x float> %v2, i8 208)
  ret <4 x float> %res

; CHECK-LABEL: @insertps_0xd0
; CHECK-NEXT:  shufflevector <4 x float> %v1, <4 x float> %v2, <4 x i32> <i32 0, i32 7, i32 2, i32 3>
; CHECK-NEXT:  ret <4 x float>
}

define <4 x float> @insertps_0xe0(<4 x float> %v1, <4 x float> %v2) {
  %res = call <4 x float> @llvm.x86.sse41.insertps(<4 x float> %v1, <4 x float> %v2, i8 224)
  ret <4 x float> %res

; CHECK-LABEL: @insertps_0xe0
; CHECK-NEXT:  shufflevector <4 x float> %v1, <4 x float> %v2, <4 x i32> <i32 0, i32 1, i32 7, i32 3>
; CHECK-NEXT:  ret <4 x float>
}

define <4 x float> @insertps_0xf0(<4 x float> %v1, <4 x float> %v2) {
  %res = call <4 x float> @llvm.x86.sse41.insertps(<4 x float> %v1, <4 x float> %v2, i8 240)
  ret <4 x float> %res

; CHECK-LABEL: @insertps_0xf0
; CHECK-NEXT:  shufflevector <4 x float> %v1, <4 x float> %v2, <4 x i32> <i32 0, i32 1, i32 2, i32 7>
; CHECK-NEXT:  ret <4 x float>
}

