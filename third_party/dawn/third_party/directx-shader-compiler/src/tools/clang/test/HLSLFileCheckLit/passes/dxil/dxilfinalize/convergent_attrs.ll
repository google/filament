; RUN: opt -hlsl-dxil-module-init -hlsl-dxilfinalize %s -S | FileCheck %s

; Convergence is an internal optimization constraint and should not appear in
; final DXIL on functions or call sites.

; CHECK-LABEL: define void @main() {
; CHECK: %deriv = call float @dx.op.unary.f32(i32 83, float 1.000000e+00){{$}}
; CHECK: %result = call float @other(float %deriv){{$}}
; CHECK: declare float @other(float){{$}}
; CHECK-NOT: convergent

target datalayout = "e-m:e-p:32:32-i1:32-i8:32-i16:32-i32:32-i64:64-f16:32-f32:32-f64:64-n8:16:32:64"
target triple = "dxil-ms-dx"

define void @main() #1 {
  %deriv = call float @dx.op.unary.f32(i32 83, float 1.000000e+00) #1
  %result = call float @other(float %deriv) #1
  call void @dx.op.storeOutput.f32(i32 5, i32 0, i32 0, i8 0, float %result)
  ret void
}

declare float @dx.op.unary.f32(i32, float) #0
declare void @dx.op.storeOutput.f32(i32, i32, i32, i8, float) #0
declare float @other(float) #1

attributes #0 = { nounwind readnone }
attributes #1 = { convergent }

!llvm.ident = !{!0}
!dx.version = !{!1}
!dx.valver = !{!2}
!dx.shaderModel = !{!3}
!dx.viewIdState = !{!4}
!dx.entryPoints = !{!5}

!0 = !{!"dxc attribute finalization test"}
!1 = !{i32 1, i32 0}
!2 = !{i32 1, i32 7}
!3 = !{!"ps", i32 6, i32 0}
!4 = !{[2 x i32] [i32 0, i32 1]}
!5 = !{void ()* @main, !"main", !6, null, null}
!6 = !{null, !7, null}
!7 = !{!8}
!8 = !{i32 0, !"SV_Target", i8 9, i8 16, !9, i8 0, i32 1, i8 1, i32 0, i8 0, !10}
!9 = !{i32 0}
!10 = !{i32 3, i32 1}
