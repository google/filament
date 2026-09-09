; RUN: %opt-exe %s -scopenested -scopenestinfo -analyze -S | %FileCheck %s

; CHECK: ScopeNestInfo:
; CHECK: @TopLevel_Begin
; CHECK:     entry
; CHECK:     @If_Begin
; CHECK:         if0.T
; CHECK:     @If_Else
; CHECK:         if0.F
; CHECK:     @If_End
; CHECK:     dx.EndIfScope
; CHECK:     exit
; CHECK: @TopLevel_End

define i32 @main(i1 %c1, i1 %c2, i1 %c3, i32 %i1, i32 %i2) {
entry:
  %res = alloca i32
  store i32 1, i32 *%res
  br i1 %c1, label %if0.T, label %if0.F

if0.T:
  store i32 2, i32 *%res
  br label %exit

if0.F:
  store i32 3, i32 *%res
  br label %exit

exit:
  %0 = load i32, i32 *%res
  ret i32 %0
}

