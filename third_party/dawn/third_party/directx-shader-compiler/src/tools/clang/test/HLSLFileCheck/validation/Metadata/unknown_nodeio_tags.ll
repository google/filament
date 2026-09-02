; RUN: %dxilver 1.8 | %dxv %s | FileCheck %s

; Test that appropriate errors are produced for unknown node IO Tags

; CHECK: error: Metadata error encountered in non-critical metadata (such as Type Annotations).

target datalayout = "e-m:e-p:32:32-i1:32-i8:32-i16:32-i32:32-i64:64-f16:32-f32:32-f64:64-n8:16:32:64"
target triple = "dxil-ms-dx"

define void @main() {
  ret void
}

!llvm.ident = !{!0}
!dx.version = !{!1}
!dx.valver = !{!1}
!dx.shaderModel = !{!2}
!dx.typeAnnotations = !{!3}
!dx.entryPoints = !{!7, !8}

!0 = !{!"dxc(private) 1.7.0.4023 (update-dndxc, 15d086f68-dirty)"}
!1 = !{i32 1, i32 8}
!2 = !{!"lib", i32 6, i32 8}
!3 = !{i32 1, void ()* @main, !4}
!4 = !{!5}
!5 = !{i32 0, !6, !6}
!6 = !{}
!7 = !{null, !"", null, null, null}
!8 = !{void ()* @main, !"main", null, null, !9}
!9 = !{i32 8, i32 15, i32 13, i32 1, i32 14, i1 true, i32 15, !10, i32 16, i32 -1, i32 18, !11, i32 20, !12, i32 21, !15, i32 4, !20, i32 5, !21}
!10 = !{!"main", i32 0}
!11 = !{i32 2, i32 3, i32 2}
!12 = !{!13}
; Arg #1: NodeIOFlags Tag (1)
; Arg #2: DispatchNodeInputRecord (97)
; Arg #3: INVALID Tag (7)
; Arg #4: RECORD type
!13 = !{i32 1, i32 97, i32 7, !14}
!14 = !{i32 0, i32 4}
!15 = !{!16, !18}
; Arg #1: NodeIOFlags Tag (1)
; Arg #2: NodeOutput (6)
; Arg #3: INVALID Tag (66)
; Arg #4: Type node
; Arg #5: NodeMaxRecords Tag (3)
; Arg #6: value of 7
; Arg #9: NodeOutputID Tag (0)
; Arg #10: output node
!16 = !{i32 1, i32 6, i32 66, !14, i32 333, i32 7, i32 0, !17}
!17 = !{!"myFascinatingNode", i32 0}

; Arg #1: NodeIOFlags Tag (1)
; Arg #2: NodeOutputArray (22)
; Arg #3: INVALID Tag (-1)
; Arg #4: Type node
; Arg #5: NodeMaxRecords Tag (3)
; Arg #6: value 0 (because shared)
; Arg #7: NodeOutputArraySize Tag (5)
; Arg #8: value 63
; Arg #9: NodeMaxRecordsSharedWith Tag (4)
; Arg #10: index 0 (myFascinatingNode)
; Arg #11: NodeAllowSparseNodes Tag (6)
; Arg #12: value true
; Arg #13: NodeOutputID Tag (0)
; Arg #14: output node
!18 = !{i32 1, i32 22, i32 -1, !14, i32 3, i32 0, i32 5, i32 63, i32 4, i32 0, i32 6, i1 true, i32 0, !19}
!19 = !{!"myMaterials", i32 0}
!20 = !{i32 1024, i32 1, i32 1}
!21 = !{i32 0}

