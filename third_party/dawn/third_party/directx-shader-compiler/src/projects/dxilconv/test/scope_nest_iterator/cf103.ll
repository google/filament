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
; CHECK:         @Switch_Break
; CHECK:     @Switch_Case
; CHECK:         switch0.case1
; CHECK:         dx.SwitchBreak.1
; CHECK:         @Switch_Break
; CHECK:         @Switch_Break
; CHECK:     @Switch_Case
; CHECK:         switch0.case2
; CHECK:         @Switch_Begin
; CHECK:         @Switch_Case
; CHECK:             switch1.default
; CHECK:             @Switch_Break
; CHECK:         @Switch_Case
; CHECK:             switch1.case0
; CHECK:             @Switch_Break
; CHECK:         @Switch_Case
; CHECK:             switch1.case1
; CHECK:             @Switch_Break
; CHECK:         @Switch_Case
; CHECK:             switch1.case2
; CHECK:             @Switch_Break
; CHECK:         @Switch_End
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
  ret i32 %i1

switch0.case1:
  store i32 1, i32 *%res
  br label %switch0.end

switch0.case2:
  store i32 2, i32 *%res

;; -------------------
  switch i32 %i1, label %switch1.default [ i32 0, label %switch1.case0
                                           i32 1, label %switch1.case1
                                           i32 11, label %switch1.case1
                                           i32 12, label %switch1.case2
                                           i32 2, label %switch1.case2 ]

switch1.case0:
  store i32 898, i32 *%res
  ret i32 85

switch1.case1:
  ret i32 86

switch1.case2:
  ret i32 88

switch1.default:
  ret i32 90

;switch1.end:
;  br label %switch0.end
;; ---------------------

switch0.default:
  store i32 -1, i32 *%res
  br label %switch0.end

switch0.end:
  %switch0.0 = load i32, i32 *%res
  %switch0.1 = add i32 %switch0.0, 1
  store i32 %switch0.1, i32 *%res
  %0 = load i32, i32 *%res
  ret i32 %0
}

