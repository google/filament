; REQUIRES: dxil-1-9
; RUN: not %dxv %s 2>&1 | FileCheck %s

target datalayout = "e-m:e-p:32:32-i1:32-i8:32-i16:32-i32:32-i64:64-f16:32-f32:32-f64:64-n8:16:32:64"
target triple = "dxil-ms-dx"

%struct.Payload = type { <3 x float> }
%dx.types.HitObject = type { i8* }

; CHECK: Function: ?main@@YAXXZ: error: Instructions should not read uninitialized value.
; CHECK-NEXT: note: at 'call void @dx.op.hitObject_Invoke.struct.Payload(i32 267, %dx.types.HitObject %nop, %struct.Payload* nonnull undef)' in block '#0' of function '?main@@YAXXZ'.
; CHECK-NEXT: Function: ?main@@YAXXZ: error: HitObject is undef.
; CHECK-NEXT: note: at 'call void @dx.op.hitObject_Invoke.struct.Payload(i32 267, %dx.types.HitObject undef, %struct.Payload* nonnull %pld)' in block '#0' of function '?main@@YAXXZ'.

; CHECK-NEXT: Validation failed.

; Function Attrs: nounwind
define void @"\01?main@@YAXXZ"() #0 {
  %pld = alloca %struct.Payload, align 4
  %nop = call %dx.types.HitObject @dx.op.hitObject_MakeNop(i32 266)  ; HitObject_MakeNop()
  call void @dx.op.hitObject_Invoke.struct.Payload(i32 267, %dx.types.HitObject %nop, %struct.Payload* nonnull %pld)  ; HitObject_Invoke(hitObject,payload)
  call void @dx.op.hitObject_Invoke.struct.Payload(i32 267, %dx.types.HitObject undef, %struct.Payload* nonnull %pld)  ; HitObject_Invoke(hitObject,payload)
  call void @dx.op.hitObject_Invoke.struct.Payload(i32 267, %dx.types.HitObject %nop, %struct.Payload* nonnull undef)  ; HitObject_Invoke(hitObject,payload)

  ret void
}

; Function Attrs: nounwind readnone
declare %dx.types.HitObject @dx.op.hitObject_MakeNop(i32) #1

; Function Attrs: nounwind
declare void @dx.op.hitObject_Invoke.struct.Payload(i32, %dx.types.HitObject, %struct.Payload*) #0

attributes #0 = { nounwind }
attributes #1 = { nounwind readnone }
attributes #2 = { nounwind readonly }

!dx.version = !{!0}
!dx.valver = !{!0}
!dx.shaderModel = !{!1}
!dx.typeAnnotations = !{!2}
!dx.dxrPayloadAnnotations = !{!3}
!dx.entryPoints = !{!4, !6}

!0 = !{i32 1, i32 9}
!1 = !{!"lib", i32 6, i32 9}
!2 = !{i32 1, void ()* @"\01?main@@YAXXZ", !7}
!3 = !{i32 0, %struct.Payload undef, !8}
!4 = !{null, !"", null, null, !5}
!5 = !{i32 0, i64 0}
!6 = !{void ()* @"\01?main@@YAXXZ", !"\01?main@@YAXXZ", null, null, !9}
!7 = !{!10}
!8 = !{!11}
!9 = !{i32 8, i32 7, i32 5, !12}
!10 = !{i32 1, !13, !13}
!11 = !{i32 0, i32 8210}
!12 = !{i32 0}
!13 = !{}
