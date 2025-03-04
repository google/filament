; RUN: %dxilver 1.8 | %dxv %s | FileCheck %s
; 

;
; Note: shader requires additional functionality:
;       UAVs at every shader stage
;
; shader hash: 27900d88c63cf1e406c339e543e4ebc9
;
; Buffer Definitions:
;
;
; Resource Bindings:
;
; Name                                 Type  Format         Dim      ID      HLSL Bind  Count
; ------------------------------ ---------- ------- ----------- ------- -------------- ------
; buf0                                  UAV     u32         buf      U0u4294967295,space4294967295     1
;
target datalayout = "e-m:e-p:32:32-i1:32-i8:32-i16:32-i32:32-i64:64-f16:32-f32:32-f64:64-n8:16:32:64"
target triple = "dxil-ms-dx"

%dx.types.Handle = type { i8* }
%dx.types.ResourceProperties = type { i32, i32 }
%dx.types.NodeRecordHandle = type { i8* }
%dx.types.NodeRecordInfo = type { i32, i32 }
%"class.RWBuffer<unsigned int>" = type { i32 }

@"\01?buf0@@3V?$RWBuffer@I@@A" = external constant %dx.types.Handle, align 4

define void @node01() {
  %1 = load %dx.types.Handle, %dx.types.Handle* @"\01?buf0@@3V?$RWBuffer@I@@A", align 4
  %2 = call %dx.types.Handle @dx.op.createHandleForLib.dx.types.Handle(i32 160, %dx.types.Handle %1)  ; CreateHandleForLib(Resource)
  %3 = call %dx.types.Handle @dx.op.annotateHandle(i32 216, %dx.types.Handle %2, %dx.types.ResourceProperties { i32 4106, i32 261 })  ; AnnotateHandle(res,props)  resource: RWTypedBuffer<U32>
  ; CHECK: error: Invalid semantic flags on DXIL operation 'barrierByMemoryHandle'
  call void @dx.op.barrierByMemoryHandle(i32 245, %dx.types.Handle %3, i32 9)  ; BarrierByMemoryHandle(object,SemanticFlags)
  ret void
}

define void @node02() {
  %1 = call %dx.types.NodeRecordHandle @dx.op.createNodeInputRecordHandle(i32 250, i32 0)  ; CreateNodeInputRecordHandle(MetadataIdx)
  %2 = call %dx.types.NodeRecordHandle @dx.op.annotateNodeRecordHandle(i32 251, %dx.types.NodeRecordHandle %1, %dx.types.NodeRecordInfo { i32 97, i32 12 })  ; AnnotateNodeRecordHandle(noderecord,props)
  ; CHECK: error: Invalid semantic flags on DXIL operation 'barrierByNodeRecordHandle'
  call void @dx.op.barrierByNodeRecordHandle(i32 246, %dx.types.NodeRecordHandle %2, i32 9)  ; BarrierByNodeRecordHandle(object,SemanticFlags)
  ret void
}

; Function Attrs: noduplicate nounwind
declare void @dx.op.barrierByMemoryHandle(i32, %dx.types.Handle, i32) #0

; Function Attrs: noduplicate nounwind
declare void @dx.op.barrierByNodeRecordHandle(i32, %dx.types.NodeRecordHandle, i32) #0

; Function Attrs: nounwind readnone
declare %dx.types.Handle @dx.op.annotateHandle(i32, %dx.types.Handle, %dx.types.ResourceProperties) #1

; Function Attrs: nounwind readnone
declare %dx.types.NodeRecordHandle @dx.op.createNodeInputRecordHandle(i32, i32) #1

; Function Attrs: nounwind readnone
declare %dx.types.NodeRecordHandle @dx.op.annotateNodeRecordHandle(i32, %dx.types.NodeRecordHandle, %dx.types.NodeRecordInfo) #1

; Function Attrs: nounwind readonly
declare %dx.types.Handle @dx.op.createHandleForLib.dx.types.Handle(i32, %dx.types.Handle) #2

attributes #0 = { noduplicate nounwind }
attributes #1 = { nounwind readnone }
attributes #2 = { nounwind readonly }

!llvm.ident = !{!0}
!dx.version = !{!1}
!dx.valver = !{!1}
!dx.shaderModel = !{!2}
!dx.resources = !{!3}
!dx.typeAnnotations = !{!7}
!dx.entryPoints = !{!11, !13, !21}

!0 = !{!"dxc(private) 1.7.0.5187 (user/jbatista/validate_Barrier_args01, c232ad072)"}
!1 = !{i32 1, i32 8}
!2 = !{!"lib", i32 6, i32 8}
!3 = !{null, !4, null, null}
!4 = !{!5}
!5 = !{i32 0, %"class.RWBuffer<unsigned int>"* bitcast (%dx.types.Handle* @"\01?buf0@@3V?$RWBuffer@I@@A" to %"class.RWBuffer<unsigned int>"*), !"buf0", i32 -1, i32 -1, i32 1, i32 10, i1 false, i1 false, i1 false, !6}
!6 = !{i32 0, i32 5}
!7 = !{i32 1, void ()* @node01, !8, void ()* @node02, !8}
!8 = !{!9}
!9 = !{i32 0, !10, !10}
!10 = !{}
!11 = !{null, !"", null, !3, !12}
!12 = !{i32 0, i64 8590000128}
!13 = !{void ()* @node01, !"node01", null, null, !14}
!14 = !{i32 8, i32 15, i32 13, i32 1, i32 15, !15, i32 16, i32 -1, i32 20, !16, i32 4, !19, i32 5, !20}
!15 = !{!"node01", i32 0}
!16 = !{!17}
!17 = !{i32 1, i32 97, i32 2, !18}
!18 = !{i32 0, i32 12}
!19 = !{i32 1024, i32 1, i32 1}
!20 = !{i32 0}
!21 = !{void ()* @node02, !"node02", null, null, !22}
!22 = !{i32 8, i32 15, i32 13, i32 1, i32 15, !23, i32 16, i32 -1, i32 20, !16, i32 4, !19, i32 5, !20}
!23 = !{!"node02", i32 0}

; SOURCE HLSL:
;struct RECORD
;{
;  uint a;
;  uint b;
;  uint c;
;};
;RWBuffer<uint> buf0;

;[Shader("node")]
;[NumThreads(1024,1,1)]
;[NodeLaunch("broadcasting")]
;void node01(DispatchNodeInputRecord<RECORD> input)
;{
;  Barrier(buf0,3);
;}

;[Shader("node")]
;[NumThreads(1024,1,1)]
;[NodeLaunch("broadcasting")]
;void node02(DispatchNodeInputRecord<RECORD> input)
;{
;  Barrier(input,3);
;}
