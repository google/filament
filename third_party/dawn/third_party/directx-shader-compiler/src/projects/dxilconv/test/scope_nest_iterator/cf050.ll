; RUN: %opt-exe %s -scopenested -scopenestinfo -analyze -S | %FileCheck %s
; CHECK: ScopeNestInfo:
; CHECK: @TopLevel_Begin
; CHECK:     entry
; CHECK:     @Loop_Begin
; CHECK:         loop0
; CHECK:         @If_Begin
; CHECK:             dx.LoopExitHelper.3
; CHECK:             @Loop_Break
; CHECK:         @If_Else
; CHECK:             loop0.body0
; CHECK:             @Loop_Begin
; CHECK:                 loop1
; CHECK:                 @If_Begin
; CHECK:                     dx.LoopExitHelper
; CHECK:                     @Loop_Break
; CHECK:                 @If_Else
; CHECK:                     loop1.latch
; CHECK:                     dx.LoopContinue
; CHECK:                     @Loop_Continue
; CHECK:                 @If_End
; CHECK:                 dx.LoopLatch
; CHECK:             @Loop_End
; CHECK:             dx.LoopExit
; CHECK:             loop1.exit
; CHECK:             loop0.latch
; CHECK:             dx.LoopContinue.4
; CHECK:             @Loop_Continue
; CHECK:         @If_End
; CHECK:         dx.LoopLatch.1
; CHECK:     @Loop_End
; CHECK:     dx.LoopExit.2
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
  br i1 %loop0.1, label %loop0.exit, label %loop0.body0

loop0.body0:
  br label %loop1

loop1:
  store i32 %i2, i32 *%res
  br i1 %c1, label %loop1.exit, label %loop1.latch

loop1.latch:
  store i32 %i2, i32 *%res
  br label %loop1

loop1.exit:
  %loop1.0 = load i32, i32 *%res
  %loop1.1 = add i32 %loop1.0, 33
  store i32 %loop1.1, i32 *%res
  br label %loop0.latch

loop0.latch:
  store i32 %i2, i32 *%res
  br label %loop0

loop0.exit:
  %ret1 = load i32, i32 *%res
  ret i32 %ret1
}
