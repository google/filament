; RUN: %opt-exe %s -scopenested -scopenestinfo -analyze -S | %FileCheck %s
; CHECK: ScopeNestInfo:
; CHECK: @TopLevel_Begin
; CHECK:     entry
; CHECK:     @Switch_Begin
; CHECK:     @Switch_Case
; CHECK:         switch0.default
; CHECK:         dx.SwitchBreak
; CHECK:         @Switch_Break
; CHECK:         @Switch_Break
; CHECK:     @Switch_Case
; CHECK:         switch0.case0
; CHECK:         dx.SwitchBreak.2
; CHECK:         @Switch_Break
; CHECK:         @Switch_Break
; CHECK:     @Switch_Case
; CHECK:         switch0.case1
; CHECK:         @Loop_Begin
; CHECK:             loop0
; CHECK:             @If_Begin
; CHECK:                 dx.LoopExitHelper
; CHECK:                 @Loop_Break
; CHECK:             @If_Else
; CHECK:                 loop0.body0
; CHECK:                 @If_Begin
; CHECK:                     dx.LoopExitHelper.1
; CHECK:                     @Loop_Break
; CHECK:                 @If_Else
; CHECK:                     loop0.latch
; CHECK:                     dx.LoopContinue
; CHECK:                     @Loop_Continue
; CHECK:                 @If_End
; CHECK:             @If_End
; CHECK:             dx.LoopLatch
; CHECK:         @Loop_End
; CHECK:         dx.LoopExit
; CHECK:         loop0.exit
; CHECK:         dx.SwitchBreak.3
; CHECK:         @Switch_Break
; CHECK:         @Switch_Break
; CHECK:     @Switch_Case
; CHECK:         switch0.case2
; CHECK:         dx.SwitchBreak.4
; CHECK:         @Switch_Break
; CHECK:         @Switch_Break
; CHECK:     @Switch_End
; CHECK:     dx.EndSwitchScope
; CHECK:     switch0.end
; CHECK: @TopLevel_End

define i32 @main(i1 %c1, i1 %c2, i1 %c3, i32 %i1, i32 %i2) {
entry:
  %res = alloca i32
  store i32 1, i32 *%res
  switch i32 %i1, label %switch0.default [ i32 0, label %switch0.case0
                                           i32 1, label %switch0.case1
                                           i32 11, label %switch0.case1
                                           i32 12, label %switch0.case2
                                           i32 2, label %switch0.case2 ]

switch0.case0:
  store i32 0, i32 *%res
  br label %switch0.end

switch0.case1:
  br label %loop0

loop0:
  %loop0.0 = load i32, i32 *%res
  %loop0.1 = icmp eq i32 %loop0.0, 20
  br i1 %loop0.1, label %loop0.exit, label %loop0.body0

loop0.body0:
  %cond2 = icmp eq i32 %i2, 10
  br i1 %cond2, label %loop0.exit, label %loop0.latch

loop0.latch:
  store i32 %i2, i32 *%res
  br label %loop0

loop0.exit:
  %loop0.2 = load i32, i32 *%res
  %loop0.3 = add i32 %loop0.2, 1
  store i32 %loop0.3, i32 *%res
  br label %switch0.end

switch0.case2:
  store i32 2, i32 *%res
  br label %switch0.end

switch0.default:
  store i32 -1, i32 *%res
  br label %switch0.end

switch0.end:
  %switch0.0 = load i32, i32 *%res
  %switch0.1 = add i32 %switch0.0, 1
  store i32 %switch0.1, i32 *%res
  ret i32 %switch0.1
}

