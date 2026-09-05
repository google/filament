; RUN: %opt-exe %s -scopenested -scopenestinfo -analyze -S | %FileCheck %s
; CHECK: ScopeNestInfo:
; CHECK: @TopLevel_Begin
; CHECK:     entry
; CHECK:     if0.end
; CHECK: @TopLevel_End

define i32 @main(i1 %c1, i1 %c2, i1 %c3, i32 %i1, i32 %i2) {
entry:
  %res = alloca i32
  store i32 1, i32 *%res
  br i1 %c1, label %if0.end, label %if0.end

if0.end:
  store i32 3, i32 *%res
  %0 = load i32, i32 *%res
  ret i32 %0
}
