; RUN: %opt-exe %s -scopenested -scopenestinfo -analyze -S | %FileCheck %s

; CHECK: ScopeNestInfo:
; CHECK: @TopLevel_Begin
; CHECK:     entry
; CHECK:     @If_Begin
; CHECK:         if0.T
; CHECK:         @If_Begin
; CHECK:             if1.T
; CHECK:         @If_Else
; CHECK:             if1.F
; CHECK:         @If_End
; CHECK:     @If_Else
; CHECK:         if0.F
; CHECK:         @If_Begin
; CHECK:             if2.T
; CHECK:         @If_Else
; CHECK:             if2.F
; CHECK:         @If_End
; CHECK:         dx.EndIfScope
; CHECK:         if2.end
; CHECK:         if0.end
; CHECK:     @If_End
; CHECK: @TopLevel_End

define i32 @main(i1 %c1, i1 %c2, i1 %c3, i32 %i1, i32 %i2) {
entry:
  %res = alloca i32
  store i32 1, i32 *%res
  br i1 %c1, label %if0.T, label %if0.F

if0.T:
  store i32 2, i32 *%res
  br i1 %c2, label %if1.T, label %if1.F

if1.T:
  store i32 5, i32 *%res
  ret i32 %i1

if1.F:
  store i32 6, i32 *%res
  ret i32 1

if0.F:
  store i32 3, i32 *%res
  br i1 %c3, label %if2.T, label %if2.F

if2.T:
  store i32 9, i32 *%res
  br label %if2.end

if2.F:
  store i32 10, i32 *%res
  br label %if2.end

if2.end:
  store i32 11, i32 *%res
  br label %if0.end

if0.end:
  %0 = load i32, i32 *%res
  ret i32 %0
}

