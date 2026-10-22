; RUN: %dxilver 1.8 | %dxv %s | FileCheck %s

; This tests the validator on emitting errors for invalid
; arguments in the wavesize range attribute for entry point functions.
; This test tests that when the Minimum wavesize value is equal to the Maximum
; wavesize value, a diagnostic is emitted.

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
!dx.entryPoints = !{!7}

!0 = !{!"dxc(private) 1.7.0.4390 (dxil_validation_on_sv_value_node_launch, 6a52940e2258)"}
!1 = !{i32 1, i32 8}
!2 = !{!"lib", i32 6, i32 6}
!3 = !{i32 1, void ()* @node01, !4}
!4 = !{!5}
!5 = !{i32 0, !6, !6}
!6 = !{}
!7 = !{void ()* @node01, !"node01", null, null, !8}
!8 = !{i32 8, i32 15, i32 13, i32 1, i32 11, !9, i32 11, !9, i32 15, !10, i32 16, i32 -1, i32 22, !11, i32 20, !12, i32 4, !16, i32 5, !17}
; CHECK: error: WaveSize or WaveSizeRange tag may only appear once per entry point.
!9 = !{i32 4}
!10 = !{!"node01", i32 0}
!11 = !{i32 32, i32 1, i32 1}
!12 = !{!13}
!13 = !{i32 1, i32 97, i32 2, !14}
!14 = !{i32 0, i32 12, i32 1, !15}
!15 = !{i32 0, i32 5, i32 1}
!16 = !{i32 1, i32 1, i32 1}
!17 = !{i32 0}