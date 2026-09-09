; RUN: %opt %s -hlsl-passes-resume -dce -S | FileCheck %s


target datalayout = "e-m:e-p:32:32-i1:32-i8:32-i16:32-i32:32-i64:64-f16:32-f32:32-f64:64-n8:16:32:64"
target triple = "dxil-ms-dx"

%dx.types.NodeHandle = type { i8* }
%dx.types.NodeInfo = type { i32, i32 }
%dx.types.NodeRecordHandle = type { i8* }
%dx.types.NodeRecordInfo = type { i32, i32 }
%struct.loadStressRecord.0 = type { [3 x i32], [3 x i32] }

@"\01?loadStressTemp@@3PAIA" = external addrspace(3) global [128 x i32], align 4

define void @loadStress_16() {
  %1 = call %dx.types.NodeHandle @dx.op.createNodeOutputHandle(i32 247, i32 0)  ; CreateNodeOutputHandle(MetadataIdx)
  %2 = call %dx.types.NodeHandle @dx.op.annotateNodeHandle(i32 249, %dx.types.NodeHandle %1, %dx.types.NodeInfo { i32 6, i32 24 })  ; AnnotateNodeHandle(node,props)
  %3 = call %dx.types.NodeRecordHandle @dx.op.allocateNodeOutputRecords(i32 238, %dx.types.NodeHandle %2, i32 1, i1 true)  ; AllocateNodeOutputRecords(output,numRecords,perThread)
  %4 = call %dx.types.NodeRecordHandle @dx.op.annotateNodeRecordHandle(i32 251, %dx.types.NodeRecordHandle %3, %dx.types.NodeRecordInfo { i32 38, i32 24 })  ; AnnotateNodeRecordHandle(noderecord,props)
  %5 = call %struct.loadStressRecord.0 addrspace(6)* @dx.op.getNodeRecordPtr.struct.loadStressRecord.0(i32 239, %dx.types.NodeRecordHandle %4, i32 0)  ; GetNodeRecordPtr(recordhandle,arrayIndex)
  %6 = getelementptr inbounds %struct.loadStressRecord.0, %struct.loadStressRecord.0 addrspace(6)* %5, i32 0, i32 0, i32 0
  store i32 3, i32 addrspace(6)* %6, align 4
  
  call void @dx.op.outputComplete(i32 241, %dx.types.NodeRecordHandle %4)  ; OutputComplete(output)
  
  ; test that the 2 subsequent output complete calls are removed by DCE
  ; CHECK: call void @dx.op.outputComplete(i32 241, %dx.types.NodeRecordHandle %4)
  ; CHECK-NEXT: ret void
  call void @dx.op.outputComplete(i32 241, %dx.types.NodeRecordHandle zeroinitializer)  ; OutputComplete(output)
  call void @dx.op.outputComplete(i32 241, %dx.types.NodeRecordHandle zeroinitializer)  ; OutputComplete(output)
  ret void
}

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
