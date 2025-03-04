; RUN: %opt-exe %s -scopenested -scopenestinfo -analyze -S | %FileCheck %s
; CHECK: ScopeNestInfo:
; CHECK: @TopLevel_Begin
; CHECK:     if0
; CHECK:     @If_Begin
; CHECK:         if0.T
; CHECK:     @If_Else
; CHECK:         if0.F
; CHECK:     @If_End
; CHECK:     dx.EndIfScope
; CHECK:     if0.end
; CHECK:     @Loop_Begin
; CHECK:         loop0
; CHECK:         @If_Begin
; CHECK:             dx.LoopExitHelper.3
; CHECK:             @Loop_Break
; CHECK:         @If_Else
; CHECK:             loop0.latch
; CHECK:             dx.LoopContinue.4
; CHECK:             @Loop_Continue
; CHECK:         @If_End
; CHECK:         dx.LoopLatch.1
; CHECK:     @Loop_End
; CHECK:     dx.LoopExit.2
; CHECK:     loop0.exit
; CHECK:     @Loop_Begin
; CHECK:         loop1
; CHECK:         @If_Begin
; CHECK:             dx.LoopExitHelper
; CHECK:             @Loop_Break
; CHECK:         @If_Else
; CHECK:             loop1.latch
; CHECK:             dx.LoopContinue
; CHECK:             @Loop_Continue
; CHECK:         @If_End
; CHECK:         dx.LoopLatch
; CHECK:     @Loop_End
; CHECK:     dx.LoopExit
; CHECK:     loop1.exit
; CHECK: @TopLevel_End

define i32 @main(i1 %c1, i1 %c2, i1 %c3, i32 %i1, i32 %i2) {
if0:
  %res = alloca i32
  store i32 %i1, i32 *%res
  br i1 %c1, label %if0.T, label %if0.F

if0.T:
  store i32 2, i32 *%res
  br label %if0.end

if0.F:
  store i32 7, i32 *%res
  br label %if0.end

if0.end:
  %if.0 = load i32, i32 *%res
  %if.1 = mul i32 %if.0, 3
  store i32 %if.1, i32 *%res
  br label %loop0

loop0:
  %0 = load i32, i32 *%res
  %1 = icmp eq i32 %0, 20
  br i1 %1, label %loop0.exit, label %loop0.latch

loop0.latch:
  store i32 %i2, i32 *%res
  br label %loop0

loop0.exit:
  %2 = load i32, i32 *%res
  %3 = add i32 %2, 33
  store i32 %3, i32 *%res
  br label %loop1

loop1:
  %l1.0 = load i32, i32 *%res
  %l1.1 = icmp eq i32 %l1.0, 20
  br i1 %l1.1, label %loop1.exit, label %loop1.latch

loop1.latch:
  store i32 %i2, i32 *%res
  br label %loop1

loop1.exit:
  %l1.2 = load i32, i32 *%res
  ret i32 %l1.2
}
