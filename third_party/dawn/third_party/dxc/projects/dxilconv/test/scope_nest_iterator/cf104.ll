; RUN: %opt-exe %s -scopenested -scopenestinfo -analyze -S | %FileCheck %s
; CHECK: ScopeNestInfo:
; CHECK: @TopLevel_Begin
; CHECK:     entry
; CHECK:     @If_Begin
; CHECK:         if0.T
; CHECK:         @Switch_Begin
; CHECK:         @Switch_Case
; CHECK:             switch0.default
; CHECK:             @Switch_Break
; CHECK:         @Switch_Case
; CHECK:             switch0.case0
; CHECK:             @Switch_Break
; CHECK:         @Switch_Case
; CHECK:             switch0.case1
; CHECK:             @Switch_Break
; CHECK:         @Switch_Case
; CHECK:             switch0.case2
; CHECK:             @Switch_Break
; CHECK:         @Switch_End
; CHECK:     @If_Else
; CHECK:         if0.F
; CHECK:         if0.end
; CHECK:     @If_End
; CHECK: @TopLevel_End

define i32 @main(i1 %c1, i1 %c2, i1 %c3, i32 %i1, i32 %i2) {
entry:
  %res = alloca i32
  store i32 1, i32 *%res
  br i1 %c1, label %if0.T, label %if0.F

if0.T:
  switch i32 %i1, label %switch0.default [ i32 0, label %switch0.case0
                                           i32 1, label %switch0.case1
                                           i32 12, label %switch0.case2
                                           i32 42, label %switch0.default
                                           i32 52, label %switch0.default
                                           i32 62, label %switch0.default
                                           i32 11, label %switch0.case1
                                           i32 2, label %switch0.case2 ]

switch0.case0:
  store i32 0, i32 *%res
  ret i32 0

switch0.case1:
  store i32 1, i32 *%res
  ret i32 1

switch0.case2:
  store i32 2, i32 *%res
  ret i32 2

switch0.default:
  store i32 -1, i32 *%res
  ret i32 -1

if0.F:
  store i32 21, i32 *%res
  br label %if0.end

if0.end:
  %0 = load i32, i32 *%res
  ret i32 %0
}

