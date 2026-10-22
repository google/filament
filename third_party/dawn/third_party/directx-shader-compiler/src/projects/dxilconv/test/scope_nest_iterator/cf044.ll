; RUN: %opt-exe %s -scopenested -scopenestinfo -analyze -S | %FileCheck %s
; CHECK: ScopeNestInfo:
; CHECK: @TopLevel_Begin
; CHECK:     entry
; CHECK:     @Loop_Begin
; CHECK:         loop0
; CHECK:         if0
; CHECK:         @If_Begin
; CHECK:             if0.T
; CHECK:         @If_Else
; CHECK:             if0.F
; CHECK:         @If_End
; CHECK:         dx.EndIfScope
; CHECK:         if0.exit
; CHECK:         @If_Begin
; CHECK:             dx.LoopContinue
; CHECK:             @Loop_Continue
; CHECK:         @If_Else
; CHECK:             dx.LoopExitHelper
; CHECK:             @Loop_Break
; CHECK:         @If_End
; CHECK:         loop0.latch
; CHECK:     @Loop_End
; CHECK:     dx.LoopExit
; CHECK:     loop0.exit
; CHECK: @TopLevel_End

define i32 @main(i1 %c1, i1 %c2, i1 %c3, i32 %i1, i32 %i2) {
entry:
  %res = alloca i32
  store i32 %i1, i32 *%res
  br label %loop0

loop0:
  %loop0.0 = load i32, i32 *%res
  %loop0.1 = icmp eq i32 %loop0.0, 20
  br label %if0

if0:
  br i1 %c1, label %if0.T, label %if0.F

if0.T:
  %if0.T.0 = load i32, i32 *%res
  %if0.T.1 = add i32 %if0.T.0, 1
  store i32 %if0.T.1, i32 *%res
  br label %if0.exit

if0.F:
  store i32 7, i32 *%res
  br label %if0.exit

if0.exit:
  br i1 %c2, label %loop0.latch, label %loop0.exit

loop0.latch:
  br label %loop0

loop0.exit:
  %ret1 = load i32, i32 *%res
  ret i32 %ret1
}
