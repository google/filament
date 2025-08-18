; REQUIRES: dxil-1-9
; RUN: %dxv %s 2>&1 | FileCheck %s

; CHECK: Validation succeeded.

target datalayout = "e-m:e-p:32:32-i1:32-i8:32-i16:32-i32:32-i64:64-f16:32-f32:32-f64:64-n8:16:32:64"
target triple = "dxil-ms-dx"

%dx.types.Handle = type { i8* }
%struct.Payload = type { <3 x float> }
%dx.types.ResourceProperties = type { i32, i32 }
%dx.types.HitObject = type { i8* }
%struct.RaytracingAccelerationStructure = type { i32 }

@"\01?RTAS@@3URaytracingAccelerationStructure@@A" = external constant %dx.types.Handle, align 4

; Function Attrs: nounwind
define void @"\01?main@@YAXXZ"() #0 {
  %1 = load %dx.types.Handle, %dx.types.Handle* @"\01?RTAS@@3URaytracingAccelerationStructure@@A", align 4
  %2 = alloca %struct.Payload, align 4
  %3 = call %dx.types.Handle @dx.op.createHandleForLib.dx.types.Handle(i32 160, %dx.types.Handle %1)  ; CreateHandleForLib(Resource)
  %4 = call %dx.types.Handle @dx.op.annotateHandle(i32 216, %dx.types.Handle %3, %dx.types.ResourceProperties { i32 16, i32 0 })  ; AnnotateHandle(res,props)  resource: RTAccelerationStructure
  %5 = call %dx.types.HitObject @dx.op.hitObject_TraceRay.struct.Payload(i32 262, %dx.types.Handle %4, i32 513, i32 1, i32 2, i32 4, i32 0, float 0.000000e+00, float 1.000000e+00, float 2.000000e+00, float 3.000000e+00, float 4.000000e+00, float 5.000000e+00, float 6.000000e+00, float 7.000000e+00, %struct.Payload* nonnull %2)  ; HitObject_TraceRay(accelerationStructure,rayFlags,instanceInclusionMask,rayContributionToHitGroupIndex,multiplierForGeometryContributionToHitGroupIndex,missShaderIndex,Origin_X,Origin_Y,Origin_Z,TMin,Direction_X,Direction_Y,Direction_Z,TMax,payload)
  call void @dx.op.hitObject_Invoke.struct.Payload(i32 267, %dx.types.HitObject %5, %struct.Payload* nonnull %2)  ; HitObject_Invoke(hitObject,payload)
  ret void
}

; Function Attrs: nounwind
declare %dx.types.HitObject @dx.op.hitObject_TraceRay.struct.Payload(i32, %dx.types.Handle, i32, i32, i32, i32, i32, float, float, float, float, float, float, float, float, %struct.Payload*) #0

; Function Attrs: nounwind
declare void @dx.op.hitObject_Invoke.struct.Payload(i32, %dx.types.HitObject, %struct.Payload*) #0

; Function Attrs: nounwind readnone
declare %dx.types.Handle @dx.op.annotateHandle(i32, %dx.types.Handle, %dx.types.ResourceProperties) #1

; Function Attrs: nounwind readonly
declare %dx.types.Handle @dx.op.createHandleForLib.dx.types.Handle(i32, %dx.types.Handle) #2

attributes #0 = { nounwind }
attributes #1 = { nounwind readnone }
attributes #2 = { nounwind readonly }

!dx.version = !{!0}
!dx.valver = !{!0}
!dx.shaderModel = !{!1}
!dx.resources = !{!2}
!dx.typeAnnotations = !{!3}
!dx.dxrPayloadAnnotations = !{!4}
!dx.entryPoints = !{!5, !6}

!0 = !{i32 1, i32 9}
!1 = !{!"lib", i32 6, i32 9}
!2 = !{!7, null, null, null}
!3 = !{i32 1, void ()* @"\01?main@@YAXXZ", !8}
!4 = !{i32 0, %struct.Payload undef, !9}
!5 = !{null, !"", null, !2, null}
!6 = !{void ()* @"\01?main@@YAXXZ", !"\01?main@@YAXXZ", null, null, !10}
!7 = !{!11}
!8 = !{!12}
!9 = !{!13}
!10 = !{i32 8, i32 7, i32 5, !14}
!11 = !{i32 0, %struct.RaytracingAccelerationStructure* bitcast (%dx.types.Handle* @"\01?RTAS@@3URaytracingAccelerationStructure@@A" to %struct.RaytracingAccelerationStructure*), !"RTAS", i32 -1, i32 -1, i32 1, i32 16, i32 0, !15}
!12 = !{i32 1, !16, !16}
!13 = !{i32 0, i32 8210}
!14 = !{i32 0}
!15 = !{i32 0, i32 4}
!16 = !{}
