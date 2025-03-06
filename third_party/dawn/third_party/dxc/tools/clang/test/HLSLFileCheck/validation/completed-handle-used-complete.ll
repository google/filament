; RUN: %dxilver 1.8 | %dxv %s | FileCheck %s

; The DXIL was modified from:
; dxc verify_output_complete.hlsl -T lib_6_8

target datalayout = "e-m:e-p:32:32-i1:32-i8:32-i16:32-i32:32-i64:64-f16:32-f32:32-f64:64-n8:16:32:64"
target triple = "dxil-ms-dx"

%dx.types.NodeHandle = type { i8* }
%dx.types.NodeInfo = type { i32, i32 }
%dx.types.NodeRecordHandle = type { i8* }
%dx.types.NodeRecordInfo = type { i32, i32 }
%struct.loadStressRecord.0 = type { [3 x i32], [3 x i32] }

define void @loadStress_16() {
  %1 = call i32 @dx.op.flattenedThreadIdInGroup.i32(i32 96)
  %2 = call %dx.types.NodeHandle @dx.op.createNodeOutputHandle(i32 247, i32 0)
  %3 = call %dx.types.NodeHandle @dx.op.annotateNodeHandle(i32 249, %dx.types.NodeHandle %2, %dx.types.NodeInfo { i32 6, i32 24 })
  %4 = call %dx.types.NodeRecordHandle @dx.op.allocateNodeOutputRecords(i32 238, %dx.types.NodeHandle %3, i32 1, i1 true)
  %5 = call %dx.types.NodeRecordHandle @dx.op.annotateNodeRecordHandle(i32 251, %dx.types.NodeRecordHandle %4, %dx.types.NodeRecordInfo { i32 38, i32 24 })
  %6 = urem i32 %1, 3
  call void @dx.op.outputComplete(i32 241, %dx.types.NodeRecordHandle %5) ; invalidated  
  
  %7 = icmp eq i32 %6, 0
  br i1 %7, label %11, label %8

; <label>:8                                      ; preds = %0
  %9 = call %struct.loadStressRecord.0 addrspace(6)* @dx.op.getNodeRecordPtr.struct.loadStressRecord.0(i32 239, %dx.types.NodeRecordHandle zeroinitializer, i32 0)  ; GetNodeRecordPtr(recordhandle,arrayIndex)
  %10 = getelementptr %struct.loadStressRecord.0, %struct.loadStressRecord.0 addrspace(6)* %9, i32 0, i32 1, i32 0
  store i32 0, i32 addrspace(6)* %10, align 4

  ; test duplicate output complete in diff block
  ; CHECK: error: Invalid use of completed record handle.
  ; CHECK: call void @dx.op.outputComplete(i32 241, %dx.types.NodeRecordHandle %5)
  ; CHECK: note: record handle invalidated by OutputComplete
  ; CHECK: note: at 'call void @dx.op.outputComplete(i32 241, %dx.types.NodeRecordHandle %5)
  call void @dx.op.outputComplete(i32 241, %dx.types.NodeRecordHandle %5) ; invalidated  

  br label %11

; <label>:11                                      ; preds = %8, %0
  ret void
}

; Function Attrs: nounwind readnone
declare i32 @dx.op.flattenedThreadIdInGroup.i32(i32) #0

; Function Attrs: nounwind readnone
declare %struct.loadStressRecord.0 addrspace(6)* @dx.op.getNodeRecordPtr.struct.loadStressRecord.0(i32, %dx.types.NodeRecordHandle, i32) #0

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
!dx.entryPoints = !{!7, !8}

!0 = !{!"dxc(private) 1.7.0.4790 (work-graphs, 35d890870)"}
!1 = !{i32 1, i32 8}
!2 = !{!"lib", i32 6, i32 8}
!3 = !{i32 1, void ()* @loadStress_16, !4}
!4 = !{!5}
!5 = !{i32 0, !6, !6}
!6 = !{}
!7 = !{null, !"", null, null, null}
!8 = !{void ()* @loadStress_16, !"loadStress_16", null, null, !9}
!9 = !{i32 8, i32 15, i32 13, i32 1, i32 15, !10, i32 16, i32 -1, i32 22, !11, i32 20, !12, i32 21, !14, i32 4, !19, i32 5, !20}
!10 = !{!"loadStress_16", i32 0}
!11 = !{i32 3, i32 1, i32 1}
!12 = !{!13}
!13 = !{i32 1, i32 9}
!14 = !{!15}
!15 = !{i32 1, i32 6, i32 2, !16, i32 3, i32 0, i32 0, !18}
!16 = !{i32 0, i32 24, i32 1, !17}
!17 = !{i32 0, i32 5, i32 3}
!18 = !{!"loadStressChild", i32 0}
!19 = !{i32 1, i32 1, i32 1}
!20 = !{i32 0}
!21 = !{!22, !22, i64 0}
!22 = !{!"int", !23, i64 0}
!23 = !{!"omnipotent char", !24, i64 0}
!24 = !{!"Simple C/C++ TBAA"}
