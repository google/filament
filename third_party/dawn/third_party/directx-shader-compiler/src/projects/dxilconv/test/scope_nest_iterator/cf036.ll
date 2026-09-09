; RUN: %opt-exe %s -simplifycfg -loop-simplify -reg2mem -scopenested -scopenestinfo -analyze -S | %FileCheck %s
; CHECK: ScopeNestInfo:
; CHECK: @TopLevel_Begin
; CHECK:     dx.bb.0
; CHECK:     @Loop_Begin
; CHECK:         loop0
; CHECK:         @If_Begin
; CHECK:             body0
; CHECK:             dx.LoopContinue
; CHECK:             @Loop_Continue
; CHECK:         @If_Else
; CHECK:             body1
; CHECK:             dx.LoopContinue.1
; CHECK:             @Loop_Continue
; CHECK:         @If_End
; CHECK:         loop0.backedge
; CHECK:     @Loop_End
; CHECK: @TopLevel_End

define i32 @main(i1 %c1, i1 %c2, i1 %c3, i32 %i1, i32 %i2) {
dx.bb.0:
  %res = alloca i32
  store i32 %i1, i32 *%res
  br label %loop0

loop0:
  %0 = load i32, i32 *%res
  %1 = icmp eq i32 %0, 20
  br i1 %1, label %body0, label %body1

body0:
  %v2 = load i32, i32 *%res
  %2 = add i32 %v2, 77
  store i32 %2, i32 *%res
  br label %loop0

body1:
  %v3 = load i32, i32 *%res
  %3 = add i32 %v3, 33
  store i32 %3, i32 *%res
  br label %loop0
}
