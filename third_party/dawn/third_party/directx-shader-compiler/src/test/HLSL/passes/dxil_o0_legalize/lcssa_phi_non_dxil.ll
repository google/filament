; RUN: opt -S -dxil-o0-simplify-inst %s | FileCheck %s

; Make sure we remove a single-value phi node.

; CHECK: %add = add i32 1, 2
; CHECK-NOT: phi
; CHECK: %mul = mul i32 %add, 2
target datalayout = "e-m:e-p:32:32-i1:32-i8:32-i16:32-i32:32-i64:64-f16:32-f32:32-f64:64-n8:16:32:64"
target triple = "dxil-ms-dx"

define void @foo() {
entry:
  %add = add i32 1, 2
  br label %exit

exit:
  %lcssa = phi i32 [ %add, %entry ]
  %mul = mul i32 %lcssa, 2
  ret void
}
