; RUN: %opt-exe %s -scopenested -scopenestinfo -analyze -S | %FileCheck %s
; CHECK: ScopeNestInfo:
; CHECK: @TopLevel_Begin
; CHECK:     entry
; CHECK:     if0
; CHECK:     @If_Begin
; CHECK:         if0.T
; CHECK:         @Loop_Begin
; CHECK:             loop0
; CHECK:             loop0.latch
; CHECK:             dx.LoopContinue
; CHECK:             @Loop_Continue
; CHECK:             dx.LoopLatch
; CHECK:         @Loop_End
; CHECK:     @If_Else
; CHECK:         if0.F
; CHECK:         if0.exit
; CHECK:     @If_End
; CHECK: @TopLevel_End

define i32 @main(i1 %c1, i1 %c2, i1 %c3, i32 %i1, i32 %i2) {
entry:
  %res = alloca i32
  store i32 %i1, i32 *%res
  br label %if0

if0:
  br i1 %c1, label %if0.T, label %if0.F

if0.T:
  br label %loop0

loop0:
  %0 = load i32, i32 *%res
  %1 = icmp eq i32 %0, 20
  br label %loop0.latch

loop0.latch:
  store i32 %i2, i32 *%res
  br label %loop0

if0.F:
  store i32 22, i32 *%res
  br label %if0.exit

if0.exit:
  %2 = load i32, i32 *%res
  ret i32 %2
}
