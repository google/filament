; RUN: opt < %s  -cost-model -analyze -mtriple=arm64-apple-ios -mcpu=cyclone | FileCheck %s
target datalayout = "e-p:32:32:32-i1:8:32-i8:8:32-i16:16:32-i32:32:32-i64:32:64-f32:32:32-f64:32:64-v64:32:64-v128:32:128-a0:0:32-n32-S32"
; CHECK-LABEL: store
define void @store() {
    ; Stores of <2 x i64> should be expensive because we don't split them and
    ; and unaligned 16b stores have bad performance.
    ; CHECK: cost of 12 {{.*}} store
    store <2 x i64> undef, <2 x i64> * undef

    ; We scalarize the loads/stores because there is no vector register name for
    ; these types (they get extended to v.4h/v.2s).
    ; CHECK: cost of 16 {{.*}} store
    store <2 x i8> undef, <2 x i8> * undef
    ; CHECK: cost of 64 {{.*}} store
    store <4 x i8> undef, <4 x i8> * undef
    ; CHECK: cost of 16 {{.*}} load
    load <2 x i8> , <2 x i8> * undef
    ; CHECK: cost of 64 {{.*}} load
    load <4 x i8> , <4 x i8> * undef

    ret void
}
