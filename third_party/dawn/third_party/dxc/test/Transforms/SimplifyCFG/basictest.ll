; Test CFG simplify removal of branch instructions.
;
; RUN: opt < %s -simplifycfg -S | FileCheck %s
; RUN: opt < %s -passes=simplify-cfg -S | FileCheck %s

define void @test1() {
        br label %1
        ret void
; CHECK-LABEL: @test1(
; CHECK-NEXT: ret void
}

define void @test2() {
        ret void
        ret void
; CHECK-LABEL: @test2(
; CHECK-NEXT: ret void
; CHECK-NEXT: }
}

define void @test3(i1 %T) {
        br i1 %T, label %1, label %1
        ret void
; CHECK-LABEL: @test3(
; CHECK-NEXT: ret void
}


; PR5795
define void @test5(i32 %A) {
  switch i32 %A, label %return [
    i32 2, label %1
    i32 10, label %2
  ]

  ret void

  ret void

return:                                           ; preds = %entry
  ret void
; CHECK-LABEL: @test5(
; CHECK-NEXT: ret void
}


; PR14893
define i8 @test6f() {
; CHECK-LABEL: @test6f
; CHECK: alloca i8, align 1
; CHECK-NEXT: call i8 @test6g
; CHECK-NEXT: icmp eq i8 %tmp, 0
; CHECK-NEXT: load i8, i8* %r, align 1{{$}}

bb0:
  %r = alloca i8, align 1
  %tmp = call i8 @test6g(i8* %r)
  %tmp1 = icmp eq i8 %tmp, 0
  br i1 %tmp1, label %bb2, label %bb1
bb1:
  %tmp3 = load i8, i8* %r, align 1, !range !2, !tbaa !1
  %tmp4 = icmp eq i8 %tmp3, 1
  br i1 %tmp4, label %bb2, label %bb3
bb2:
  br label %bb3
bb3:
  %tmp6 = phi i8 [ 0, %bb2 ], [ 1, %bb1 ]
  ret i8 %tmp6
}
declare i8 @test6g(i8*)

!0 = !{!1, !1, i64 0}
!1 = !{!"foo"}
!2 = !{i8 0, i8 2}
