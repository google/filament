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
; CHECK:         switch0.case1
; CHECK:         dx.SwitchBreak.2
; CHECK:         @Switch_Break
; CHECK:         @Switch_Break
; CHECK:     @Switch_Case
; CHECK:         switch0.case2
; CHECK:         dx.SwitchBreak.3
; CHECK:         @Switch_Break
; CHECK:         @Switch_Break
; CHECK:     @Switch_End
; CHECK:     dx.EndSwitchScope
; CHECK:     switch0.end
; CHECK:     @Switch_Begin
; CHECK:     @Switch_Case
; CHECK:         switch1.default
; CHECK:         dx.SwitchBreak.5
; CHECK:         @Switch_Break
; CHECK:         @Switch_Break
; CHECK:     @Switch_Case
; CHECK:         switch1.case0
; CHECK:         dx.SwitchBreak.6
; CHECK:         @Switch_Break
; CHECK:         @Switch_Break
; CHECK:     @Switch_Case
; CHECK:         switch1.case1
; CHECK:         dx.SwitchBreak.7
; CHECK:         @Switch_Break
; CHECK:         @Switch_Break
; CHECK:     @Switch_End
; CHECK:     dx.EndSwitchScope.4
; CHECK:     switch1.end
; CHECK: @TopLevel_End

define i32 @main(i1 %c1, i1 %c2, i1 %c3, i32 %i1, i32 %i2) {
entry:
  %res = alloca i32
  store i32 1, i32 *%res
  switch i32 %i1, label %switch0.default [ i32 0, label %switch0.case0
                                           i32 1, label %switch0.case1
                                           i32 2, label %switch0.case2 ]

switch0.case0:
  store i32 0, i32 *%res
  br label %switch0.end

switch0.case1:
  store i32 1, i32 *%res
  br label %switch0.end

switch0.case2:
  store i32 2, i32 *%res
  br label %switch0.end

switch0.default:
  store i32 -1, i32 *%res
  br label %switch0.end

switch0.end:
  switch i32 %i1, label %switch1.default [ i32 0, label %switch1.case0
                                           i32 1, label %switch1.case1
                                           i32 2, label %switch1.default
                                           i32 77, label %switch1.default
                                           i32 33, label %switch1.case1 ]

switch1.case0:
  store i32 0, i32 *%res
  br label %switch1.end

switch1.case1:
  store i32 1, i32 *%res
  br label %switch1.end

switch1.default:
  store i32 -1, i32 *%res
  br label %switch1.end

switch1.end:
  %0 = load i32, i32 *%res
  ret i32 %0
}

