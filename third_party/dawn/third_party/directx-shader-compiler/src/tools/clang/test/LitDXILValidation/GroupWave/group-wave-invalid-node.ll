; REQUIRES: dxil-1-10
; RUN: not %dxv %s 2>&1 | FileCheck %s

; CHECK:  error: Function requires a visible group, but is called from a shader without one.

target datalayout = "e-m:e-p:32:32-i1:32-i8:32-i16:32-i32:32-i64:64-f16:32-f32:32-f64:64-n8:16:32:64"
target triple = "dxil-ms-dx"

%dx.types.NodeHandle = type { i8* }
%dx.types.NodeInfo = type { i32, i32 }
%dx.types.NodeRecordHandle = type { i8* }
%dx.types.NodeRecordInfo = type { i32, i32 }
%struct.OutputRecord = type { i32 }

define void @ThreadNode() {
  %1 = call %dx.types.NodeHandle @dx.op.createNodeOutputHandle(i32 247, i32 0)  ; CreateNodeOutputHandle(MetadataIdx)
  %2 = call %dx.types.NodeHandle @dx.op.annotateNodeHandle(i32 249, %dx.types.NodeHandle %1, %dx.types.NodeInfo { i32 6, i32 4 })  ; AnnotateNodeHandle(node,props)
  %3 = call i32 @dx.op.getGroupWaveIndex(i32 -2147483647)  ; GetGroupWaveIndex()
  %4 = call i32 @dx.op.getGroupWaveCount(i32 -2147483646)  ; GetGroupWaveCount()
  %5 = call %dx.types.NodeRecordHandle @dx.op.allocateNodeOutputRecords(i32 238, %dx.types.NodeHandle %2, i32 1, i1 true)  ; AllocateNodeOutputRecords(output,numRecords,perThread)
  %6 = call %dx.types.NodeRecordHandle @dx.op.annotateNodeRecordHandle(i32 251, %dx.types.NodeRecordHandle %5, %dx.types.NodeRecordInfo { i32 38, i32 4 })  ; AnnotateNodeRecordHandle(noderecord,props)
  %7 = add i32 %4, %3
  %8 = call %struct.OutputRecord addrspace(6)* @dx.op.getNodeRecordPtr.struct.OutputRecord(i32 239, %dx.types.NodeRecordHandle %6, i32 0)  ; GetNodeRecordPtr(recordhandle,arrayIndex)
  %9 = getelementptr %struct.OutputRecord, %struct.OutputRecord addrspace(6)* %8, i32 0, i32 0
  store i32 %7, i32 addrspace(6)* %9, align 4
  call void @dx.op.outputComplete(i32 241, %dx.types.NodeRecordHandle %6)  ; OutputComplete(output)
  ret void
}

; Function Attrs: nounwind readnone
declare %struct.OutputRecord addrspace(6)* @dx.op.getNodeRecordPtr.struct.OutputRecord(i32, %dx.types.NodeRecordHandle, i32) #0

; Function Attrs: nounwind readnone
declare i32 @dx.op.getGroupWaveCount(i32) #0

; Function Attrs: nounwind readnone
declare i32 @dx.op.getGroupWaveIndex(i32) #0

; Function Attrs: nounwind
declare %dx.types.NodeRecordHandle @dx.op.allocateNodeOutputRecords(i32, %dx.types.NodeHandle, i32, i1) #1

; Function Attrs: nounwind
declare void @dx.op.outputComplete(i32, %dx.types.NodeRecordHandle) #1

; Function Attrs: nounwind readnone
declare %dx.types.NodeRecordHandle @dx.op.annotateNodeRecordHandle(i32, %dx.types.NodeRecordHandle, %dx.types.NodeRecordInfo) #0

; Function Attrs: nounwind readnone
declare %dx.types.NodeHandle @dx.op.createNodeOutputHandle(i32, i32) #0

; Function Attrs: nounwind readnone
declare %dx.types.NodeHandle @dx.op.annotateNodeHandle(i32, %dx.types.NodeHandle, %dx.types.NodeInfo) #0

attributes #0 = { nounwind readnone }
attributes #1 = { nounwind }

!llvm.ident = !{!0}
!dx.version = !{!1}
!dx.valver = !{!1}
!dx.shaderModel = !{!2}
!dx.typeAnnotations = !{!3}
!dx.entryPoints = !{!7, !9}

!0 = !{!"dxc(private) 1.9.0.5169 (Group-Wave-Intrinsics, deeac02f3-dirty)"}
!1 = !{i32 1, i32 10}
!2 = !{!"lib", i32 6, i32 10}
!3 = !{i32 1, void ()* @ThreadNode, !4}
!4 = !{!5}
!5 = !{i32 0, !6, !6}
!6 = !{}
!7 = !{null, !"", null, null, !8}
!8 = !{i32 0, i64 524288}
!9 = !{void ()* @ThreadNode, !"ThreadNode", null, null, !10}
!10 = !{i32 8, i32 15, i32 13, i32 3, i32 15, !11, i32 16, i32 -1, i32 20, !12, i32 21, !15, i32 4, !18, i32 5, !19}
!11 = !{!"ThreadNode", i32 0}
!12 = !{!13}
!13 = !{i32 1, i32 37, i32 2, !14}
!14 = !{i32 0, i32 4, i32 2, i32 4}
!15 = !{!16}
!16 = !{i32 1, i32 6, i32 2, !14, i32 3, i32 1, i32 0, !17}
!17 = !{!"outputData", i32 0}
!18 = !{i32 1, i32 1, i32 1}
!19 = !{i32 0}