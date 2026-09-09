; RUN: not %dxv %s 2>&1 | FileCheck %s

target datalayout = "e-m:e-p:32:32-i1:32-i8:32-i16:32-i32:32-i64:64-f16:32-f32:32-f64:64-n8:16:32:64"
target triple = "dxil-ms-dx"

; CHECK: Int type 'i25' has an invalid width.
; CHECK: Int type 'i25' has an invalid width.
; CHECK: Validation failed.

define void @main() {
  ; Test invalid int width in operand and result types.
  %1 = add i25 0, 1
  ; Test invalid int width inside array result type.
  %2 = select i1 true, [2 x i25] zeroinitializer, [2 x i25] zeroinitializer
  call void @dx.op.storeOutput.f32(i32 5, i32 0, i32 0, i8 0, float 0.000000e+00)
  ret void
}

declare void @dx.op.storeOutput.f32(i32, i32, i32, i8, float) #0

attributes #0 = { nounwind }

!dx.version = !{!0}
!dx.valver = !{!0}
!dx.shaderModel = !{!1}
!dx.entryPoints = !{!2}

!0 = !{i32 1, i32 0}
!1 = !{!"ps", i32 6, i32 0}
!2 = !{void ()* @main, !"main", !3, null, null}
!3 = !{null, !4, null}
!4 = !{!5}
!5 = !{i32 0, !"SV_Target", i8 9, i8 16, !6, i8 0, i32 1, i8 1, i32 0, i8 0, !7}
!6 = !{i32 0}
!7 = !{i32 3, i32 1}
