; RUN: %dxopt %s -hlsl-passes-resume -hlsl-dxil-expand-trig-intrinsics -simplify-inst -S | FileCheck %s

; Regression test: SimplifyDxilCall must constant-fold the
; VectorReduceAnd / VectorReduceOr DXIL operations when their vector
; argument is a constant. In SM 6.9 'any' / 'all' lower to a
; VectorReduce call that the scalarize pass skips. Without constant
; folding, an 'any(constVector)' check that should be a trivially
; false / true value remains a call, which prevents downstream loop
; deletion and ultimately fails the 'Loop must have break' validation
; rule on a body that became dead.
;
; The preceding -hlsl-dxil-expand-trig-intrinsics pass is used purely
; to force the DxilModule to be created from the metadata in this IR
; file (it has no trig calls to expand). The actual transformation
; under test is performed by -simplify-inst via SimplifyDxilCall.

target datalayout = "e-m:e-p:32:32-i1:32-i8:32-i16:32-i32:32-i64:64-f16:32-f32:32-f64:64-n8:16:32:64"
target triple = "dxil-ms-dx"

; CHECK-LABEL: define void @main_reduce_or_zero()
; The OR-reduce of an all-zero vector is i1 false (== i32 0 after zext).
; CHECK-NOT: dx.op.vectorReduce
; CHECK: call void @dx.op.storeOutput.i32(i32 5, i32 0, i32 0, i8 0, i32 0)
define void @main_reduce_or_zero() {
  %r = call i1 @dx.op.vectorReduce.v4i1(i32 310, <4 x i1> zeroinitializer)
  %r2 = zext i1 %r to i32
  call void @dx.op.storeOutput.i32(i32 5, i32 0, i32 0, i8 0, i32 %r2)
  ret void
}

; CHECK-LABEL: define void @main_reduce_or_ones()
; The OR-reduce of an all-one vector is i1 true (== i32 1 after zext).
; CHECK-NOT: dx.op.vectorReduce
; CHECK: call void @dx.op.storeOutput.i32(i32 5, i32 0, i32 0, i8 0, i32 1)
define void @main_reduce_or_ones() {
  %r = call i1 @dx.op.vectorReduce.v4i1(i32 310, <4 x i1> <i1 true, i1 true, i1 true, i1 true>)
  %r2 = zext i1 %r to i32
  call void @dx.op.storeOutput.i32(i32 5, i32 0, i32 0, i8 0, i32 %r2)
  ret void
}

; CHECK-LABEL: define void @main_reduce_or_mixed()
; The OR-reduce of <false, true, false, false> is i1 true.
; CHECK-NOT: dx.op.vectorReduce
; CHECK: call void @dx.op.storeOutput.i32(i32 5, i32 0, i32 0, i8 0, i32 1)
define void @main_reduce_or_mixed() {
  %r = call i1 @dx.op.vectorReduce.v4i1(i32 310, <4 x i1> <i1 false, i1 true, i1 false, i1 false>)
  %r2 = zext i1 %r to i32
  call void @dx.op.storeOutput.i32(i32 5, i32 0, i32 0, i8 0, i32 %r2)
  ret void
}

; CHECK-LABEL: define void @main_reduce_and_zero()
; The AND-reduce of an all-zero vector is i1 false.
; CHECK-NOT: dx.op.vectorReduce
; CHECK: call void @dx.op.storeOutput.i32(i32 5, i32 0, i32 0, i8 0, i32 0)
define void @main_reduce_and_zero() {
  %r = call i1 @dx.op.vectorReduce.v4i1(i32 309, <4 x i1> zeroinitializer)
  %r2 = zext i1 %r to i32
  call void @dx.op.storeOutput.i32(i32 5, i32 0, i32 0, i8 0, i32 %r2)
  ret void
}

; CHECK-LABEL: define void @main_reduce_and_ones()
; The AND-reduce of an all-one vector is i1 true.
; CHECK-NOT: dx.op.vectorReduce
; CHECK: call void @dx.op.storeOutput.i32(i32 5, i32 0, i32 0, i8 0, i32 1)
define void @main_reduce_and_ones() {
  %r = call i1 @dx.op.vectorReduce.v4i1(i32 309, <4 x i1> <i1 true, i1 true, i1 true, i1 true>)
  %r2 = zext i1 %r to i32
  call void @dx.op.storeOutput.i32(i32 5, i32 0, i32 0, i8 0, i32 %r2)
  ret void
}

; CHECK-LABEL: define void @main_reduce_and_mixed()
; The AND-reduce of <true, true, false, true> is i1 false.
; CHECK-NOT: dx.op.vectorReduce
; CHECK: call void @dx.op.storeOutput.i32(i32 5, i32 0, i32 0, i8 0, i32 0)
define void @main_reduce_and_mixed() {
  %r = call i1 @dx.op.vectorReduce.v4i1(i32 309, <4 x i1> <i1 true, i1 true, i1 false, i1 true>)
  %r2 = zext i1 %r to i32
  call void @dx.op.storeOutput.i32(i32 5, i32 0, i32 0, i8 0, i32 %r2)
  ret void
}

; CHECK-LABEL: define void @main_reduce_or_i32_const()
; The OR-reduce of <i32 1, 2, 4, 8> is i32 15.
; CHECK-NOT: dx.op.vectorReduce
; CHECK: call void @dx.op.storeOutput.i32(i32 5, i32 0, i32 0, i8 0, i32 15)
define void @main_reduce_or_i32_const() {
  %r = call i32 @dx.op.vectorReduce.v4i32(i32 310, <4 x i32> <i32 1, i32 2, i32 4, i32 8>)
  call void @dx.op.storeOutput.i32(i32 5, i32 0, i32 0, i8 0, i32 %r)
  ret void
}

; CHECK-LABEL: define void @main_reduce_or_nonconst(<4 x i1> %v)
; When the vector operand is not a constant the call must remain.
; CHECK: call i1 @dx.op.vectorReduce.v4i1(i32 310, <4 x i1> %v)
define void @main_reduce_or_nonconst(<4 x i1> %v) {
  %r = call i1 @dx.op.vectorReduce.v4i1(i32 310, <4 x i1> %v)
  %r2 = zext i1 %r to i32
  call void @dx.op.storeOutput.i32(i32 5, i32 0, i32 0, i8 0, i32 %r2)
  ret void
}

declare i1 @dx.op.vectorReduce.v4i1(i32, <4 x i1>) #0
declare i32 @dx.op.vectorReduce.v4i32(i32, <4 x i32>) #0
declare void @dx.op.storeOutput.i32(i32, i32, i32, i8, i32) #1

attributes #0 = { nounwind readnone }
attributes #1 = { nounwind }

!dx.version = !{!0}
!dx.shaderModel = !{!1}
!dx.entryPoints = !{!2}

!0 = !{i32 1, i32 9}
!1 = !{!"cs", i32 6, i32 9}
!2 = !{void ()* @main_reduce_or_zero, !"main_reduce_or_zero", null, null, !3}
!3 = !{i32 4, !4}
!4 = !{i32 1, i32 1, i32 1}
