; RUN: %opt-exe %s -reg2mem -scopenested -scopenestinfo -analyze -S | %FileCheck %s

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
  %0 = load i32, i32 *%res
  store i32 2, i32 *%res
  br label %exit

if0.F:
  %1 = load i32, i32 *%res
  store i32 3, i32 *%res
  br label %exit

exit:
  %2 = phi i32 [%0, %if0.T], [%1, %if0.F]
  %3 = load i32, i32 *%res
  %4 = add i32 %2, %3
  ret i32 %4
}

