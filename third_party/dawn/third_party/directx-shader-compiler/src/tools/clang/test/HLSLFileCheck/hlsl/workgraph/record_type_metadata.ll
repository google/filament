; RUN: %dxilver 1.8 | %D3DReflect %s | FileCheck %s -check-prefixes=RDAT

; Make sure NodeRecordType metdata loading is robust to tag,value list ordering.

; RDAT: FunctionTable[{{.*}}] = {
; RDAT-LABEL: UnmangledName: "node01"
; RDAT: Inputs: <11:RecordArrayRef<IONode>[1]>  = {
; RDAT: AttribKind: RecordSizeInBytes
; RDAT-NEXT: RecordSizeInBytes: 12
; RDAT: AttribKind: RecordDispatchGrid
; RDAT-NEXT: RecordDispatchGrid: <RecordDispatchGrid>
; RDAT-NEXT: ByteOffset: 0
; RDAT-NEXT: ComponentNumAndType: 23
; RDAT-LABEL: RecordTable

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

!0 = !{!"dxc(private) 1.8.0.4454 (rdat-dump-flags, c997ea026-dirty)"}
!1 = !{i32 1, i32 8}
!2 = !{!"lib", i32 6, i32 8}
!3 = !{i32 1, void ()* @node01, !4}
!4 = !{!5}
!5 = !{i32 0, !6, !6}
!6 = !{}
!7 = !{null, !"", null, null, null}
!8 = !{void ()* @node01, !"node01", null, null, !9}
!9 = !{i32 8, i32 15, i32 13, i32 1, i32 15, !10, i32 16, i32 -1, i32 22, !11, i32 20, !12, i32 4, !11, i32 5, !16}
!10 = !{!"node01", i32 0}
!11 = !{i32 4, i32 4, i32 4}
!12 = !{!13}
!13 = !{i32 1, i32 101, i32 2, !14}
!14 = !{i32 1, !15, i32 0, i32 12} ; reordered tag,value list entries for NodeRecordType
!15 = !{i32 0, i32 5, i32 3}
!16 = !{i32 0}
