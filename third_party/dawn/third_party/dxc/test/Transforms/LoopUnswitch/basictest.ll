; RUN: opt < %s -loop-unswitch -verify-loop-info -S < %s 2>&1 | FileCheck %s

define i32 @test(i32* %A, i1 %C) {
entry:
	br label %no_exit
no_exit:		; preds = %no_exit.backedge, %entry
	%i.0.0 = phi i32 [ 0, %entry ], [ %i.0.0.be, %no_exit.backedge ]		; <i32> [#uses=3]
	%gep.upgrd.1 = zext i32 %i.0.0 to i64		; <i64> [#uses=1]
	%tmp.7 = getelementptr i32, i32* %A, i64 %gep.upgrd.1		; <i32*> [#uses=4]
	%tmp.13 = load i32, i32* %tmp.7		; <i32> [#uses=2]
	%tmp.14 = add i32 %tmp.13, 1		; <i32> [#uses=1]
	store i32 %tmp.14, i32* %tmp.7
	br i1 %C, label %then, label %endif
then:		; preds = %no_exit
	%tmp.29 = load i32, i32* %tmp.7		; <i32> [#uses=1]
	%tmp.30 = add i32 %tmp.29, 2		; <i32> [#uses=1]
	store i32 %tmp.30, i32* %tmp.7
	%inc9 = add i32 %i.0.0, 1		; <i32> [#uses=2]
	%tmp.112 = icmp ult i32 %inc9, 100000		; <i1> [#uses=1]
	br i1 %tmp.112, label %no_exit.backedge, label %return
no_exit.backedge:		; preds = %endif, %then
	%i.0.0.be = phi i32 [ %inc9, %then ], [ %inc, %endif ]		; <i32> [#uses=1]
	br label %no_exit
endif:		; preds = %no_exit
	%inc = add i32 %i.0.0, 1		; <i32> [#uses=2]
	%tmp.1 = icmp ult i32 %inc, 100000		; <i1> [#uses=1]
	br i1 %tmp.1, label %no_exit.backedge, label %return
return:		; preds = %endif, %then
	ret i32 %tmp.13
}

; This simple test would normally unswitch, but should be inhibited by the presence of
; the noduplicate call.

; CHECK-LABEL: @test2(
define i32 @test2(i32* %var) {
  %mem = alloca i32
  store i32 2, i32* %mem
  %c = load i32, i32* %mem

  br label %loop_begin

loop_begin:

  %var_val = load i32, i32* %var

  switch i32 %c, label %default [
      i32 1, label %inc
      i32 2, label %dec
  ]

inc:
  call void @incf() noreturn nounwind
  br label %loop_begin
dec:
; CHECK: call void @decf()
; CHECK-NOT: call void @decf()
  call void @decf() noreturn nounwind noduplicate
  br label %loop_begin
default:
  br label %loop_exit
loop_exit:
  ret i32 0
; CHECK: }
}

declare void @incf() noreturn
declare void @decf() noreturn
