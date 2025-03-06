; RUN: %opt-exe %s -scopenested -scopenestinfo -analyze -S | %FileCheck %s
; CHECK: ScopeNestInfo:
; CHECK: @TopLevel_Begin
; CHECK:     dx.bb.0
; CHECK:     @Loop_Begin
; CHECK:         loop0
; CHECK:         @If_Begin
; CHECK:             dx.LoopExitHelper
; CHECK:             @Loop_Break
; CHECK:         @If_Else
; CHECK:             dx.LoopContinue
; CHECK:             @Loop_Continue
; CHECK:         @If_End
; CHECK:         dx.LoopLatch
; CHECK:     @Loop_End
; CHECK:     dx.LoopExit
; CHECK:     loop0.exit
; CHECK: @TopLevel_End

define i32 @main(i1 %c1, i1 %c2, i1 %c3, i32 %i1, i32 %i2) {
dx.bb.0:
  %res = alloca i32
  store i32 %i1, i32 *%res
  br label %loop0

loop0:
  %0 = load i32, i32 *%res
  %1 = icmp eq i32 %0, 20
  br i1 %1, label %loop0.exit, label %loop0

loop0.exit:
  %v3 = load i32, i32 *%res
  %2 = add i32 %v3, 33
  store i32 %2, i32 *%res
  ret i32 %2
}
