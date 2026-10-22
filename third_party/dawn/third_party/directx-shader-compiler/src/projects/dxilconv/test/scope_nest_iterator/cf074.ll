; RUN: %opt-exe %s -simplifycfg -scopenested -scopenestinfo -analyze -S | %FileCheck %s
; CHECK: ScopeNestInfo:
; CHECK: @TopLevel_Begin
; CHECK:     entry
; CHECK:     @If_Begin
; CHECK:         switch0.case0
; CHECK:         @If_Begin
; CHECK:             if0.T
; CHECK:         @If_Else
; CHECK:             if0.F
; CHECK:         @If_End
; CHECK:         dx.EndIfScope.1
; CHECK:     @If_Else
; CHECK:         switch0.default
; CHECK:     @If_End
; CHECK:     dx.EndIfScope
; CHECK:     switch0.end
; CHECK: @TopLevel_End

define i32 @main(i1 %c1, i1 %c2, i1 %c3, i32 %i1, i32 %i2) {
entry:
  %res = alloca i32
  store i32 1, i32 *%res
  switch i32 %i1, label %switch0.default [ i32 0, label %switch0.case0 ]

switch0.case0:
  store i32 0, i32 *%res
  br i1 %c1, label %if0.T, label %if0.F

if0.T:
  store i32 2, i32 *%res
  br label %if0.end
  
if0.F:
  store i32 3, i32 *%res
  br label %switch0.end

if0.end:
  br i1 %c1, label %if1.T, label %if1.F

if1.T:
  store i32 11, i32 *%res
  br label %if1.end
  
if1.F:
  store i32 12, i32 *%res
  br label %if1.end

if1.end:
  br label %switch0.end

switch0.default:
  store i32 -1, i32 *%res
  br label %switch0.end

switch0.end:
  %ret.0 = load i32, i32 *%res
  ret i32 %ret.0
}

