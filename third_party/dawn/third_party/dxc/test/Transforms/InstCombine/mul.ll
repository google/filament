; This test makes sure that mul instructions are properly eliminated.
; RUN: opt < %s -instcombine -S | FileCheck %s

define i32 @test1(i32 %A) {
; CHECK-LABEL: @test1(
        %B = mul i32 %A, 1              ; <i32> [#uses=1]
        ret i32 %B
; CHECK: ret i32 %A
}

define i32 @test2(i32 %A) {
; CHECK-LABEL: @test2(
        ; Should convert to an add instruction
        %B = mul i32 %A, 2              ; <i32> [#uses=1]
        ret i32 %B
; CHECK: shl i32 %A, 1
}

define i32 @test3(i32 %A) {
; CHECK-LABEL: @test3(
        ; This should disappear entirely
        %B = mul i32 %A, 0              ; <i32> [#uses=1]
        ret i32 %B
; CHECK: ret i32 0
}

define double @test4(double %A) {
; CHECK-LABEL: @test4(
        ; This is safe for FP
        %B = fmul double 1.000000e+00, %A                ; <double> [#uses=1]
        ret double %B
; CHECK: ret double %A
}

define i32 @test5(i32 %A) {
; CHECK-LABEL: @test5(
        %B = mul i32 %A, 8              ; <i32> [#uses=1]
        ret i32 %B
; CHECK: shl i32 %A, 3
}

define i8 @test6(i8 %A) {
; CHECK-LABEL: @test6(
        %B = mul i8 %A, 8               ; <i8> [#uses=1]
        %C = mul i8 %B, 8               ; <i8> [#uses=1]
        ret i8 %C
; CHECK: shl i8 %A, 6
}

define i32 @test7(i32 %i) {
; CHECK-LABEL: @test7(
        %tmp = mul i32 %i, -1           ; <i32> [#uses=1]
        ret i32 %tmp
; CHECK: sub i32 0, %i
}

define i64 @test8(i64 %i) {
; CHECK-LABEL: @test8(
        %j = mul i64 %i, -1             ; <i64> [#uses=1]
        ret i64 %j
; CHECK: sub i64 0, %i
}

define i32 @test9(i32 %i) {
; CHECK-LABEL: @test9(
        %j = mul i32 %i, -1             ; <i32> [#uses=1]
        ret i32 %j
; CHECK: sub i32 0, %i
}

define i32 @test10(i32 %a, i32 %b) {
; CHECK-LABEL: @test10(
        %c = icmp slt i32 %a, 0         ; <i1> [#uses=1]
        %d = zext i1 %c to i32          ; <i32> [#uses=1]
       ; e = b & (a >> 31)
        %e = mul i32 %d, %b             ; <i32> [#uses=1]
        ret i32 %e
; CHECK: [[TEST10:%.*]] = ashr i32 %a, 31
; CHECK-NEXT: %e = and i32 [[TEST10]], %b
; CHECK-NEXT: ret i32 %e
}

define i32 @test11(i32 %a, i32 %b) {
; CHECK-LABEL: @test11(
        %c = icmp sle i32 %a, -1                ; <i1> [#uses=1]
        %d = zext i1 %c to i32          ; <i32> [#uses=1]
        ; e = b & (a >> 31)
        %e = mul i32 %d, %b             ; <i32> [#uses=1]
        ret i32 %e
; CHECK: [[TEST11:%.*]] = ashr i32 %a, 31
; CHECK-NEXT: %e = and i32 [[TEST11]], %b
; CHECK-NEXT: ret i32 %e
}

define i32 @test12(i32 %a, i32 %b) {
; CHECK-LABEL: @test12(
        %c = icmp ugt i32 %a, 2147483647                ; <i1> [#uses=1]
        %d = zext i1 %c to i32          ; <i32> [#uses=1]
        %e = mul i32 %d, %b             ; <i32> [#uses=1]
        ret i32 %e
; CHECK: [[TEST12:%.*]] = ashr i32 %a, 31
; CHECK-NEXT: %e = and i32 [[TEST12]], %b
; CHECK-NEXT: ret i32 %e

}

; PR2642
define internal void @test13(<4 x float>*) {
; CHECK-LABEL: @test13(
	load <4 x float>, <4 x float>* %0, align 1
	fmul <4 x float> %2, < float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00 >
	store <4 x float> %3, <4 x float>* %0, align 1
	ret void
; CHECK-NEXT: ret void
}

define <16 x i8> @test14(<16 x i8> %a) {
; CHECK-LABEL: @test14(
        %b = mul <16 x i8> %a, zeroinitializer
        ret <16 x i8> %b
; CHECK-NEXT: ret <16 x i8> zeroinitializer
}

; rdar://7293527
define i32 @test15(i32 %A, i32 %B) {
; CHECK-LABEL: @test15(
entry:
  %shl = shl i32 1, %B
  %m = mul i32 %shl, %A
  ret i32 %m
; CHECK: shl i32 %A, %B
}

; X * Y (when Y is 0 or 1) --> x & (0-Y)
define i32 @test16(i32 %b, i1 %c) {
; CHECK-LABEL: @test16(
        %d = zext i1 %c to i32          ; <i32> [#uses=1]
        ; e = b & (a >> 31)
        %e = mul i32 %d, %b             ; <i32> [#uses=1]
        ret i32 %e
; CHECK: [[TEST16:%.*]] = select i1 %c, i32 %b, i32 0
; CHECK-NEXT: ret i32 [[TEST16]]
}

; X * Y (when Y is 0 or 1) --> x & (0-Y)
define i32 @test17(i32 %a, i32 %b) {
; CHECK-LABEL: @test17(
  %a.lobit = lshr i32 %a, 31
  %e = mul i32 %a.lobit, %b
  ret i32 %e
; CHECK: [[TEST17:%.*]] = ashr i32 %a, 31
; CHECK-NEXT: %e = and i32 [[TEST17]], %b
; CHECK-NEXT: ret i32 %e
}

define i32 @test18(i32 %A, i32 %B) {
; CHECK-LABEL: @test18(
  %C = and i32 %A, 1
  %D = and i32 %B, 1

  %E = mul i32 %C, %D
  %F = and i32 %E, 16
  ret i32 %F
; CHECK-NEXT: ret i32 0
}

declare {i32, i1} @llvm.smul.with.overflow.i32(i32, i32)
declare void @use(i1)

define i32 @test19(i32 %A, i32 %B) {
; CHECK-LABEL: @test19(
  %C = and i32 %A, 1
  %D = and i32 %B, 1

; It would be nice if we also started proving that this doesn't overflow.
  %E = call {i32, i1} @llvm.smul.with.overflow.i32(i32 %C, i32 %D)
  %F = extractvalue {i32, i1} %E, 0
  %G = extractvalue {i32, i1} %E, 1
  call void @use(i1 %G)
  %H = and i32 %F, 16
  ret i32 %H
; CHECK: ret i32 0
}

define <2 x i64> @test20(<2 x i64> %A) {
; CHECK-LABEL: @test20(
        %B = add <2 x i64> %A, <i64 12, i64 14>
        %C = mul <2 x i64> %B, <i64 3, i64 2>
        ret <2 x i64> %C
; CHECK: mul <2 x i64> %A, <i64 3, i64 2>
; CHECK: add <2 x i64> %{{.}}, <i64 36, i64 28>
}

define <2 x i1> @test21(<2 x i1> %A, <2 x i1> %B) {
; CHECK-LABEL: @test21(
        %C = mul <2 x i1> %A, %B
        ret <2 x i1> %C
; CHECK: %C = and <2 x i1> %A, %B
}

define i32 @test22(i32 %A) {
; CHECK-LABEL: @test22(
        %B = mul nsw i32 %A, -1
        ret i32 %B
; CHECK: sub nsw i32 0, %A
}

define i32 @test23(i32 %A) {
; CHECK-LABEL: @test23(
        %B = shl nuw i32 %A, 1
        %C = mul nuw i32 %B, 3
        ret i32 %C
; CHECK: mul nuw i32 %A, 6
}

define i32 @test24(i32 %A) {
; CHECK-LABEL: @test24(
        %B = shl nsw i32 %A, 1
        %C = mul nsw i32 %B, 3
        ret i32 %C
; CHECK: mul nsw i32 %A, 6
}

define i32 @test25(i32 %A, i32 %B) {
; CHECK-LABEL: @test25(
        %C = sub nsw i32 0, %A
        %D = sub nsw i32 0, %B
        %E = mul nsw i32 %C, %D
        ret i32 %E
; CHECK: mul nsw i32 %A, %B
}

define i32 @test26(i32 %A, i32 %B) {
; CHECK-LABEL: @test26(
        %C = shl nsw i32 1, %B
        %D = mul nsw i32 %A, %C
        ret i32 %D
; CHECK: shl nsw i32 %A, %B
}

define i32 @test27(i32 %A, i32 %B) {
; CHECK-LABEL: @test27(
        %C = shl i32 1, %B
        %D = mul nuw i32 %A, %C
        ret i32 %D
; CHECK: shl nuw i32 %A, %B
}

define i32 @test28(i32 %A) {
; CHECK-LABEL: @test28(
        %B = shl i32 1, %A
        %C = mul nsw i32 %B, %B
        ret i32 %C
; CHECK:      %[[shl1:.*]] = shl i32 1, %A
; CHECK-NEXT: %[[shl2:.*]] = shl i32 %[[shl1]], %A
; CHECK-NEXT: ret i32 %[[shl2]]
}

define i64 @test29(i31 %A, i31 %B) {
; CHECK-LABEL: @test29(
        %C = sext i31 %A to i64
        %D = sext i31 %B to i64
        %E = mul i64 %C, %D
        ret i64 %E
; CHECK:      %[[sext1:.*]] = sext i31 %A to i64
; CHECK-NEXT: %[[sext2:.*]] = sext i31 %B to i64
; CHECK-NEXT: %[[mul:.*]] = mul nsw i64 %[[sext1]], %[[sext2]]
; CHECK-NEXT: ret i64 %[[mul]]
}

define i64 @test30(i32 %A, i32 %B) {
; CHECK-LABEL: @test30(
        %C = zext i32 %A to i64
        %D = zext i32 %B to i64
        %E = mul i64 %C, %D
        ret i64 %E
; CHECK:      %[[zext1:.*]] = zext i32 %A to i64
; CHECK-NEXT: %[[zext2:.*]] = zext i32 %B to i64
; CHECK-NEXT: %[[mul:.*]] = mul nuw i64 %[[zext1]], %[[zext2]]
; CHECK-NEXT: ret i64 %[[mul]]
}

@PR22087 = external global i32
define i32 @test31(i32 %V) {
; CHECK-LABEL: @test31
  %mul = mul i32 %V, shl (i32 1, i32 zext (i1 icmp ne (i32* inttoptr (i64 1 to i32*), i32* @PR22087) to i32))
  ret i32 %mul
; CHECK:      %[[mul:.*]] = shl i32 %V, zext (i1 icmp ne (i32* inttoptr (i64 1 to i32*), i32* @PR22087) to i32)
; CHECK-NEXT: ret i32 %[[mul]]
}

define i32 @test32(i32 %X) {
; CHECK-LABEL: @test32
  %mul = mul nsw i32 %X, -2147483648
  ret i32 %mul
; CHECK:      %[[shl:.*]] = shl i32 %X, 31
; CHECK-NEXT: ret i32 %[[shl]]
}

define i32 @test33(i32 %X) {
; CHECK-LABEL: @test33
  %mul = mul nsw i32 %X, 1073741824
; CHECK:      %[[shl:.*]] = shl nsw i32 %X, 30
; CHECK-NEXT: ret i32 %[[shl]]
  ret i32 %mul
}
