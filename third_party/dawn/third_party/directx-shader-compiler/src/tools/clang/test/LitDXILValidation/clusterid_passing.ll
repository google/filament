; REQUIRES: dxil-1-10
; RUN: %dxv %s | FileCheck %s

; CHECK: Validation succeeded.

target datalayout = "e-m:e-p:32:32-i1:32-i8:32-i16:32-i32:32-i64:64-f16:32-f32:32-f64:64-n8:16:32:64"
target triple = "dxil-ms-dx"

%dx.types.Handle = type { i8* }
%struct.Payload = type { <4 x float> }
%struct.BuiltInTriangleIntersectionAttributes = type { <2 x float> }
%dx.types.ResourceProperties = type { i32, i32 }
%dx.types.HitObject = type { i8* }

; Function Attrs: nounwind
define void @"\01?test_closesthit@@YAXUPayload@@UBuiltInTriangleIntersectionAttributes@@@Z"(%struct.Payload* noalias nocapture %payload, %struct.BuiltInTriangleIntersectionAttributes* nocapture readnone %attr) #0 {
  %1 = call i32 @dx.op.clusterID(i32 -2147483645)  ; ClusterID()
  ret void
}

; Function Attrs: nounwind
define void @"\01?test_raygeneration@@YAXXZ"() #0 {
  %1 = call %dx.types.HitObject @dx.op.hitObject_MakeNop(i32 266)  ; HitObject_MakeNop()
  %2 = call i32 @dx.op.hitObject_StateScalar.i32(i32 -2147483642, %dx.types.HitObject %1)  ; HitObject_ClusterID(hitObject)
  %3 = call i32 @dx.op.allocateRayQuery(i32 178, i32 0)  ; AllocateRayQuery(constRayFlags)
  %4 = call i32 @dx.op.rayQuery_StateScalar.i32(i32 -2147483644, i32 %3)  ; RayQuery_CandidateClusterID(rayQueryHandle)
  %5 = call i32 @dx.op.rayQuery_StateScalar.i32(i32 -2147483643, i32 %3)  ; RayQuery_CommittedClusterID(rayQueryHandle)
  ret void
}

; Function Attrs: nounwind readnone
declare i32 @dx.op.clusterID(i32) #1

; Function Attrs: nounwind readnone
declare %dx.types.HitObject @dx.op.hitObject_MakeNop(i32) #1

; Function Attrs: nounwind readnone
declare i32 @dx.op.hitObject_StateScalar.i32(i32, %dx.types.HitObject) #1

; Function Attrs: nounwind
declare i32 @dx.op.allocateRayQuery(i32, i32) #0

; Function Attrs: nounwind readonly
declare i32 @dx.op.rayQuery_StateScalar.i32(i32, i32) #2

attributes #0 = { nounwind }
attributes #1 = { nounwind readnone }
attributes #2 = { nounwind readonly }

!dx.version = !{!0}
!dx.valver = !{!0}
!dx.shaderModel = !{!1}
!dx.typeAnnotations = !{!2}
!dx.dxrPayloadAnnotations = !{!9}
!dx.entryPoints = !{!12, !14, !17}

!0 = !{i32 1, i32 10}
!1 = !{!"lib", i32 6, i32 10}
!2 = !{i32 1, void (%struct.Payload*, %struct.BuiltInTriangleIntersectionAttributes*)* @"\01?test_closesthit@@YAXUPayload@@UBuiltInTriangleIntersectionAttributes@@@Z", !3, void ()* @"\01?test_raygeneration@@YAXXZ", !8}
!3 = !{!4, !6, !7}
!4 = !{i32 1, !5, !5}
!5 = !{}
!6 = !{i32 2, !5, !5}
!7 = !{i32 0, !5, !5}
!8 = !{!4}
!9 = !{i32 0, %struct.Payload undef, !10}
!10 = !{!11}
!11 = !{i32 0, i32 33}
!12 = !{null, !"", null, null, !13}
!13 = !{i32 0, i64 33554432}
!14 = !{void (%struct.Payload*, %struct.BuiltInTriangleIntersectionAttributes*)* @"\01?test_closesthit@@YAXUPayload@@UBuiltInTriangleIntersectionAttributes@@@Z", !"\01?test_closesthit@@YAXUPayload@@UBuiltInTriangleIntersectionAttributes@@@Z", null, null, !15}
!15 = !{i32 8, i32 10, i32 6, i32 16, i32 7, i32 8, i32 5, !16}
!16 = !{i32 0}
!17 = !{void ()* @"\01?test_raygeneration@@YAXXZ", !"\01?test_raygeneration@@YAXXZ", null, null, !18}
!18 = !{i32 8, i32 7, i32 5, !16}
