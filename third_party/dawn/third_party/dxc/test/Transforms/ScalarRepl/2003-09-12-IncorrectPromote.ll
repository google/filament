; Scalar replacement was incorrectly promoting this alloca!!
;
; RUN: opt < %s -scalarrepl -S | FileCheck %s

define i8* @test() {
	%A = alloca [30 x i8]		; <[30 x i8]*> [#uses=1]
	%B = getelementptr [30 x i8], [30 x i8]* %A, i64 0, i64 0		; <i8*> [#uses=2]
	%C = getelementptr i8, i8* %B, i64 1		; <i8*> [#uses=1]
	store i8 0, i8* %B
	ret i8* %C
}
; CHECK: alloca [
