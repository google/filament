; RUN: %dxilver 1.8 | %dxv %s | FileCheck %s

; This tests that the validator fails when it finds the 
; wavesize metadata tag as opposed to the wavesize range metadata tag,
; when the shader model version is 6.8
target datalayout = "e-m:e-p:32:32-i1:32-i8:32-i16:32-i32:32-i64:64-f16:32-f32:32-f64:64-n8:16:32:64"
target triple = "dxil-ms-dx"

define void @node01() {
  ret void
}

!llvm.ident = !{!0}
!dx.version = !{!1}
!dx.valver = !{!1}
!dx.shaderModel = !{!2}
!dx.typeAnnotations = !{!3}
!dx.entryPoints = !{!7, !8}

!0 = !{!"dxc(private) 1.7.0.4390 (dxil_validation_on_sv_value_node_launch, 6a52940e2258)"}
!1 = !{i32 1, i32 8}
!2 = !{!"lib", i32 6, i32 8}
!3 = !{i32 1, void ()* @node01, !4}
!4 = !{!5}
!5 = !{i32 0, !6, !6}
!6 = !{}
!7 = !{null, !"", null, null, null}
!8 = !{void ()* @node01, !"node01", null, null, !9}
!9 = !{i32 8, i32 15, i32 13, i32 1, i32 11, !10, i32 15, !11, i32 16, i32 -1, i32 22, !12, i32 20, !13, i32 4, !17, i32 5, !18}

; even with a valid wavesize argument, we do not expect this metadata tag (tag i32 11 -> kDxilWaveSizeTag) 
; in any dxil for validator version 1.8+
; CHECK: error: WaveSize is valid only for Shader Model 6.6 and 6.7.
!10 = !{i32 16}
!11 = !{!"node01", i32 0}
!12 = !{i32 32, i32 1, i32 1}
!13 = !{!14}
!14 = !{i32 1, i32 97, i32 2, !15}
!15 = !{i32 0, i32 12, i32 1, !16}
!16 = !{i32 0, i32 5, i32 1}
!17 = !{i32 1, i32 1, i32 1}
!18 = !{i32 0}
