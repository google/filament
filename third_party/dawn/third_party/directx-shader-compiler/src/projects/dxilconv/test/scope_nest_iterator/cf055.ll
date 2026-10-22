; RUN: %opt-exe %s -scopenested -scopenestinfo -analyze -S | %FileCheck %s
; CHECK: ScopeNestInfo:
; CHECK: @TopLevel_Begin
; CHECK:     entry
; CHECK:     @If_Begin
; CHECK:         if0.T
; CHECK:         @If_Begin
; CHECK:             if1.T
; CHECK:         @If_Else
; CHECK:             if.shared
; CHECK:             @Loop_Begin
; CHECK:                 loop0
; CHECK:                 @If_Begin
; CHECK:                     dx.LoopExitHelper
; CHECK:                     @Loop_Break
; CHECK:                 @If_Else
; CHECK:                     loop0.latch
; CHECK:                     dx.LoopContinue
; CHECK:                     @Loop_Continue
; CHECK:                 @If_End
; CHECK:                 dx.LoopLatch
; CHECK:             @Loop_End
; CHECK:             dx.LoopExit
; CHECK:             loop0.exit
; CHECK:         @If_End
; CHECK:         dx.EndIfScope.1
; CHECK:     @If_Else
; CHECK:         if0.F
; CHECK:         @If_Begin
; CHECK:             if.shared.3
; CHECK:             @Loop_Begin
; CHECK:                 loop0.6
; CHECK:                 @If_Begin
; CHECK:                     dx.LoopExitHelper.7
; CHECK:                     @Loop_Break
; CHECK:                 @If_Else
; CHECK:                     loop0.latch.9
; CHECK:                     dx.LoopContinue.10
; CHECK:                     @Loop_Continue
; CHECK:                 @If_End
; CHECK:                 dx.LoopLatch.11
; CHECK:             @Loop_End
; CHECK:             dx.LoopExit.8
; CHECK:             loop0.exit.14
; CHECK:         @If_Else
; CHECK:             if2.F
; CHECK:         @If_End
; CHECK:         dx.EndIfScope.2
; CHECK:     @If_End
; CHECK:     dx.EndIfScope
; CHECK:     if0.end
; CHECK: @TopLevel_End

define i32 @main(i1 %c1, i1 %c2, i1 %c3, i32 %i1, i32 %i2) {
entry:
  %res = alloca i32
  store i32 1, i32 *%res
  br i1 %c1, label %if0.T, label %if0.F

if0.T:
  store i32 2, i32 *%res
  br i1 %c2, label %if1.T, label %if.shared

if1.T:
  store i32 5, i32 *%res
  br label %if0.end

if.shared:
  %0 = load i32, i32 *%res
  %1 = add i32 %0, 2
  store i32 %1, i32 *%res
  br label %loop0

loop0:
  %loop0.0 = load i32, i32 *%res
  %loop0.1 = icmp eq i32 %loop0.0, 20
  br i1 %loop0.1, label %loop0.exit, label %loop0.latch

loop0.latch:
  store i32 %i2, i32 *%res
  br label %loop0

loop0.exit:
  %loop0.2 = load i32, i32 *%res
  %loop0.3 = add i32 %loop0.2, 2
  store i32 %loop0.3, i32 *%res
  br label %if0.end

if0.F:
  store i32 3, i32 *%res
  br i1 %c3, label %if.shared, label %if2.F

if2.F:
  store i32 10, i32 *%res
  br label %if0.end

if0.end:
  %r = load i32, i32 *%res
  ret i32 %r
}

