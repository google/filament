; Test that the stpcpy library call simplifier works correctly.
; RUN: opt < %s -instcombine -S | FileCheck %s
;
; This transformation requires the pointer size, as it assumes that size_t is
; the size of a pointer.
target datalayout = "e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:32:64-f32:32:32-f64:32:64-v64:64:64-v128:128:128-a0:0:64-f80:128:128-n8:16:32"

@hello = constant [6 x i8] c"hello\00"
@a = common global [32 x i8] zeroinitializer, align 1
@b = common global [32 x i8] zeroinitializer, align 1

declare i8* @stpcpy(i8*, i8*)

define i8* @test_simplify1() {
; CHECK-LABEL: @test_simplify1(

  %dst = getelementptr [32 x i8], [32 x i8]* @a, i32 0, i32 0
  %src = getelementptr [6 x i8], [6 x i8]* @hello, i32 0, i32 0

  %ret = call i8* @stpcpy(i8* %dst, i8* %src)
; CHECK: @llvm.memcpy.p0i8.p0i8.i32
; CHECK-NEXT: getelementptr inbounds ([32 x i8], [32 x i8]* @a, i32 0, i32 5)
  ret i8* %ret
}

define i8* @test_simplify2() {
; CHECK-LABEL: @test_simplify2(

  %dst = getelementptr [32 x i8], [32 x i8]* @a, i32 0, i32 0

  %ret = call i8* @stpcpy(i8* %dst, i8* %dst)
; CHECK: [[LEN:%[a-z]+]] = call i32 @strlen
; CHECK-NEXT: getelementptr inbounds [32 x i8], [32 x i8]* @a, i32 0, i32 [[LEN]]
  ret i8* %ret
}

define i8* @test_no_simplify1() {
; CHECK-LABEL: @test_no_simplify1(

  %dst = getelementptr [32 x i8], [32 x i8]* @a, i32 0, i32 0
  %src = getelementptr [32 x i8], [32 x i8]* @b, i32 0, i32 0

  %ret = call i8* @stpcpy(i8* %dst, i8* %src)
; CHECK: call i8* @stpcpy
  ret i8* %ret
}
