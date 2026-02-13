; REQUIRES: dxil-1-9
; RUN: not %dxv %s 2>&1 | FileCheck %s

target datalayout = "e-m:e-p:32:32-i1:32-i8:32-i16:32-i32:32-i64:64-f16:32-f32:32-f64:64-n8:16:32:64"
target triple = "dxil-ms-dx"

%dx.types.HitObject = type { i8* }

; CHECK: Function: ?main@@YAXXZ: error: HitObject is undef.
; CHECK: note: at '%r287_ud = call %dx.types.HitObject @dx.op.hitObject_SetShaderTableIndex(i32 287, %dx.types.HitObject undef, i32 undef)' in block '#0' of function '?main@@YAXXZ'.
; CHECK: Function: ?main@@YAXXZ: error: Instructions should not read uninitialized value.
; CHECK: note: at '%r287_ud = call %dx.types.HitObject @dx.op.hitObject_SetShaderTableIndex(i32 287, %dx.types.HitObject undef, i32 undef)' in block '#0' of function '?main@@YAXXZ'.
; CHECK: Function: ?main@@YAXXZ: error: HitObject is undef.
; CHECK: note: at '%r287 = call %dx.types.HitObject @dx.op.hitObject_SetShaderTableIndex(i32 287, %dx.types.HitObject undef, i32 1)' in block '#0' of function '?main@@YAXXZ'.
; CHECK: Function: ?main@@YAXXZ: error: HitObject is undef.
; CHECK: note: at '%r271 = call i1 @dx.op.hitObject_StateScalar.i1(i32 271, %dx.types.HitObject undef)' in block '#0' of function '?main@@YAXXZ'.
; CHECK: Function: ?main@@YAXXZ: error: HitObject is undef.
; CHECK: note: at '%r270 = call i1 @dx.op.hitObject_StateScalar.i1(i32 270, %dx.types.HitObject undef)' in block '#0' of function '?main@@YAXXZ'.
; CHECK: Function: ?main@@YAXXZ: error: HitObject is undef.
; CHECK: note: at '%r269 = call i1 @dx.op.hitObject_StateScalar.i1(i32 269, %dx.types.HitObject undef)' in block '#0' of function '?main@@YAXXZ'.
; CHECK: Function: ?main@@YAXXZ: error: HitObject is undef.
; CHECK: note: at '%r286 = call i32 @dx.op.hitObject_StateScalar.i32(i32 286, %dx.types.HitObject undef)' in block '#0' of function '?main@@YAXXZ'.
; CHECK: Function: ?main@@YAXXZ: error: HitObject is undef.
; CHECK: note: at '%r285 = call i32 @dx.op.hitObject_StateScalar.i32(i32 285, %dx.types.HitObject undef)' in block '#0' of function '?main@@YAXXZ'.
; CHECK: Function: ?main@@YAXXZ: error: HitObject is undef.
; CHECK: note: at '%r284 = call i32 @dx.op.hitObject_StateScalar.i32(i32 284, %dx.types.HitObject undef)' in block '#0' of function '?main@@YAXXZ'.
; CHECK: Function: ?main@@YAXXZ: error: HitObject is undef.
; CHECK: note: at '%r283 = call i32 @dx.op.hitObject_StateScalar.i32(i32 283, %dx.types.HitObject undef)' in block '#0' of function '?main@@YAXXZ'.
; CHECK: Function: ?main@@YAXXZ: error: HitObject is undef.
; CHECK: note: at '%r282 = call i32 @dx.op.hitObject_StateScalar.i32(i32 282, %dx.types.HitObject undef)' in block '#0' of function '?main@@YAXXZ'.
; CHECK: Function: ?main@@YAXXZ: error: HitObject is undef.
; CHECK: note: at '%r281 = call i32 @dx.op.hitObject_StateScalar.i32(i32 281, %dx.types.HitObject undef)' in block '#0' of function '?main@@YAXXZ'.
; CHECK: Function: ?main@@YAXXZ: error: HitObject is undef.
; CHECK: note: at '%r272 = call i32 @dx.op.hitObject_StateScalar.i32(i32 272, %dx.types.HitObject undef)' in block '#0' of function '?main@@YAXXZ'.
; CHECK: Function: ?main@@YAXXZ: error: HitObject is undef.
; CHECK: note: at '%r288_wrongmul = call i32 @dx.op.hitObject_LoadLocalRootTableConstant(i32 288, %dx.types.HitObject undef, i32 7)' in block '#0' of function '?main@@YAXXZ'.
; CHECK: Function: ?main@@YAXXZ: error: parameter 'offset' must be a multiple of 4, got 7
; CHECK: note: at '%r288_wrongmul = call i32 @dx.op.hitObject_LoadLocalRootTableConstant(i32 288, %dx.types.HitObject undef, i32 7)' in block '#0' of function '?main@@YAXXZ'.
; CHECK: Function: ?main@@YAXXZ: error: HitObject is undef.
; CHECK: note: at '%r288 = call i32 @dx.op.hitObject_LoadLocalRootTableConstant(i32 288, %dx.types.HitObject undef, i32 42)' in block '#0' of function '?main@@YAXXZ'.
; CHECK: Function: ?main@@YAXXZ: error: parameter 'offset' must be a multiple of 4, got 42
; CHECK: note: at '%r288 = call i32 @dx.op.hitObject_LoadLocalRootTableConstant(i32 288, %dx.types.HitObject undef, i32 42)' in block '#0' of function '?main@@YAXXZ'.
; CHECK: Function: ?main@@YAXXZ: error: expect component between 0~2, got 3.
; CHECK: note: at '%r278_oobc = call float @dx.op.hitObject_StateVector.f32(i32 278, %dx.types.HitObject %nop, i32 3)' in block '#0' of function '?main@@YAXXZ'.
; CHECK: Function: ?main@@YAXXZ: error: component of HitObject_ObjectRayDirection must be an immediate constant.
; CHECK: note: at '%r278_dync = call float @dx.op.hitObject_StateVector.f32(i32 278, %dx.types.HitObject %nop, i32 %r272)' in block '#0' of function '?main@@YAXXZ'.
; CHECK: Function: ?main@@YAXXZ: error: HitObject is undef.
; CHECK: note: at '%r278 = call float @dx.op.hitObject_StateVector.f32(i32 278, %dx.types.HitObject undef, i32 0)' in block '#0' of function '?main@@YAXXZ'.
; CHECK: Function: ?main@@YAXXZ: error: expect component between 0~2, got 3.
; CHECK: note: at '%r277_oobc = call float @dx.op.hitObject_StateVector.f32(i32 277, %dx.types.HitObject %nop, i32 3)' in block '#0' of function '?main@@YAXXZ'.
; CHECK: Function: ?main@@YAXXZ: error: component of HitObject_ObjectRayOrigin must be an immediate constant.
; CHECK: note: at '%r277_dync = call float @dx.op.hitObject_StateVector.f32(i32 277, %dx.types.HitObject %nop, i32 %r272)' in block '#0' of function '?main@@YAXXZ'.
; CHECK: Function: ?main@@YAXXZ: error: HitObject is undef.
; CHECK: note: at '%r277 = call float @dx.op.hitObject_StateVector.f32(i32 277, %dx.types.HitObject undef, i32 0)' in block '#0' of function '?main@@YAXXZ'.
; CHECK: Function: ?main@@YAXXZ: error: expect component between 0~2, got 3.
; CHECK: note: at '%r276_oobc = call float @dx.op.hitObject_StateVector.f32(i32 276, %dx.types.HitObject %nop, i32 3)' in block '#0' of function '?main@@YAXXZ'.
; CHECK: Function: ?main@@YAXXZ: error: component of HitObject_WorldRayDirection must be an immediate constant.
; CHECK: note: at '%r276_dync = call float @dx.op.hitObject_StateVector.f32(i32 276, %dx.types.HitObject %nop, i32 %r272)' in block '#0' of function '?main@@YAXXZ'.
; CHECK: Function: ?main@@YAXXZ: error: HitObject is undef.
; CHECK: note: at '%r276 = call float @dx.op.hitObject_StateVector.f32(i32 276, %dx.types.HitObject undef, i32 0)' in block '#0' of function '?main@@YAXXZ'.
; CHECK: Function: ?main@@YAXXZ: error: expect component between 0~2, got 3.
; CHECK: note: at '%r275_oobc = call float @dx.op.hitObject_StateVector.f32(i32 275, %dx.types.HitObject %nop, i32 3)' in block '#0' of function '?main@@YAXXZ'.
; CHECK: Function: ?main@@YAXXZ: error: component of HitObject_WorldRayOrigin must be an immediate constant.
; CHECK: note: at '%r275_dync = call float @dx.op.hitObject_StateVector.f32(i32 275, %dx.types.HitObject %nop, i32 %r272)' in block '#0' of function '?main@@YAXXZ'.
; CHECK: Function: ?main@@YAXXZ: error: HitObject is undef.
; CHECK: note: at '%r275 = call float @dx.op.hitObject_StateVector.f32(i32 275, %dx.types.HitObject undef, i32 0)' in block '#0' of function '?main@@YAXXZ'.
; CHECK: Function: ?main@@YAXXZ: error: HitObject is undef.
; CHECK: note: at '%r274 = call float @dx.op.hitObject_StateScalar.f32(i32 274, %dx.types.HitObject undef)' in block '#0' of function '?main@@YAXXZ'.
; CHECK: Function: ?main@@YAXXZ: error: HitObject is undef.
; CHECK: note: at '%r273 = call float @dx.op.hitObject_StateScalar.f32(i32 273, %dx.types.HitObject undef)' in block '#0' of function '?main@@YAXXZ'.
; CHECK: Function: ?main@@YAXXZ: error: expect column between 0~3, got 4.
; CHECK: note: at '%r280_oobc = call float @dx.op.hitObject_StateMatrix.f32(i32 280, %dx.types.HitObject %nop, i32 0, i32 4)' in block '#0' of function '?main@@YAXXZ'.
; CHECK: Function: ?main@@YAXXZ: error: column of HitObject_WorldToObject3x4 must be an immediate constant.
; CHECK: note: at '%r280_dync = call float @dx.op.hitObject_StateMatrix.f32(i32 280, %dx.types.HitObject %nop, i32 0, i32 %r272)' in block '#0' of function '?main@@YAXXZ'.
; CHECK: Function: ?main@@YAXXZ: error: expect row between 0~2, got 3.
; CHECK: note: at '%r280_oobr = call float @dx.op.hitObject_StateMatrix.f32(i32 280, %dx.types.HitObject %nop, i32 3, i32 0)' in block '#0' of function '?main@@YAXXZ'.
; CHECK: Function: ?main@@YAXXZ: error: row of HitObject_WorldToObject3x4 must be an immediate constant.
; CHECK: note: at '%r280_dynr = call float @dx.op.hitObject_StateMatrix.f32(i32 280, %dx.types.HitObject %nop, i32 %r272, i32 0)' in block '#0' of function '?main@@YAXXZ'.
; CHECK: Function: ?main@@YAXXZ: error: HitObject is undef.
; CHECK: note: at '%r280 = call float @dx.op.hitObject_StateMatrix.f32(i32 280, %dx.types.HitObject undef, i32 0, i32 0)' in block '#0' of function '?main@@YAXXZ'.
; CHECK: Function: ?main@@YAXXZ: error: expect column between 0~3, got 4.
; CHECK: note: at '%r279_oobc = call float @dx.op.hitObject_StateMatrix.f32(i32 279, %dx.types.HitObject %nop, i32 0, i32 4)' in block '#0' of function '?main@@YAXXZ'.
; CHECK: Function: ?main@@YAXXZ: error: column of HitObject_ObjectToWorld3x4 must be an immediate constant.
; CHECK: note: at '%r279_dync = call float @dx.op.hitObject_StateMatrix.f32(i32 279, %dx.types.HitObject %nop, i32 0, i32 %r272)' in block '#0' of function '?main@@YAXXZ'.
; CHECK: Function: ?main@@YAXXZ: error: expect row between 0~2, got 3.
; CHECK: note: at '%r279_oobr = call float @dx.op.hitObject_StateMatrix.f32(i32 279, %dx.types.HitObject %nop, i32 3, i32 0)' in block '#0' of function '?main@@YAXXZ'.
; CHECK: Function: ?main@@YAXXZ: error: row of HitObject_ObjectToWorld3x4 must be an immediate constant.
; CHECK: note: at '%r279_dynr = call float @dx.op.hitObject_StateMatrix.f32(i32 279, %dx.types.HitObject %nop, i32 %r272, i32 0)' in block '#0' of function '?main@@YAXXZ'.
; CHECK: Function: ?main@@YAXXZ: error: HitObject is undef.
; CHECK: note: at '%r279 = call float @dx.op.hitObject_StateMatrix.f32(i32 279, %dx.types.HitObject undef, i32 0, i32 0)' in block '#0' of function '?main@@YAXXZ'.
; CHECK: Validation failed.

; Function Attrs: nounwind
define void @"\01?main@@YAXXZ"() #0 {
%nop = call %dx.types.HitObject @dx.op.hitObject_MakeNop(i32 266)  ; HitObject_MakeNop()
  %r269 = call i1 @dx.op.hitObject_StateScalar.i1(i32 269, %dx.types.HitObject undef)  ; HitObject_IsMiss(hitObject)

  %r270 = call i1 @dx.op.hitObject_StateScalar.i1(i32 270, %dx.types.HitObject undef)  ; HitObject_IsHit(hitObject)

  %r271 = call i1 @dx.op.hitObject_StateScalar.i1(i32 271, %dx.types.HitObject undef)  ; HitObject_IsNop(hitObject)

  %r272 = call i32 @dx.op.hitObject_StateScalar.i32(i32 272, %dx.types.HitObject undef)  ; HitObject_RayFlags(hitObject)

  %r273 = call float @dx.op.hitObject_StateScalar.f32(i32 273, %dx.types.HitObject undef)  ; HitObject_RayTMin(hitObject)

  %r274 = call float @dx.op.hitObject_StateScalar.f32(i32 274, %dx.types.HitObject undef)  ; HitObject_RayTCurrent(hitObject)

  %r275 = call float @dx.op.hitObject_StateVector.f32(i32 275, %dx.types.HitObject undef, i32 0)  ; HitObject_WorldRayOrigin(hitObject,component)
  %r275_dync = call float @dx.op.hitObject_StateVector.f32(i32 275, %dx.types.HitObject %nop, i32 %r272)  ; HitObject_WorldRayOrigin(hitObject,component)
  %r275_oobc = call float @dx.op.hitObject_StateVector.f32(i32 275, %dx.types.HitObject %nop, i32 3)  ; HitObject_WorldRayOrigin(hitObject,component)

  %r276 = call float @dx.op.hitObject_StateVector.f32(i32 276, %dx.types.HitObject undef, i32 0)  ; HitObject_WorldRayDirection(hitObject,component)
  %r276_dync = call float @dx.op.hitObject_StateVector.f32(i32 276, %dx.types.HitObject %nop, i32 %r272)  ; HitObject_WorldRayDirection(hitObject,component)
  %r276_oobc = call float @dx.op.hitObject_StateVector.f32(i32 276, %dx.types.HitObject %nop, i32 3)  ; HitObject_WorldRayDirection(hitObject,component)

  %r277 = call float @dx.op.hitObject_StateVector.f32(i32 277, %dx.types.HitObject undef, i32 0)  ; HitObject_ObjectRayOrigin(hitObject,component)
  %r277_dync = call float @dx.op.hitObject_StateVector.f32(i32 277, %dx.types.HitObject %nop, i32 %r272)  ; HitObject_ObjectRayOrigin(hitObject,component)
  %r277_oobc = call float @dx.op.hitObject_StateVector.f32(i32 277, %dx.types.HitObject %nop, i32 3)  ; HitObject_ObjectRayOrigin(hitObject,component)

  %r278 = call float @dx.op.hitObject_StateVector.f32(i32 278, %dx.types.HitObject undef, i32 0)  ; HitObject_ObjectRayDirection(hitObject,component)
  %r278_dync = call float @dx.op.hitObject_StateVector.f32(i32 278, %dx.types.HitObject %nop, i32 %r272)  ; HitObject_ObjectRayDirection(hitObject,component)
  %r278_oobc = call float @dx.op.hitObject_StateVector.f32(i32 278, %dx.types.HitObject %nop, i32 3)  ; HitObject_ObjectRayDirection(hitObject,component)

  %r279 = call float @dx.op.hitObject_StateMatrix.f32(i32 279, %dx.types.HitObject undef, i32 0, i32 0)  ; HitObject_ObjectToWorld3x4(hitObject,row,col)
  %r279_dynr = call float @dx.op.hitObject_StateMatrix.f32(i32 279, %dx.types.HitObject %nop, i32 %r272, i32 0)  ; HitObject_ObjectToWorld3x4(hitObject,row,col)
  %r279_oobr = call float @dx.op.hitObject_StateMatrix.f32(i32 279, %dx.types.HitObject %nop, i32 3, i32 0)  ; HitObject_ObjectToWorld3x4(hitObject,row,col)
  %r279_dync = call float @dx.op.hitObject_StateMatrix.f32(i32 279, %dx.types.HitObject %nop, i32 0, i32 %r272)  ; HitObject_ObjectToWorld3x4(hitObject,row,col)
  %r279_oobc = call float @dx.op.hitObject_StateMatrix.f32(i32 279, %dx.types.HitObject %nop, i32 0, i32 4)  ; HitObject_ObjectToWorld3x4(hitObject,row,col)

  %r280 = call float @dx.op.hitObject_StateMatrix.f32(i32 280, %dx.types.HitObject undef, i32 0, i32 0)  ; HitObject_WorldToObject3x4(hitObject,row,col)
  %r280_dynr = call float @dx.op.hitObject_StateMatrix.f32(i32 280, %dx.types.HitObject %nop, i32 %r272, i32 0)  ; HitObject_WorldToObject3x4(hitObject,row,col)
  %r280_oobr = call float @dx.op.hitObject_StateMatrix.f32(i32 280, %dx.types.HitObject %nop, i32 3, i32 0)  ; HitObject_WorldToObject3x4(hitObject,row,col)
  %r280_dync = call float @dx.op.hitObject_StateMatrix.f32(i32 280, %dx.types.HitObject %nop, i32 0, i32 %r272)  ; HitObject_WorldToObject3x4(hitObject,row,col)
  %r280_oobc = call float @dx.op.hitObject_StateMatrix.f32(i32 280, %dx.types.HitObject %nop, i32 0, i32 4)  ; HitObject_WorldToObject3x4(hitObject,row,col)

  %r281 = call i32 @dx.op.hitObject_StateScalar.i32(i32 281, %dx.types.HitObject undef)  ; HitObject_GeometryIndex(hitObject)

  %r282 = call i32 @dx.op.hitObject_StateScalar.i32(i32 282, %dx.types.HitObject undef)  ; HitObject_InstanceIndex(hitObject)

  %r283 = call i32 @dx.op.hitObject_StateScalar.i32(i32 283, %dx.types.HitObject undef)  ; HitObject_InstanceID(hitObject)

  %r284 = call i32 @dx.op.hitObject_StateScalar.i32(i32 284, %dx.types.HitObject undef)  ; HitObject_PrimitiveIndex(hitObject)

  %r285 = call i32 @dx.op.hitObject_StateScalar.i32(i32 285, %dx.types.HitObject undef)  ; HitObject_HitKind(hitObject)

  %r286 = call i32 @dx.op.hitObject_StateScalar.i32(i32 286, %dx.types.HitObject undef)  ; HitObject_ShaderTableIndex(hitObject)

  %r287 = call %dx.types.HitObject @dx.op.hitObject_SetShaderTableIndex(i32 287, %dx.types.HitObject undef, i32 1)  ; HitObject_SetShaderTableIndex(hitObject,shaderTableIndex)
  %r287_ud = call %dx.types.HitObject @dx.op.hitObject_SetShaderTableIndex(i32 287, %dx.types.HitObject undef, i32 undef)  ; HitObject_SetShaderTableIndex(hitObject,shaderTableIndex)

  %r288 = call i32 @dx.op.hitObject_LoadLocalRootTableConstant(i32 288, %dx.types.HitObject undef, i32 42)  ; HitObject_LoadLocalRootTableConstant(hitObject,offset)
  %r288_wrongmul = call i32 @dx.op.hitObject_LoadLocalRootTableConstant(i32 288, %dx.types.HitObject undef, i32 7)  ; HitObject_LoadLocalRootTableConstant(hitObject,offset)

  ret void
}

; Function Attrs: nounwind readnone
declare %dx.types.HitObject @dx.op.hitObject_MakeNop(i32) #1

; Function Attrs: nounwind readnone
declare %dx.types.HitObject @dx.op.hitObject_SetShaderTableIndex(i32, %dx.types.HitObject, i32) #1

; Function Attrs: nounwind readnone
declare i1 @dx.op.hitObject_StateScalar.i1(i32, %dx.types.HitObject) #1

; Function Attrs: nounwind readnone
declare i32 @dx.op.hitObject_StateScalar.i32(i32, %dx.types.HitObject) #1

; Function Attrs: nounwind readonly
declare i32 @dx.op.hitObject_LoadLocalRootTableConstant(i32, %dx.types.HitObject, i32) #2

; Function Attrs: nounwind readnone
declare float @dx.op.hitObject_StateVector.f32(i32, %dx.types.HitObject, i32) #1

; Function Attrs: nounwind readnone
declare float @dx.op.hitObject_StateScalar.f32(i32, %dx.types.HitObject) #1

; Function Attrs: nounwind readnone
declare float @dx.op.hitObject_StateMatrix.f32(i32, %dx.types.HitObject, i32, i32) #1

attributes #0 = { nounwind }
attributes #1 = { nounwind readnone }
attributes #2 = { nounwind readonly }
attributes #3 = { nounwind argmemonly }

!dx.version = !{!0}
!dx.valver = !{!0}
!dx.shaderModel = !{!1}
!dx.typeAnnotations = !{!2}
!dx.entryPoints = !{!3, !4}

!0 = !{i32 1, i32 9}
!1 = !{!"lib", i32 6, i32 9}
!2 = !{i32 1, void ()* @"\01?main@@YAXXZ", !5}
!3 = !{null, !"", null, null, !6}
!4 = !{void ()* @"\01?main@@YAXXZ", !"\01?main@@YAXXZ", null, null, !7}
!5 = !{!8}
!6 = !{i32 0, i64 0}
!7 = !{i32 8, i32 7, i32 5, !9}
!8 = !{i32 1, !10, !10}
!9 = !{i32 0}
!10 = !{}
