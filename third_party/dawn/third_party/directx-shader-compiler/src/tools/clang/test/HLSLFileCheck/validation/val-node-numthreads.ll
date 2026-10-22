; RUN: %dxv %s

; Compiled from: val-node-numthreads.hlsl

; Previously, validation errors would occur after loading metadata, because
; it wouldn't fill numThreads for the node info when shader type was compute.
; Function: compute_node: error: Declared Thread Group X size 0 outside valid range [1..1024].
; Function: compute_node: error: Declared Thread Group Y size 0 outside valid range [1..1024].
; Function: compute_node: error: Declared Thread Group Z size 0 outside valid range [1..64].

target datalayout = "e-m:e-p:32:32-i1:32-i8:32-i16:32-i32:32-i64:64-f16:32-f32:32-f64:64-n8:16:32:64"
target triple = "dxil-ms-dx"

; Function Attrs: nounwind readnone
define void @compute_node() #0 {
  ret void
}

attributes #0 = { nounwind readnone }

!llvm.ident = !{!0}
!dx.version = !{!1}
!dx.valver = !{!1}
!dx.shaderModel = !{!2}
!dx.typeAnnotations = !{!3}
!dx.entryPoints = !{!7, !8}

!0 = !{!"dxc(private) 1.7.0.4775 (user/texr/merge-sm68-core-to-work-graphs, 34c249085-dirty)"}
!1 = !{i32 1, i32 8}
!2 = !{!"lib", i32 6, i32 8}
!3 = !{i32 1, void ()* @compute_node, !4}
!4 = !{!5}
!5 = !{i32 1, !6, !6}
!6 = !{}
!7 = !{null, !"", null, null, null}
!8 = !{void ()* @compute_node, !"compute_node", null, null, !9}
!9 = !{i32 8, i32 5, i32 13, i32 1, i32 15, !10, i32 16, i32 -1, i32 18, !11, i32 20, !12, i32 4, !14, i32 5, !15}
!10 = !{!"compute_node", i32 0}
!11 = !{i32 2, i32 1, i32 1}
!12 = !{!13}
!13 = !{i32 1, i32 9}
!14 = !{i32 9, i32 1, i32 2}
!15 = !{i32 0}
