; RUN: opt < %s -domtree -break-crit-edges -analyze -domtree | FileCheck %s
; PR932

; CHECK: [3] %brtrue {1,2}

declare void @use1(i32)

define void @f(i32 %i, i1 %c) {
entry:
	%A = icmp eq i32 %i, 0		; <i1> [#uses=1]
	br i1 %A, label %brtrue, label %brfalse

brtrue:		; preds = %brtrue, %entry
	%B = phi i1 [ true, %brtrue ], [ false, %entry ]		; <i1> [#uses=1]
	call void @use1( i32 %i )
	br i1 %B, label %brtrue, label %brfalse

brfalse:		; preds = %brtrue, %entry
	call void @use1( i32 %i )
	ret void
}
