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
; CHECK:                 dx.LoopExitHelper.1
; CHECK:                 @Loop_Break
; CHECK:             @If_Else
; CHECK:                 loop0.latch
; CHECK:                 dx.LoopContinue
; CHECK:                 @Loop_Continue
; CHECK:             @If_End
; CHECK:         @If_End
; CHECK:         dx.LoopLatch
; CHECK:     @Loop_End
; CHECK:     dx.LoopExit
; CHECK:     dx.LoopExitHelperIf
; CHECK:     @If_Begin
; CHECK:         loop0.exit1
; CHECK:     @If_End
; CHECK:     dx.EndIfScope
; CHECK:     loop0.exit2
; CHECK: @TopLevel_End

define i32 @main(i1 %c1, i1 %c2, i1 %c3, i32 %i1, i32 %i2) {
entry:
  %res = alloca i32
  store i32 %i1, i32 *%res
  br label %loop0

loop0:
  %0 = load i32, i32 *%res
  %1 = icmp eq i32 %0, 20
  br i1 %1, label %loop0.exit1, label %loop0.body0

loop0.body0:
  %cond2 = icmp eq i32 %i2, 10
  br i1 %cond2, label %loop0.exit2, label %loop0.latch

loop0.latch:
  store i32 %i2, i32 *%res
  br label %loop0

loop0.exit1:
  %r1 = load i32, i32 *%res
  %r2 = add i32 %r1, 3
  store i32 %r2, i32 *%res
  br label %loop0.exit2

loop0.exit2:
  %r3 = load i32, i32 *%res
  ret i32 %r3
}
