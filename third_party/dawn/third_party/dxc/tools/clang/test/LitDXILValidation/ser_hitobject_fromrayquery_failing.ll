; REQUIRES: dxil-1-9
; RUN: not %dxv %s 2>&1 | FileCheck %s

; CHECK: Function: ?main@@YAXXZ: error: Instructions should not read uninitialized value.
; CHECK-NEXT: note: at '%attrsud3 = call %dx.types.HitObject @dx.op.hitObject_FromRayQueryWithAttrs.struct.CustomAttrs(i32 264, i32 %rq, i32 16, %struct.CustomAttrs* nonnull undef)' in block '#0' of function '?main@@YAXXZ'.
; CHECK-NEXT: Function: ?main@@YAXXZ: error: Instructions should not read uninitialized value.
; CHECK-NEXT: note: at '%attrsud2 = call %dx.types.HitObject @dx.op.hitObject_FromRayQueryWithAttrs.struct.CustomAttrs(i32 264, i32 %rq, i32 undef, %struct.CustomAttrs* nonnull %attra)' in block '#0' of function '?main@@YAXXZ'.
; CHECK-NEXT: Function: ?main@@YAXXZ: error: Instructions should not read uninitialized value.
; CHECK-NEXT: note: at '%attrsud1 = call %dx.types.HitObject @dx.op.hitObject_FromRayQueryWithAttrs.struct.CustomAttrs(i32 264, i32 undef, i32 16, %struct.CustomAttrs* nonnull %attra)' in block '#0' of function '?main@@YAXXZ'.
; CHECK-NEXT: Function: ?main@@YAXXZ: error: Instructions should not read uninitialized value.
; CHECK-NEXT: note: at '%ud1 = call %dx.types.HitObject @dx.op.hitObject_FromRayQuery(i32 263, i32 undef)' in block '#0' of function '?main@@YAXXZ'.
; CHECK-NEXT: Validation failed.

target datalayout = "e-m:e-p:32:32-i1:32-i8:32-i16:32-i32:32-i64:64-f16:32-f32:32-f64:64-n8:16:32:64"
target triple = "dxil-ms-dx"

%dx.types.Handle = type { i8* }
%struct.Payload = type { <3 x float> }
%struct.CustomAttrs = type { float, float }
%dx.types.ResourceProperties = type { i32, i32 }
%dx.types.HitObject = type { i8* }
%struct.RaytracingAccelerationStructure = type { i32 }

@"\01?RTAS@@3URaytracingAccelerationStructure@@A" = external constant %dx.types.Handle, align 4

; Function Attrs: nounwind
declare void @llvm.lifetime.start(i64, i8* nocapture) #0

; Function Attrs: nounwind
declare void @llvm.lifetime.end(i64, i8* nocapture) #0

; Function Attrs: nounwind
define void @"\01?main@@YAXXZ"() #0 {
  %ldh = load %dx.types.Handle, %dx.types.Handle* @"\01?RTAS@@3URaytracingAccelerationStructure@@A", align 4
  %attra = alloca %struct.CustomAttrs, align 4
  %rq = call i32 @dx.op.allocateRayQuery(i32 178, i32 5)  ; AllocateRayQuery(constRayFlags)
  %createh = call %dx.types.Handle @dx.op.createHandleForLib.dx.types.Handle(i32 160, %dx.types.Handle %ldh)  ; CreateHandleForLib(Resource)
  %annoth = call %dx.types.Handle @dx.op.annotateHandle(i32 216, %dx.types.Handle %createh, %dx.types.ResourceProperties { i32 16, i32 0 })  ; AnnotateHandle(res,props)  resource: RTAccelerationStructure
  call void @dx.op.rayQuery_TraceRayInline(i32 179, i32 %rq, %dx.types.Handle %annoth, i32 0, i32 255, float 0.000000e+00, float 0.000000e+00, float 0.000000e+00, float 0.000000e+00, float 1.000000e+00, float 0.000000e+00, float 0.000000e+00, float 9.999000e+03)  ; RayQuery_TraceRayInline(rayQueryHandle,accelerationStructure,rayFlags,instanceInclusionMask,origin_X,origin_Y,origin_Z,tMin,direction_X,direction_Y,direction_Z,tMax)

  %ok = call %dx.types.HitObject @dx.op.hitObject_FromRayQuery(i32 263, i32 %rq)  ; HitObject_FromRayQuery(rayQueryHandle)
  %ud1 = call %dx.types.HitObject @dx.op.hitObject_FromRayQuery(i32 263, i32 undef)  ; HitObject_FromRayQuery(rayQueryHandle)

  %attrsok = call %dx.types.HitObject @dx.op.hitObject_FromRayQueryWithAttrs.struct.CustomAttrs(i32 264, i32 %rq, i32 16, %struct.CustomAttrs* nonnull %attra)  ; HitObject_FromRayQueryWithAttrs(rayQueryHandle,HitKind,CommittedAttribs)
  %attrsud1 = call %dx.types.HitObject @dx.op.hitObject_FromRayQueryWithAttrs.struct.CustomAttrs(i32 264, i32 undef, i32 16, %struct.CustomAttrs* nonnull %attra)  ; HitObject_FromRayQueryWithAttrs(rayQueryHandle,HitKind,CommittedAttribs)
  %attrsud2 = call %dx.types.HitObject @dx.op.hitObject_FromRayQueryWithAttrs.struct.CustomAttrs(i32 264, i32 %rq, i32 undef, %struct.CustomAttrs* nonnull %attra)  ; HitObject_FromRayQueryWithAttrs(rayQueryHandle,HitKind,CommittedAttribs)
  %attrsud3 = call %dx.types.HitObject @dx.op.hitObject_FromRayQueryWithAttrs.struct.CustomAttrs(i32 264, i32 %rq, i32 16, %struct.CustomAttrs* nonnull undef)  ; HitObject_FromRayQueryWithAttrs(rayQueryHandle,HitKind,CommittedAttribs)

  ret void
}

; Function Attrs: nounwind
declare i32 @dx.op.allocateRayQuery(i32, i32) #0

; Function Attrs: nounwind
declare void @dx.op.rayQuery_TraceRayInline(i32, i32, %dx.types.Handle, i32, i32, float, float, float, float, float, float, float, float) #0

; Function Attrs: nounwind readonly
declare %dx.types.HitObject @dx.op.hitObject_FromRayQueryWithAttrs.struct.CustomAttrs(i32, i32, i32, %struct.CustomAttrs*) #1

; Function Attrs: nounwind readonly
declare %dx.types.HitObject @dx.op.hitObject_FromRayQuery(i32, i32) #1

; Function Attrs: nounwind readnone
declare %dx.types.Handle @dx.op.annotateHandle(i32, %dx.types.Handle, %dx.types.ResourceProperties) #2

; Function Attrs: nounwind readonly
declare %dx.types.Handle @dx.op.createHandleForLib.dx.types.Handle(i32, %dx.types.Handle) #1

attributes #0 = { nounwind }
attributes #1 = { nounwind readonly }
attributes #2 = { nounwind readnone }

!dx.version = !{!0}
!dx.valver = !{!0}
!dx.shaderModel = !{!1}
!dx.resources = !{!2}
!dx.typeAnnotations = !{!6}
!dx.dxrPayloadAnnotations = !{!10}
!dx.entryPoints = !{!13, !15}

!0 = !{i32 1, i32 9}
!1 = !{!"lib", i32 6, i32 9}
!2 = !{!3, null, null, null}
!3 = !{!4}
!4 = !{i32 0, %struct.RaytracingAccelerationStructure* bitcast (%dx.types.Handle* @"\01?RTAS@@3URaytracingAccelerationStructure@@A" to %struct.RaytracingAccelerationStructure*), !"RTAS", i32 -1, i32 -1, i32 1, i32 16, i32 0, !5}
!5 = !{i32 0, i32 4}
!6 = !{i32 1, void ()* @"\01?main@@YAXXZ", !7}
!7 = !{!8}
!8 = !{i32 1, !9, !9}
!9 = !{}
!10 = !{i32 0, %struct.Payload undef, !11}
!11 = !{!12}
!12 = !{i32 0, i32 8210}
!13 = !{null, !"", null, !2, !14}
!14 = !{i32 0, i64 33554432}
!15 = !{void ()* @"\01?main@@YAXXZ", !"\01?main@@YAXXZ", null, null, !16}
!16 = !{i32 8, i32 7, i32 5, !17}
!17 = !{i32 0}
