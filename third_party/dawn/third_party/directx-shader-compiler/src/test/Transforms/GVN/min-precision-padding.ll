; RUN: opt < %s -basicaa -gvn -S | FileCheck %s

; Regression test for min precision vector GVN miscompilation.
; DXC's data layout pads i16 to 32 bits (i16:32). GVN must not:
;   1. Coerce padded vector types via bitcast (CanCoerceMustAliasedValueToLoad)
;   2. Forward a zeroinitializer store past partial element stores (processLoad)
;
; Without the fix, GVN would forward the zeroinitializer vector load, producing
; incorrect all-zero results for elements that were individually written.

target datalayout = "e-m:e-p:32:32-i1:32-i8:32-i16:32-i32:32-i64:64-f16:32-f32:32-f64:64-n8:16:32:64"
target triple = "dxil-ms-dx"

; Test 1: GVN must not forward zeroinitializer past element store for <3 x i16>.
; The store of zeroinitializer to %dst is followed by an element store to
; %dst[0], then a vector load of %dst. GVN must not replace the vector load
; with the zeroinitializer.

; CHECK-LABEL: @test_no_forward_i16_vec3
; CHECK: store <3 x i16> zeroinitializer
; CHECK: store i16 %val
; The vector load must survive — GVN must not replace it with zeroinitializer.
; CHECK: %result = load <3 x i16>
; CHECK: ret <3 x i16> %result
define <3 x i16> @test_no_forward_i16_vec3(i16 %val) {
entry:
  %dst = alloca <3 x i16>, align 4
  store <3 x i16> zeroinitializer, <3 x i16>* %dst, align 4
  %elem0 = getelementptr inbounds <3 x i16>, <3 x i16>* %dst, i32 0, i32 0
  store i16 %val, i16* %elem0, align 4
  %result = load <3 x i16>, <3 x i16>* %dst, align 4
  ret <3 x i16> %result
}

; Test 2: Same pattern with <3 x half> (f16:32 padding).

; CHECK-LABEL: @test_no_forward_f16_vec3
; CHECK: store <3 x half> zeroinitializer
; CHECK: store half %val
; CHECK: %result = load <3 x half>
; CHECK: ret <3 x half> %result
define <3 x half> @test_no_forward_f16_vec3(half %val) {
entry:
  %dst = alloca <3 x half>, align 4
  store <3 x half> zeroinitializer, <3 x half>* %dst, align 4
  %elem0 = getelementptr inbounds <3 x half>, <3 x half>* %dst, i32 0, i32 0
  store half %val, half* %elem0, align 4
  %result = load <3 x half>, <3 x half>* %dst, align 4
  ret <3 x half> %result
}

; Test 3: Multiple element stores — all must survive.
; Stores to elements 0, 1, 2 of a <3 x i16> vector after zeroinitializer.

; CHECK-LABEL: @test_no_forward_i16_vec3_all_elems
; CHECK: store <3 x i16> zeroinitializer
; CHECK: store i16 %v0
; CHECK: store i16 %v1
; CHECK: store i16 %v2
; CHECK: %result = load <3 x i16>
; CHECK: ret <3 x i16> %result
define <3 x i16> @test_no_forward_i16_vec3_all_elems(i16 %v0, i16 %v1, i16 %v2) {
entry:
  %dst = alloca <3 x i16>, align 4
  store <3 x i16> zeroinitializer, <3 x i16>* %dst, align 4
  %e0 = getelementptr inbounds <3 x i16>, <3 x i16>* %dst, i32 0, i32 0
  store i16 %v0, i16* %e0, align 4
  %e1 = getelementptr inbounds <3 x i16>, <3 x i16>* %dst, i32 0, i32 1
  store i16 %v1, i16* %e1, align 4
  %e2 = getelementptr inbounds <3 x i16>, <3 x i16>* %dst, i32 0, i32 2
  store i16 %v2, i16* %e2, align 4
  %result = load <3 x i16>, <3 x i16>* %dst, align 4
  ret <3 x i16> %result
}

; Test 4: Coercion rejection — store a <3 x i16> vector, load as different type.
; GVN must not attempt bitcast coercion on padded types.
; If coercion happened, the load would be eliminated and replaced with a bitcast.

; CHECK-LABEL: @test_no_coerce_i16_vec3
; CHECK: store <3 x i16>
; CHECK: load i96
; CHECK-NOT: bitcast
; CHECK: ret
define i96 @test_no_coerce_i16_vec3(<3 x i16> %v) {
entry:
  %ptr = alloca <3 x i16>, align 4
  store <3 x i16> %v, <3 x i16>* %ptr, align 4
  %iptr = bitcast <3 x i16>* %ptr to i96*
  %result = load i96, i96* %iptr, align 4
  ret i96 %result
}

; Test 5: Long vector variant — <5 x i16> (exceeds 4-element native size).

; CHECK-LABEL: @test_no_forward_i16_vec5
; CHECK: store <5 x i16> zeroinitializer
; CHECK: store i16 %val
; CHECK: %result = load <5 x i16>
; CHECK: ret <5 x i16> %result
define <5 x i16> @test_no_forward_i16_vec5(i16 %val) {
entry:
  %dst = alloca <5 x i16>, align 4
  store <5 x i16> zeroinitializer, <5 x i16>* %dst, align 4
  %elem0 = getelementptr inbounds <5 x i16>, <5 x i16>* %dst, i32 0, i32 0
  store i16 %val, i16* %elem0, align 4
  %result = load <5 x i16>, <5 x i16>* %dst, align 4
  ret <5 x i16> %result
}

; Test 6: Long vector variant — <8 x half>.

; CHECK-LABEL: @test_no_forward_f16_vec8
; CHECK: store <8 x half> zeroinitializer
; CHECK: store half %val
; CHECK: %result = load <8 x half>
; CHECK: ret <8 x half> %result
define <8 x half> @test_no_forward_f16_vec8(half %val) {
entry:
  %dst = alloca <8 x half>, align 4
  store <8 x half> zeroinitializer, <8 x half>* %dst, align 4
  %elem0 = getelementptr inbounds <8 x half>, <8 x half>* %dst, i32 0, i32 0
  store half %val, half* %elem0, align 4
  %result = load <8 x half>, <8 x half>* %dst, align 4
  ret <8 x half> %result
}

; Test 7: Same-type store-to-load forwarding must still work for padded types.
; GVN should forward %v directly — no intervening writes, same type.

; CHECK-LABEL: @test_same_type_forward_i16_vec3
; The load should be eliminated and %v returned directly.
; CHECK-NOT: load
; CHECK: ret <3 x i16> %v
define <3 x i16> @test_same_type_forward_i16_vec3(<3 x i16> %v) {
entry:
  %ptr = alloca <3 x i16>, align 4
  store <3 x i16> %v, <3 x i16>* %ptr, align 4
  %result = load <3 x i16>, <3 x i16>* %ptr, align 4
  ret <3 x i16> %result
}

; Test 8: Same-type forwarding for <3 x half>.

; CHECK-LABEL: @test_same_type_forward_f16_vec3
; CHECK-NOT: load
; CHECK: ret <3 x half> %v
define <3 x half> @test_same_type_forward_f16_vec3(<3 x half> %v) {
entry:
  %ptr = alloca <3 x half>, align 4
  store <3 x half> %v, <3 x half>* %ptr, align 4
  %result = load <3 x half>, <3 x half>* %ptr, align 4
  ret <3 x half> %result
}
