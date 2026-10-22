; RUN: opt -hlsl-dxil-module-init -hlsl-dxilfinalize %s -S | FileCheck %s

; Make sure !dx.temp is removed even for lib targets.

; CHECK-LABEL:define void @main
; CHECK-NOT:!dx.temp

target datalayout = "e-m:e-p:32:32-i1:32-i8:32-i16:32-i32:32-i64:64-f16:32-f32:32-f64:64-n8:16:32:64"
target triple = "dxil-ms-dx"

; Function Attrs: nounwind
define void @main(float* noalias nocapture readnone) #0 {
entry:
  call void @dx.op.storeOutput.f32(i32 5, i32 0, i32 0, i8 0, float 0.000000e+00), !dx.temp !9
  ret void
}

; Function Attrs: nounwind
declare void @dx.op.storeOutput.f32(i32, i32, i32, i8, float) #0

attributes #0 = { nounwind }

!llvm.module.flags = !{!0}
!pauseresume = !{!1}
!llvm.ident = !{!2}
!dx.version = !{!3}
!dx.valver = !{!4}
!dx.shaderModel = !{!5}
!dx.typeAnnotations = !{!6}
!dx.entryPoints = !{!13, !14}

!0 = !{i32 2, !"Debug Info Version", i32 3}
!1 = !{!"hlsl-dxilemit", !"hlsl-dxilload"}
!2 = !{!"dxc(private) 1.7.0.14005 (main, 96e13a0e0-dirty)"}
!3 = !{i32 1, i32 3}
!4 = !{i32 1, i32 7}
!5 = !{!"lib", i32 6, i32 3}
!6 = !{i32 1, void (float*)* @main, !7}
!7 = !{!8, !10}
!8 = !{i32 0, !9, !9}
!9 = !{}
!10 = !{i32 1, !11, !12}
!11 = !{i32 4, !"SV_Target", i32 7, i32 9}
!12 = !{i32 0}
!13 = !{null, !"", null, null, null}
!14 = !{void (float*)* @main, !"main", !15, null, !18}
!15 = !{null, !16, null}
!16 = !{!17}
!17 = !{i32 0, !"SV_Target", i8 9, i8 16, !12, i8 0, i32 1, i8 1, i32 0, i8 0, null}
!18 = !{i32 8, i32 0, i32 5, !12}