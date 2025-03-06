; This test checks to make sure that constant exprs fold in some simple 
; situations

; RUN: llvm-as < %s | llvm-dis | not grep "("
; RUN: verify-uselistorder %s

@A = global i64 0

global i64* inttoptr (i64 add (i64 ptrtoint (i64* @A to i64), i64 0) to i64*) ; X + 0 == X
global i64* inttoptr (i64 sub (i64 ptrtoint (i64* @A to i64), i64 0) to i64*) ; X - 0 == X
global i64* inttoptr (i64 mul (i64 ptrtoint (i64* @A to i64), i64 0) to i64*) ; X * 0 == 0
global i64* inttoptr (i64 sdiv (i64 ptrtoint (i64* @A to i64), i64 1) to i64*) ; X / 1 == X
global i64* inttoptr (i64 srem (i64 ptrtoint (i64* @A to i64), i64 1) to i64*) ; X % 1 == 0
global i64* inttoptr (i64 and (i64 ptrtoint (i64* @A to i64), i64 0) to i64*) ; X & 0 == 0
global i64* inttoptr (i64 and (i64 ptrtoint (i64* @A to i64), i64 -1) to i64*) ; X & -1 == X
global i64 or (i64 ptrtoint (i64* @A to i64), i64 -1)  ; X | -1 == -1
global i64* inttoptr (i64 xor (i64 ptrtoint (i64* @A to i64), i64 0) to i64*) ; X ^ 0 == X

%Ty = type { i32, i32 }
@B = external global %Ty 

global i1 icmp slt (i64* @A, i64* getelementptr (i64, i64* @A, i64 1))        ; true
global i1 icmp ult (i64* @A, i64* getelementptr (i64, i64* @A, i64 1))        ; true
global i1 icmp slt (i64* @A, i64* getelementptr (i64, i64* @A, i64 0))        ; false
global i1 icmp slt (i32* getelementptr (%Ty, %Ty* @B, i64 0, i32 0), 
                   i32* getelementptr (%Ty, %Ty* @B, i64 0, i32 1))            ; true
;global i1 icmp ne (i64* @A, i64* bitcast (%Ty* @B to i64*))                 ; true

; PR2206
@cons = weak global i32 0, align 8              ; <i32*> [#uses=1]
global i64 and (i64 ptrtoint (i32* @cons to i64), i64 7)

