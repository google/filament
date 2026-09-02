; REQUIRES: dxil-1-10
; RUN: not %dxv %s 2>&1 | FileCheck %s

; CHECK: error: Opcode GetGroupWaveCount not valid in shader model lib_6_10(raygeneration).
; CHECK: error: Opcode GetGroupWaveCount not valid in shader model lib_6_10(closesthit).
; CHECK: error: Opcode GetGroupWaveCount not valid in shader model lib_6_10(miss).
; CHECK: error: Opcode GetGroupWaveCount not valid in shader model lib_6_10(anyhit).
; CHECK: error: Opcode GetGroupWaveCount not valid in shader model lib_6_10(intersection).
; CHECK: error: Opcode GetGroupWaveCount not valid in shader model lib_6_10(callable).
; CHECK: error: Opcode GetGroupWaveIndex not valid in shader model lib_6_10(raygeneration).
; CHECK: error: Opcode GetGroupWaveIndex not valid in shader model lib_6_10(closesthit).
; CHECK: error: Opcode GetGroupWaveIndex not valid in shader model lib_6_10(miss).
; CHECK: error: Opcode GetGroupWaveIndex not valid in shader model lib_6_10(anyhit).
; CHECK: error: Opcode GetGroupWaveIndex not valid in shader model lib_6_10(intersection).
; CHECK: error: Opcode GetGroupWaveIndex not valid in shader model lib_6_10(callable).
; CHECK: Validation failed.

target datalayout = "e-m:e-p:32:32-i1:32-i8:32-i16:32-i32:32-i64:64-f16:32-f32:32-f64:64-n8:16:32:64"
target triple = "dxil-ms-dx"

%dx.types.Handle = type { i8* }
%dx.types.ResourceProperties = type { i32, i32 }
%struct.Payload = type { float }
%struct.Attributes = type { <2 x float> }
%"class.RWStructuredBuffer<unsigned int>" = type { i32 }

@"\01?output@@3V?$RWStructuredBuffer@I@@A" = external constant %dx.types.Handle, align 4

; Function Attrs: nounwind
define void @"\01?RayGenMain@@YAXXZ"() #0 {
  %1 = load %dx.types.Handle, %dx.types.Handle* @"\01?output@@3V?$RWStructuredBuffer@I@@A", align 4
  %2 = call i32 @dx.op.getGroupWaveIndex(i32 -2147483647)  ; GetGroupWaveIndex()
  %3 = call i32 @dx.op.getGroupWaveCount(i32 -2147483646)  ; GetGroupWaveCount()
  %4 = add i32 %3, %2
  %5 = call %dx.types.Handle @dx.op.createHandleForLib.dx.types.Handle(i32 160, %dx.types.Handle %1)  ; CreateHandleForLib(Resource)
  %6 = call %dx.types.Handle @dx.op.annotateHandle(i32 216, %dx.types.Handle %5, %dx.types.ResourceProperties { i32 4108, i32 4 })  ; AnnotateHandle(res,props)  resource: RWStructuredBuffer<stride=4>
  call void @dx.op.rawBufferStore.i32(i32 140, %dx.types.Handle %6, i32 0, i32 0, i32 %4, i32 undef, i32 undef, i32 undef, i8 1, i32 4)  ; RawBufferStore(uav,index,elementOffset,value0,value1,value2,value3,mask,alignment)
  ret void
}

; Function Attrs: nounwind
define void @"\01?ClosestHitMain@@YAXUPayload@@UAttributes@@@Z"(%struct.Payload* noalias nocapture %payload, %struct.Attributes* nocapture readnone %attribs) #0 {
  %1 = call i32 @dx.op.getGroupWaveIndex(i32 -2147483647)  ; GetGroupWaveIndex()
  %2 = call i32 @dx.op.getGroupWaveCount(i32 -2147483646)  ; GetGroupWaveCount()
  %3 = add i32 %2, %1
  %4 = uitofp i32 %3 to float
  %5 = getelementptr inbounds %struct.Payload, %struct.Payload* %payload, i32 0, i32 0
  store float %4, float* %5, align 4
  ret void
}

; Function Attrs: nounwind
define void @"\01?MissMain@@YAXUPayload@@@Z"(%struct.Payload* noalias nocapture %payload) #0 {
  %1 = call i32 @dx.op.getGroupWaveIndex(i32 -2147483647)  ; GetGroupWaveIndex()
  %2 = call i32 @dx.op.getGroupWaveCount(i32 -2147483646)  ; GetGroupWaveCount()
  %3 = add i32 %2, %1
  %4 = uitofp i32 %3 to float
  %5 = getelementptr inbounds %struct.Payload, %struct.Payload* %payload, i32 0, i32 0
  store float %4, float* %5, align 4
  ret void
}

; Function Attrs: nounwind
define void @"\01?AnyHitMain@@YAXUPayload@@UAttributes@@@Z"(%struct.Payload* noalias nocapture %payload, %struct.Attributes* nocapture readnone %attribs) #0 {
  %1 = call i32 @dx.op.getGroupWaveIndex(i32 -2147483647)  ; GetGroupWaveIndex()
  %2 = call i32 @dx.op.getGroupWaveCount(i32 -2147483646)  ; GetGroupWaveCount()
  %3 = add i32 %2, %1
  %4 = uitofp i32 %3 to float
  %5 = getelementptr inbounds %struct.Payload, %struct.Payload* %payload, i32 0, i32 0
  store float %4, float* %5, align 4
  ret void
}

; Function Attrs: nounwind
define void @"\01?IntersectionMain@@YAXXZ"() #0 {
  %1 = load %dx.types.Handle, %dx.types.Handle* @"\01?output@@3V?$RWStructuredBuffer@I@@A", align 4
  %2 = call i32 @dx.op.getGroupWaveIndex(i32 -2147483647)  ; GetGroupWaveIndex()
  %3 = call i32 @dx.op.getGroupWaveCount(i32 -2147483646)  ; GetGroupWaveCount()
  %4 = add i32 %3, %2
  %5 = call %dx.types.Handle @dx.op.createHandleForLib.dx.types.Handle(i32 160, %dx.types.Handle %1)  ; CreateHandleForLib(Resource)
  %6 = call %dx.types.Handle @dx.op.annotateHandle(i32 216, %dx.types.Handle %5, %dx.types.ResourceProperties { i32 4108, i32 4 })  ; AnnotateHandle(res,props)  resource: RWStructuredBuffer<stride=4>
  call void @dx.op.rawBufferStore.i32(i32 140, %dx.types.Handle %6, i32 0, i32 0, i32 %4, i32 undef, i32 undef, i32 undef, i8 1, i32 4)  ; RawBufferStore(uav,index,elementOffset,value0,value1,value2,value3,mask,alignment)
  ret void
}

; Function Attrs: nounwind
define void @"\01?CallableMain@@YAXUPayload@@@Z"(%struct.Payload* noalias nocapture %payload) #0 {
  %1 = call i32 @dx.op.getGroupWaveIndex(i32 -2147483647)  ; GetGroupWaveIndex()
  %2 = call i32 @dx.op.getGroupWaveCount(i32 -2147483646)  ; GetGroupWaveCount()
  %3 = add i32 %2, %1
  %4 = uitofp i32 %3 to float
  %5 = getelementptr inbounds %struct.Payload, %struct.Payload* %payload, i32 0, i32 0
  store float %4, float* %5, align 4
  ret void
}

; Function Attrs: nounwind readnone
declare i32 @dx.op.getGroupWaveCount(i32) #1

; Function Attrs: nounwind readnone
declare i32 @dx.op.getGroupWaveIndex(i32) #1

; Function Attrs: nounwind
declare void @dx.op.rawBufferStore.i32(i32, %dx.types.Handle, i32, i32, i32, i32, i32, i32, i8, i32) #0

; Function Attrs: nounwind readnone
declare %dx.types.Handle @dx.op.annotateHandle(i32, %dx.types.Handle, %dx.types.ResourceProperties) #1

; Function Attrs: nounwind readonly
declare %dx.types.Handle @dx.op.createHandleForLib.dx.types.Handle(i32, %dx.types.Handle) #2

attributes #0 = { nounwind }
attributes #1 = { nounwind readnone }
attributes #2 = { nounwind readonly }

!llvm.ident = !{!0}
!dx.version = !{!1}
!dx.valver = !{!1}
!dx.shaderModel = !{!2}
!dx.resources = !{!3}
!dx.typeAnnotations = !{!7}
!dx.dxrPayloadAnnotations = !{!15}
!dx.entryPoints = !{!18, !20, !23, !25, !27, !29, !31}

!0 = !{!"dxc(private) 1.9.0.5169 (Group-Wave-Intrinsics, deeac02f3-dirty)"}
!1 = !{i32 1, i32 10}
!2 = !{!"lib", i32 6, i32 10}
!3 = !{null, !4, null, null}
!4 = !{!5}
!5 = !{i32 0, %"class.RWStructuredBuffer<unsigned int>"* bitcast (%dx.types.Handle* @"\01?output@@3V?$RWStructuredBuffer@I@@A" to %"class.RWStructuredBuffer<unsigned int>"*), !"output", i32 0, i32 0, i32 1, i32 12, i1 false, i1 false, i1 false, !6}
!6 = !{i32 1, i32 4}
!7 = !{i32 1, void ()* @"\01?RayGenMain@@YAXXZ", !8, void (%struct.Payload*, %struct.Attributes*)* @"\01?ClosestHitMain@@YAXUPayload@@UAttributes@@@Z", !11, void (%struct.Payload*)* @"\01?MissMain@@YAXUPayload@@@Z", !14, void (%struct.Payload*, %struct.Attributes*)* @"\01?AnyHitMain@@YAXUPayload@@UAttributes@@@Z", !11, void ()* @"\01?IntersectionMain@@YAXXZ", !8, void (%struct.Payload*)* @"\01?CallableMain@@YAXUPayload@@@Z", !14}
!8 = !{!9}
!9 = !{i32 1, !10, !10}
!10 = !{}
!11 = !{!9, !12, !13}
!12 = !{i32 2, !10, !10}
!13 = !{i32 0, !10, !10}
!14 = !{!9, !12}
!15 = !{i32 0, %struct.Payload undef, !16}
!16 = !{!17}
!17 = !{i32 0, i32 8739}
!18 = !{null, !"", null, !3, !19}
!19 = !{i32 0, i64 8590458896}
!20 = !{void (%struct.Payload*, %struct.Attributes*)* @"\01?AnyHitMain@@YAXUPayload@@UAttributes@@@Z", !"\01?AnyHitMain@@YAXUPayload@@UAttributes@@@Z", null, null, !21}
!21 = !{i32 8, i32 9, i32 6, i32 4, i32 7, i32 8, i32 5, !22}
!22 = !{i32 0}
!23 = !{void (%struct.Payload*)* @"\01?CallableMain@@YAXUPayload@@@Z", !"\01?CallableMain@@YAXUPayload@@@Z", null, null, !24}
!24 = !{i32 8, i32 12, i32 6, i32 4, i32 5, !22}
!25 = !{void (%struct.Payload*, %struct.Attributes*)* @"\01?ClosestHitMain@@YAXUPayload@@UAttributes@@@Z", !"\01?ClosestHitMain@@YAXUPayload@@UAttributes@@@Z", null, null, !26}
!26 = !{i32 8, i32 10, i32 6, i32 4, i32 7, i32 8, i32 5, !22}
!27 = !{void ()* @"\01?IntersectionMain@@YAXXZ", !"\01?IntersectionMain@@YAXXZ", null, null, !28}
!28 = !{i32 8, i32 8, i32 5, !22}
!29 = !{void (%struct.Payload*)* @"\01?MissMain@@YAXUPayload@@@Z", !"\01?MissMain@@YAXUPayload@@@Z", null, null, !30}
!30 = !{i32 8, i32 11, i32 6, i32 4, i32 5, !22}
!31 = !{void ()* @"\01?RayGenMain@@YAXXZ", !"\01?RayGenMain@@YAXXZ", null, null, !32}
!32 = !{i32 8, i32 7, i32 5, !22}
