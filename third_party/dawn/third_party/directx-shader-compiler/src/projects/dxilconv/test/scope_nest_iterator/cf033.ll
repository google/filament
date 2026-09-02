; RUN: %opt-exe %s -scopenested -scopenestinfo -analyze -S | %FileCheck %s
; CHECK: ScopeNestInfo:
; CHECK: @TopLevel_Begin
; CHECK:     entry
; CHECK:     @Loop_Begin
; CHECK:         loop0
; CHECK:         @If_Begin
; CHECK:             dx.LoopExitHelper
; CHECK:             @Loop_Break
; CHECK:         @If_Else
; CHECK:             loop0.body0
; CHECK:             @If_Begin
; CHECK:             @If_Else
; CHECK:                 loop0.body1
; CHECK:             @If_End
; CHECK:             dx.EndIfScope
; CHECK:             loop0.latch
; CHECK:             dx.LoopContinue
; CHECK:             @Loop_Continue
; CHECK:         @If_End
; CHECK:         dx.LoopLatch
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
  %0 = load i32, i32 *%res
  %1 = icmp eq i32 %0, 20
  br i1 %1, label %loop0.exit, label %loop0.body0

loop0.body0:
  store i32 7, i32 *%res
  br i1 %c2, label %loop0.latch, label %loop0.body1

loop0.body1:
  store i32 71, i32 *%res
  br label %loop0.latch

loop0.latch:
  store i32 %i2, i32 *%res
  br label %loop0

loop0.exit:
  %2 = load i32, i32 *%res
  ret i32 %2
}
