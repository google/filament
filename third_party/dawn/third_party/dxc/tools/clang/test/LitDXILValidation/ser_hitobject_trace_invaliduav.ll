; REQUIRES: dxil-1-9
; RUN: not %dxv %s 2>&1 | FileCheck %s

; shader hash: b22988e7874179601860019e56fb877e
;
; Buffer Definitions:
;
;
; Resource Bindings:
;
; Name                                 Type  Format         Dim      ID      HLSL Bind  Count
; ------------------------------ ---------- ------- ----------- ------- -------------- ------
; RTAS                              texture     i32         ras      T0t4294967295,space4294967295     1
; nonas_buf                             UAV    byte         r/w      U0u4294967295,space4294967295     1
;
target datalayout = "e-m:e-p:32:32-i1:32-i8:32-i16:32-i32:32-i64:64-f16:32-f32:32-f64:64-n8:16:32:64"
target triple = "dxil-ms-dx"

%dx.types.Handle = type { i8* }
%struct.Payload = type { <3 x float> }
%dx.types.ResourceProperties = type { i32, i32 }
%dx.types.HitObject = type { i8* }
%struct.RaytracingAccelerationStructure = type { i32 }
%struct.RWByteAddressBuffer = type { i32 }

@"\01?RTAS@@3URaytracingAccelerationStructure@@A" = external constant %dx.types.Handle, align 4
@"\01?nonas_buf@@3URWByteAddressBuffer@@A" = external constant %dx.types.Handle, align 4

; CHECK: Function: ?main@@YAXXZ: error: TraceRay should only use RTAccelerationStructure.
; CHECK-NEXT: note: at '%invalid = call %dx.types.HitObject @dx.op.hitObject_TraceRay.struct.Payload(i32 262, %dx.types.Handle %7, i32 513, i32 1, i32 2, i32 4, i32 0, float 0.000000e+00, float 1.000000e+00, float 2.000000e+00, float 3.000000e+00, float 4.000000e+00, float 5.000000e+00, float 6.000000e+00, float 7.000000e+00, %struct.Payload* nonnull %3)' in block '#0' of function '?main@@YAXXZ'.
; CHECK-NEXT: Validation failed.

; Function Attrs: nounwind
define void @"\01?main@@YAXXZ"() #0 {
  %1 = load %dx.types.Handle, %dx.types.Handle* @"\01?RTAS@@3URaytracingAccelerationStructure@@A", align 4
  %2 = load %dx.types.Handle, %dx.types.Handle* @"\01?nonas_buf@@3URWByteAddressBuffer@@A", align 4
  %3 = alloca %struct.Payload, align 4
  %4 = bitcast %struct.Payload* %3 to i8*
  call void @llvm.lifetime.start(i64 12, i8* %4) #0
  %5 = getelementptr inbounds %struct.Payload, %struct.Payload* %3, i32 0, i32 0
  store <3 x float> <float 7.000000e+00, float 8.000000e+00, float 9.000000e+00>, <3 x float>* %5, align 4, !tbaa !20
  %6 = call %dx.types.Handle @dx.op.createHandleForLib.dx.types.Handle(i32 160, %dx.types.Handle %2)  ; CreateHandleForLib(Resource)
  %7 = call %dx.types.Handle @dx.op.annotateHandle(i32 216, %dx.types.Handle %6, %dx.types.ResourceProperties { i32 4107, i32 0 })  ; AnnotateHandle(res,props)  resource: RWByteAddressBuffer
  call void @dx.op.rawBufferStore.f32(i32 140, %dx.types.Handle %7, i32 0, i32 undef, float 1.100000e+01, float undef, float undef, float undef, i8 1, i32 4)  ; RawBufferStore(uav,index,elementOffset,value0,value1,value2,value3,mask,alignment)
  %8 = call %dx.types.Handle @dx.op.createHandleForLib.dx.types.Handle(i32 160, %dx.types.Handle %1)  ; CreateHandleForLib(Resource)
  %9 = call %dx.types.Handle @dx.op.annotateHandle(i32 216, %dx.types.Handle %8, %dx.types.ResourceProperties { i32 16, i32 0 })  ; AnnotateHandle(res,props)  resource: RTAccelerationStructure

  %valid = call %dx.types.HitObject @dx.op.hitObject_TraceRay.struct.Payload(i32 262, %dx.types.Handle %9, i32 513, i32 1, i32 2, i32 4, i32 0, float 0.000000e+00, float 1.000000e+00, float 2.000000e+00, float 3.000000e+00, float 4.000000e+00, float 5.000000e+00, float 6.000000e+00, float 7.000000e+00, %struct.Payload* nonnull %3)  ; HitObject_TraceRay(accelerationStructure,rayFlags,instanceInclusionMask,rayContributionToHitGroupIndex,multiplierForGeometryContributionToHitGroupIndex,missShaderIndex,Origin_X,Origin_Y,Origin_Z,TMin,Direction_X,Direction_Y,Direction_Z,TMax,payload)

  %invalid = call %dx.types.HitObject @dx.op.hitObject_TraceRay.struct.Payload(i32 262, %dx.types.Handle %7, i32 513, i32 1, i32 2, i32 4, i32 0, float 0.000000e+00, float 1.000000e+00, float 2.000000e+00, float 3.000000e+00, float 4.000000e+00, float 5.000000e+00, float 6.000000e+00, float 7.000000e+00, %struct.Payload* nonnull %3)  ; HitObject_TraceRay(accelerationStructure,rayFlags,instanceInclusionMask,rayContributionToHitGroupIndex,multiplierForGeometryContributionToHitGroupIndex,missShaderIndex,Origin_X,Origin_Y,Origin_Z,TMin,Direction_X,Direction_Y,Direction_Z,TMax,payload)

  call void @llvm.lifetime.end(i64 12, i8* %4) #0
  ret void
}

; Function Attrs: nounwind
declare void @llvm.lifetime.start(i64, i8* nocapture) #0

; Function Attrs: nounwind
declare void @llvm.lifetime.end(i64, i8* nocapture) #0

; Function Attrs: nounwind
declare void @dx.op.rawBufferStore.f32(i32, %dx.types.Handle, i32, i32, float, float, float, float, i8, i32) #0

; Function Attrs: nounwind
declare %dx.types.HitObject @dx.op.hitObject_TraceRay.struct.Payload(i32, %dx.types.Handle, i32, i32, i32, i32, i32, float, float, float, float, float, float, float, float, %struct.Payload*) #0

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
!dx.typeAnnotations = !{!8}
!dx.dxrPayloadAnnotations = !{!12}
!dx.entryPoints = !{!15, !17}

!0 = !{i32 1, i32 9}
!1 = !{!"lib", i32 6, i32 9}
!2 = !{!3, !6, null, null}
!3 = !{!4}
!4 = !{i32 0, %struct.RaytracingAccelerationStructure* bitcast (%dx.types.Handle* @"\01?RTAS@@3URaytracingAccelerationStructure@@A" to %struct.RaytracingAccelerationStructure*), !"RTAS", i32 -1, i32 -1, i32 1, i32 16, i32 0, !5}
!5 = !{i32 0, i32 4}
!6 = !{!7}
!7 = !{i32 0, %struct.RWByteAddressBuffer* bitcast (%dx.types.Handle* @"\01?nonas_buf@@3URWByteAddressBuffer@@A" to %struct.RWByteAddressBuffer*), !"nonas_buf", i32 -1, i32 -1, i32 1, i32 11, i1 false, i1 false, i1 false, null}
!8 = !{i32 1, void ()* @"\01?main@@YAXXZ", !9}
!9 = !{!10}
!10 = !{i32 1, !11, !11}
!11 = !{}
!12 = !{i32 0, %struct.Payload undef, !13}
!13 = !{!14}
!14 = !{i32 0, i32 8210}
!15 = !{null, !"", null, !2, !16}
!16 = !{i32 0, i64 8589934608}
!17 = !{void ()* @"\01?main@@YAXXZ", !"\01?main@@YAXXZ", null, null, !18}
!18 = !{i32 8, i32 7, i32 5, !19}
!19 = !{i32 0}
!20 = !{!21, !21, i64 0}
!21 = !{!"omnipotent char", !22, i64 0}
!22 = !{!"Simple C/C++ TBAA"}
