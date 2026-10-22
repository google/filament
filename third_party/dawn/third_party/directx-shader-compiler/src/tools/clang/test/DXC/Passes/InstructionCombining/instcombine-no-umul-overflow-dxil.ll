; RUN: %dxopt %s -instcombine -S | FileCheck %s

; Regression test for https://github.com/microsoft/DirectXShaderCompiler/issues/8598
;
; InstCombine used to fold the "widened multiply compared against UINT_MAX"
; idiom into llvm.umul.with.overflow, which is not a legal DXIL intrinsic and
; failed validation. For DXIL the multiply/compare must be left alone.

target datalayout = "e-m:e-p:32:32-i1:32-i8:32-i16:32-i32:32-i64:64-f16:32-f32:32-f64:64-n8:16:32:64"
target triple = "dxil-ms-dx"

; CHECK-LABEL: define i1 @test
; CHECK-NOT: umul.with.overflow
; CHECK-NOT: extractvalue
; CHECK: mul {{(nuw )?}}i64 %xw, %yw
; CHECK: icmp ugt i64
define i1 @test(i32 %x, i32 %y) {
  %xw = zext i32 %x to i64
  %yw = zext i32 %y to i64
  %mul = mul i64 %xw, %yw
  %cmp = icmp ugt i64 %mul, 4294967295
  ret i1 %cmp
}
