; This test makes sure that these instructions are properly eliminated.
;
; RUN: opt < %s -instcombine -S | \
; RUN:    FileCheck %s
; END.
@G1 = global i32 0		; <i32*> [#uses=1]
@G2 = global i32 0		; <i32*> [#uses=1]

define i1 @test0(i1 %A) {
; CHECK-LABEL: @test0(
; CHECK-NEXT: ret i1 %A
	%B = xor i1 %A, false		; <i1> [#uses=1]
	ret i1 %B
}

define i32 @test1(i32 %A) {
; CHECK-LABEL: @test1(
; CHECK-NEXT: ret i32 %A
	%B = xor i32 %A, 0		; <i32> [#uses=1]
	ret i32 %B
}

define i1 @test2(i1 %A) {
; CHECK-LABEL: @test2(
; CHECK-NEXT: ret i1 false
	%B = xor i1 %A, %A		; <i1> [#uses=1]
	ret i1 %B
}

define i32 @test3(i32 %A) {
; CHECK-LABEL: @test3(
; CHECK-NEXT: ret i32 0
	%B = xor i32 %A, %A		; <i32> [#uses=1]
	ret i32 %B
}

define i32 @test4(i32 %A) {
; CHECK-LABEL: @test4(
; CHECK-NEXT: ret i32 -1
	%NotA = xor i32 -1, %A		; <i32> [#uses=1]
	%B = xor i32 %A, %NotA		; <i32> [#uses=1]
	ret i32 %B
}

define i32 @test5(i32 %A) {
; CHECK-LABEL: @test5(
; CHECK-NEXT: %1 = and i32 %A, -124
; CHECK-NEXT: ret i32 %1
	%t1 = or i32 %A, 123		; <i32> [#uses=1]
	%r = xor i32 %t1, 123		; <i32> [#uses=1]
	ret i32 %r
}

define i8 @test6(i8 %A) {
; CHECK-LABEL: @test6(
; CHECK-NEXT: ret i8 %A
	%B = xor i8 %A, 17		; <i8> [#uses=1]
	%C = xor i8 %B, 17		; <i8> [#uses=1]
	ret i8 %C
}

define i32 @test7(i32 %A, i32 %B) {
; CHECK-LABEL: @test7(
; CHECK-NEXT: %A1 = and i32 %A, 7
; CHECK-NEXT: %B1 = and i32 %B, 128
; CHECK-NEXT: %C1.1 = or i32 %A1, %B1
; CHECK-NEXT: ret i32 %C1.1
	%A1 = and i32 %A, 7		; <i32> [#uses=1]
	%B1 = and i32 %B, 128		; <i32> [#uses=1]
	%C1 = xor i32 %A1, %B1		; <i32> [#uses=1]
	ret i32 %C1
}

define i8 @test8(i1 %c) {
; CHECK-LABEL: @test8(
; CHECK: br i1 %c, label %False, label %True
	%d = xor i1 %c, true		; <i1> [#uses=1]
	br i1 %d, label %True, label %False

True:		; preds = %0
	ret i8 1

False:		; preds = %0
	ret i8 3
}

define i1 @test9(i8 %A) {
; CHECK-LABEL: @test9(
; CHECK-NEXT: %C = icmp eq i8 %A, 89
; CHECK-NEXT: ret i1 %C
	%B = xor i8 %A, 123		; <i8> [#uses=1]
	%C = icmp eq i8 %B, 34		; <i1> [#uses=1]
	ret i1 %C
}

define i8 @test10(i8 %A) {
; CHECK-LABEL: @test10(
; CHECK-NEXT: %B = and i8 %A, 3
; CHECK-NEXT: %C.1 = or i8 %B, 4
; CHECK-NEXT: ret i8 %C.1
	%B = and i8 %A, 3		; <i8> [#uses=1]
	%C = xor i8 %B, 4		; <i8> [#uses=1]
	ret i8 %C
}

define i8 @test11(i8 %A) {
; CHECK-LABEL: @test11(
; CHECK-NEXT: %B = and i8 %A, -13
; CHECK-NEXT: %1 = or i8 %B, 8
; CHECK-NEXT: ret i8 %1
	%B = or i8 %A, 12		; <i8> [#uses=1]
	%C = xor i8 %B, 4		; <i8> [#uses=1]
	ret i8 %C
}

define i1 @test12(i8 %A) {
; CHECK-LABEL: @test12(
; CHECK-NEXT: %c = icmp ne i8 %A, 4
; CHECK-NEXT: ret i1 %c
	%B = xor i8 %A, 4		; <i8> [#uses=1]
	%c = icmp ne i8 %B, 0		; <i1> [#uses=1]
	ret i1 %c
}

define i1 @test13(i8 %A, i8 %B) {
; CHECK-LABEL: @test13(
; CHECK-NEXT: %1 = icmp ne i8 %A, %B
; CHECK-NEXT: ret i1 %1
	%C = icmp ult i8 %A, %B		; <i1> [#uses=1]
	%D = icmp ugt i8 %A, %B		; <i1> [#uses=1]
	%E = xor i1 %C, %D		; <i1> [#uses=1]
	ret i1 %E
}

define i1 @test14(i8 %A, i8 %B) {
; CHECK-LABEL: @test14(
; CHECK-NEXT: ret i1 true
	%C = icmp eq i8 %A, %B		; <i1> [#uses=1]
	%D = icmp ne i8 %B, %A		; <i1> [#uses=1]
	%E = xor i1 %C, %D		; <i1> [#uses=1]
	ret i1 %E
}

define i32 @test15(i32 %A) {
; CHECK-LABEL: @test15(
; CHECK-NEXT: %C = sub i32 0, %A
; CHECK-NEXT: ret i32 %C
	%B = add i32 %A, -1		; <i32> [#uses=1]
	%C = xor i32 %B, -1		; <i32> [#uses=1]
	ret i32 %C
}

define i32 @test16(i32 %A) {
; CHECK-LABEL: @test16(
; CHECK-NEXT: %C = sub i32 -124, %A
; CHECK-NEXT: ret i32 %C
	%B = add i32 %A, 123		; <i32> [#uses=1]
	%C = xor i32 %B, -1		; <i32> [#uses=1]
	ret i32 %C
}

define i32 @test17(i32 %A) {
; CHECK-LABEL: @test17(
; CHECK-NEXT: %C = add i32 %A, -124
; CHECK-NEXT: ret i32 %C
	%B = sub i32 123, %A		; <i32> [#uses=1]
	%C = xor i32 %B, -1		; <i32> [#uses=1]
	ret i32 %C
}

define i32 @test18(i32 %A) {
; CHECK-LABEL: @test18(
; CHECK-NEXT: %C = add i32 %A, 124
; CHECK-NEXT: ret i32 %C
	%B = xor i32 %A, -1		; <i32> [#uses=1]
	%C = sub i32 123, %B		; <i32> [#uses=1]
	ret i32 %C
}

define i32 @test19(i32 %A, i32 %B) {
; CHECK-LABEL: @test19(
; CHECK-NEXT: ret i32 %B
	%C = xor i32 %A, %B		; <i32> [#uses=1]
	%D = xor i32 %C, %A		; <i32> [#uses=1]
	ret i32 %D
}

define void @test20(i32 %A, i32 %B) {
; CHECK-LABEL: @test20(
; CHECK-NEXT: store i32 %B, i32* @G1
; CHECK-NEXT: store i32 %A, i32* @G2
; CHECK-NEXT: ret void
	%tmp.2 = xor i32 %B, %A		; <i32> [#uses=2]
	%tmp.5 = xor i32 %tmp.2, %B		; <i32> [#uses=2]
	%tmp.8 = xor i32 %tmp.5, %tmp.2		; <i32> [#uses=1]
	store i32 %tmp.8, i32* @G1
	store i32 %tmp.5, i32* @G2
	ret void
}

define i32 @test21(i1 %C, i32 %A, i32 %B) {
; CHECK-LABEL: @test21(
; CHECK-NEXT: %D = select i1 %C, i32 %B, i32 %A
; CHECK-NEXT: ret i32 %D
	%C2 = xor i1 %C, true		; <i1> [#uses=1]
	%D = select i1 %C2, i32 %A, i32 %B		; <i32> [#uses=1]
	ret i32 %D
}

define i32 @test22(i1 %X) {
; CHECK-LABEL: @test22(
; CHECK-NEXT: %1 = zext i1 %X to i32
; CHECK-NEXT: ret i32 %1
	%Y = xor i1 %X, true		; <i1> [#uses=1]
	%Z = zext i1 %Y to i32		; <i32> [#uses=1]
	%Q = xor i32 %Z, 1		; <i32> [#uses=1]
	ret i32 %Q
}

define i1 @test23(i32 %a, i32 %b) {
; CHECK-LABEL: @test23(
; CHECK-NEXT: %tmp.4 = icmp eq i32 %b, 0
; CHECK-NEXT: ret i1 %tmp.4
	%tmp.2 = xor i32 %b, %a		; <i32> [#uses=1]
	%tmp.4 = icmp eq i32 %tmp.2, %a		; <i1> [#uses=1]
	ret i1 %tmp.4
}

define i1 @test24(i32 %c, i32 %d) {
; CHECK-LABEL: @test24(
; CHECK-NEXT: %tmp.4 = icmp ne i32 %d, 0
; CHECK-NEXT: ret i1 %tmp.4
	%tmp.2 = xor i32 %d, %c		; <i32> [#uses=1]
	%tmp.4 = icmp ne i32 %tmp.2, %c		; <i1> [#uses=1]
	ret i1 %tmp.4
}

define i32 @test25(i32 %g, i32 %h) {
; CHECK-LABEL: @test25(
; CHECK-NEXT: %tmp4 = and i32 %h, %g
; CHECK-NEXT: ret i32 %tmp4
	%h2 = xor i32 %h, -1		; <i32> [#uses=1]
	%tmp2 = and i32 %h2, %g		; <i32> [#uses=1]
	%tmp4 = xor i32 %tmp2, %g		; <i32> [#uses=1]
	ret i32 %tmp4
}

define i32 @test26(i32 %a, i32 %b) {
; CHECK-LABEL: @test26(
; CHECK-NEXT: %tmp4 = and i32 %a, %b
; CHECK-NEXT: ret i32 %tmp4
	%b2 = xor i32 %b, -1		; <i32> [#uses=1]
	%tmp2 = xor i32 %a, %b2		; <i32> [#uses=1]
	%tmp4 = and i32 %tmp2, %a		; <i32> [#uses=1]
	ret i32 %tmp4
}

define i32 @test27(i32 %b, i32 %c, i32 %d) {
; CHECK-LABEL: @test27(
; CHECK-NEXT: %tmp = icmp eq i32 %b, %c
; CHECK-NEXT: %tmp6 = zext i1 %tmp to i32
; CHECK-NEXT: ret i32 %tmp6
	%tmp2 = xor i32 %d, %b		; <i32> [#uses=1]
	%tmp5 = xor i32 %d, %c		; <i32> [#uses=1]
	%tmp = icmp eq i32 %tmp2, %tmp5		; <i1> [#uses=1]
	%tmp6 = zext i1 %tmp to i32		; <i32> [#uses=1]
	ret i32 %tmp6
}

define i32 @test28(i32 %indvar) {
; CHECK-LABEL: @test28(
; CHECK-NEXT: %tmp214 = add i32 %indvar, 1
; CHECK-NEXT: ret i32 %tmp214
	%tmp7 = add i32 %indvar, -2147483647		; <i32> [#uses=1]
	%tmp214 = xor i32 %tmp7, -2147483648		; <i32> [#uses=1]
	ret i32 %tmp214
}
