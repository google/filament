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
; CHECK:         dx.SwitchBreak.1
; CHECK:         @Switch_Break
; CHECK:         @Switch_Break
; CHECK:     @Switch_Case
; CHECK:         switch0.case3
; CHECK:         dx.SwitchBreak.2
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
                                           i32 1, label %switch0.default
                                           i32 2, label %switch0.case0
                                           i32 3, label %switch0.case3 ]

switch0.case0:
  store i32 0, i32 *%res
  br label %switch0.end

switch0.case3:
  store i32 3, i32 *%res
  br label %switch0.end

switch0.default:
  store i32 -1, i32 *%res
  br label %switch0.end

switch0.end:
  %0 = load i32, i32 *%res
  ret i32 %0
}

