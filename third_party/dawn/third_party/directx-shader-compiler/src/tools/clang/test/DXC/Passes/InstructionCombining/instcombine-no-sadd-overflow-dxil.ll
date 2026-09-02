; RUN: %dxopt %s -instcombine -S | FileCheck %s

; Regression test for https://github.com/microsoft/DirectXShaderCompiler/issues/8598
;
; InstCombine used to fold this signed add overflow-check idiom into
; llvm.sadd.with.overflow, which is not a legal DXIL intrinsic. For DXIL the
; add/compare must be left alone.

target datalayout = "e-m:e-p:32:32-i1:32-i8:32-i16:32-i32:32-i64:64-f16:32-f32:32-f64:64-n8:16:32:64"
target triple = "dxil-ms-dx"

; CHECK-LABEL: define i1 @test_sadd
; CHECK-NOT: sadd.with.overflow
; CHECK-NOT: extractvalue
; CHECK: add {{(nsw )?}}i32
; CHECK: icmp ugt i32
define i1 @test_sadd(i8 %a8, i8 %b8) {
  %a = sext i8 %a8 to i32
  %b = sext i8 %b8 to i32
  %add = add i32 %a, %b
  %addcst = add i32 %add, 128
  %cmp = icmp ugt i32 %addcst, 255
  ret i1 %cmp
}
