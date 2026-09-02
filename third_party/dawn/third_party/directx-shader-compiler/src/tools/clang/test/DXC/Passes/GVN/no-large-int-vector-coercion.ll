; RUN: %dxopt %s -hlsl-passes-resume -gvn -S | FileCheck %s

; Regression test: when a load is fed by a wider vector store at the same
; address, GVN previously coerced the value by bit-casting the vector to a
; same-sized integer (e.g. <4 x i32> -> i128) and truncating. DXIL does not
; support integer widths greater than 64 bits, so the resulting module
; failed validation with "Int type 'i128' has an invalid width."
;
; This test ensures GVN does not introduce a bitcast to an oversized
; integer when forwarding the wider vector value to a narrower scalar load.

target datalayout = "e-m:e-p:32:32-i1:32-i8:32-i16:32-i32:32-i64:64-f16:32-f32:32-f64:64-n8:16:32:64"
target triple = "dxil-ms-dx"

; CHECK-LABEL: @test_v4i32_to_i32
; CHECK-NOT: i128
; CHECK-NOT: i256
; CHECK: ret i32
define i32 @test_v4i32_to_i32(<4 x i32>* %p) {
entry:
  store <4 x i32> <i32 1, i32 2, i32 3, i32 4>, <4 x i32>* %p
  %sp = bitcast <4 x i32>* %p to i32*
  %v = load i32, i32* %sp
  ret i32 %v
}

; CHECK-LABEL: @test_v8i32_to_i32
; CHECK-NOT: i256
; CHECK: ret i32
define i32 @test_v8i32_to_i32(<8 x i32>* %p, <8 x i32> %vec) {
entry:
  store <8 x i32> %vec, <8 x i32>* %p
  %sp = bitcast <8 x i32>* %p to i32*
  %v = load i32, i32* %sp
  ret i32 %v
}

; CHECK-LABEL: @test_v2i32_to_i32
; The <2 x i32> case is 64 bits wide, which is a legal DXIL integer, so
; coercion here is fine. We just make sure no i128 is introduced.
; CHECK-NOT: i128
; CHECK: ret i32
define i32 @test_v2i32_to_i32(<2 x i32>* %p, <2 x i32> %vec) {
entry:
  store <2 x i32> %vec, <2 x i32>* %p
  %sp = bitcast <2 x i32>* %p to i32*
  %v = load i32, i32* %sp
  ret i32 %v
}
