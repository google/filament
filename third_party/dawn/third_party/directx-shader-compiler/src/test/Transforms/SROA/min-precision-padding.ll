; RUN: opt < %s -sroa -S | FileCheck %s

; Regression test for SROA miscompilation of min precision vector element access.
; DXC's data layout pads i16/f16 to 32 bits (i16:32, f16:32), so GEP offsets
; between vector elements are 4 bytes apart. SROA must use alloc size (not
; primitive size) for element stride, otherwise element stores get misplaced.

target datalayout = "e-m:e-p:32:32-i1:32-i8:32-i16:32-i32:32-i64:64-f16:32-f32:32-f64:64-n8:16:32:64"
target triple = "dxil-ms-dx"

; Test 1: Element-wise write to <3 x i16> vector.
; SROA must map GEP byte offsets to correct element indices using alloc size
; (4 bytes per i16), not primitive size (2 bytes). All stores must survive
; with correct indices, and the final vector load must be preserved.

; CHECK-LABEL: @test_sroa_i16_vec3
; CHECK: getelementptr inbounds <3 x i16>, <3 x i16>* %{{.*}}, i32 0, i32 0
; CHECK: store i16 %v0
; CHECK: getelementptr inbounds <3 x i16>, <3 x i16>* %{{.*}}, i32 0, i32 1
; CHECK: store i16 %v1
; CHECK: getelementptr inbounds <3 x i16>, <3 x i16>* %{{.*}}, i32 0, i32 2
; CHECK: store i16 %v2
; CHECK: load <3 x i16>
; CHECK: ret <3 x i16>
define <3 x i16> @test_sroa_i16_vec3(i16 %v0, i16 %v1, i16 %v2) {
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

; Test 2: Same pattern with <3 x half> (f16:32 padding).

; CHECK-LABEL: @test_sroa_f16_vec3
; CHECK: getelementptr inbounds <3 x half>, <3 x half>* %{{.*}}, i32 0, i32 0
; CHECK: store half %v0
; CHECK: getelementptr inbounds <3 x half>, <3 x half>* %{{.*}}, i32 0, i32 1
; CHECK: store half %v1
; CHECK: getelementptr inbounds <3 x half>, <3 x half>* %{{.*}}, i32 0, i32 2
; CHECK: store half %v2
; CHECK: load <3 x half>
; CHECK: ret <3 x half>
define <3 x half> @test_sroa_f16_vec3(half %v0, half %v1, half %v2) {
entry:
  %dst = alloca <3 x half>, align 4
  store <3 x half> zeroinitializer, <3 x half>* %dst, align 4
  %e0 = getelementptr inbounds <3 x half>, <3 x half>* %dst, i32 0, i32 0
  store half %v0, half* %e0, align 4
  %e1 = getelementptr inbounds <3 x half>, <3 x half>* %dst, i32 0, i32 1
  store half %v1, half* %e1, align 4
  %e2 = getelementptr inbounds <3 x half>, <3 x half>* %dst, i32 0, i32 2
  store half %v2, half* %e2, align 4
  %result = load <3 x half>, <3 x half>* %dst, align 4
  ret <3 x half> %result
}

; Test 3: Partial write — only element 1 is stored. SROA must index it correctly.

; CHECK-LABEL: @test_sroa_i16_vec3_elem1
; Element 1 store must be correctly placed at GEP index 1, not index 2.
; Without the fix, byte offset 4 / prim_size 2 = index 2 (wrong).
; With the fix, byte offset 4 / alloc_size 4 = index 1 (correct).
; CHECK: getelementptr inbounds <3 x i16>, <3 x i16>* %{{.*}}, i32 0, i32 1
; CHECK: store i16 %val
; CHECK: load <3 x i16>
; CHECK: ret <3 x i16>
define <3 x i16> @test_sroa_i16_vec3_elem1(i16 %val) {
entry:
  %dst = alloca <3 x i16>, align 4
  store <3 x i16> zeroinitializer, <3 x i16>* %dst, align 4
  %e1 = getelementptr inbounds <3 x i16>, <3 x i16>* %dst, i32 0, i32 1
  store i16 %val, i16* %e1, align 4
  %result = load <3 x i16>, <3 x i16>* %dst, align 4
  ret <3 x i16> %result
}

; Test 4: Element 2 store — verifies highest index is correct.

; CHECK-LABEL: @test_sroa_i16_vec3_elem2
; CHECK: getelementptr inbounds <3 x i16>, <3 x i16>* %{{.*}}, i32 0, i32 2
; CHECK: store i16 %val
; CHECK: load <3 x i16>
; CHECK: ret <3 x i16>
define <3 x i16> @test_sroa_i16_vec3_elem2(i16 %val) {
entry:
  %dst = alloca <3 x i16>, align 4
  store <3 x i16> zeroinitializer, <3 x i16>* %dst, align 4
  %e2 = getelementptr inbounds <3 x i16>, <3 x i16>* %dst, i32 0, i32 2
  store i16 %val, i16* %e2, align 4
  %result = load <3 x i16>, <3 x i16>* %dst, align 4
  ret <3 x i16> %result
}

; Test 5: Long vector — <5 x i16> (exceeds 4-element native size).

; CHECK-LABEL: @test_sroa_i16_vec5
; CHECK: getelementptr inbounds <5 x i16>, <5 x i16>* %{{.*}}, i32 0, i32 0
; CHECK: store i16 %v0
; CHECK: getelementptr inbounds <5 x i16>, <5 x i16>* %{{.*}}, i32 0, i32 1
; CHECK: store i16 %v1
; CHECK: getelementptr inbounds <5 x i16>, <5 x i16>* %{{.*}}, i32 0, i32 4
; CHECK: store i16 %v4
; CHECK: load <5 x i16>
; CHECK: ret <5 x i16>
define <5 x i16> @test_sroa_i16_vec5(i16 %v0, i16 %v1, i16 %v2, i16 %v3, i16 %v4) {
entry:
  %dst = alloca <5 x i16>, align 4
  store <5 x i16> zeroinitializer, <5 x i16>* %dst, align 4
  %e0 = getelementptr inbounds <5 x i16>, <5 x i16>* %dst, i32 0, i32 0
  store i16 %v0, i16* %e0, align 4
  %e1 = getelementptr inbounds <5 x i16>, <5 x i16>* %dst, i32 0, i32 1
  store i16 %v1, i16* %e1, align 4
  %e2 = getelementptr inbounds <5 x i16>, <5 x i16>* %dst, i32 0, i32 2
  store i16 %v2, i16* %e2, align 4
  %e3 = getelementptr inbounds <5 x i16>, <5 x i16>* %dst, i32 0, i32 3
  store i16 %v3, i16* %e3, align 4
  %e4 = getelementptr inbounds <5 x i16>, <5 x i16>* %dst, i32 0, i32 4
  store i16 %v4, i16* %e4, align 4
  %result = load <5 x i16>, <5 x i16>* %dst, align 4
  ret <5 x i16> %result
}

; Test 6: Long vector — <8 x half>.

; CHECK-LABEL: @test_sroa_f16_vec8_partial
; CHECK: getelementptr inbounds <8 x half>, <8 x half>* %{{.*}}, i32 0, i32 0
; CHECK: store half %v0
; CHECK: getelementptr inbounds <8 x half>, <8 x half>* %{{.*}}, i32 0, i32 7
; CHECK: store half %v7
; CHECK: load <8 x half>
; CHECK: ret <8 x half>
define <8 x half> @test_sroa_f16_vec8_partial(half %v0, half %v7) {
entry:
  %dst = alloca <8 x half>, align 4
  store <8 x half> zeroinitializer, <8 x half>* %dst, align 4
  %e0 = getelementptr inbounds <8 x half>, <8 x half>* %dst, i32 0, i32 0
  store half %v0, half* %e0, align 4
  %e7 = getelementptr inbounds <8 x half>, <8 x half>* %dst, i32 0, i32 7
  store half %v7, half* %e7, align 4
  %result = load <8 x half>, <8 x half>* %dst, align 4
  ret <8 x half> %result
}
