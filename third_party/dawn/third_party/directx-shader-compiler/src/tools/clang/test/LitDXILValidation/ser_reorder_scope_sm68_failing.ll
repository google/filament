; REQUIRES: dxil-1-8
; RUN: not %dxv %s 2>&1 | FileCheck %s

; Buffer Definitions:
;
;
; Resource Bindings:
;
; Name                                 Type  Format         Dim      ID      HLSL Bind  Count
; ------------------------------ ---------- ------- ----------- ------- -------------- ------
; BAB                                   UAV    byte         r/w      U0             u1     1
;
target datalayout = "e-m:e-p:32:32-i1:32-i8:32-i16:32-i32:32-i64:64-f16:32-f32:32-f64:64-n8:16:32:64"
target triple = "dxil-ms-dx"

%dx.types.Handle = type { i8* }
%dx.types.ResourceProperties = type { i32, i32 }
%struct.RWByteAddressBuffer = type { i32 }

@"\01?BAB@@3URWByteAddressBuffer@@A" = external constant %dx.types.Handle, align 4

; CHECK: Function: ?main@@YAXXZ: error: Invalid semantic flags on DXIL operation 'BarrierByMemoryType'
; CHECK-NEXT: note: at 'call void @dx.op.barrierByMemoryType(i32 244, i32 1, i32 8)' in block '#0' of function '?main@@YAXXZ'.
; CHECK-NEXT: Function: ?main@@YAXXZ: error: Invalid semantic flags on DXIL operation 'barrierByMemoryHandle'
; CHECK-NEXT: note: at 'call void @dx.op.barrierByMemoryHandle(i32 245, %dx.types.Handle %3, i32 8)' in block '#0' of function '?main@@YAXXZ'.
; CHECK-NEXT: Function: ?main@@YAXXZ: error: Entry function performs some operation that is incompatible with the shader stage or other entry properties.  See other errors for details.
; CHECK-NEXT: Function: ?main@@YAXXZ: error: Function uses features incompatible with the shader model.
; CHECK-NEXT: Validation failed.

; Function Attrs: nounwind
define void @"\01?main@@YAXXZ"() #0 {
  %1 = load %dx.types.Handle, %dx.types.Handle* @"\01?BAB@@3URWByteAddressBuffer@@A", align 4
  call void @dx.op.barrierByMemoryType(i32 244, i32 1, i32 8)  ; BarrierByMemoryType(MemoryTypeFlags,SemanticFlags)
  %2 = call %dx.types.Handle @dx.op.createHandleForLib.dx.types.Handle(i32 160, %dx.types.Handle %1)  ; CreateHandleForLib(Resource)
  %3 = call %dx.types.Handle @dx.op.annotateHandle(i32 216, %dx.types.Handle %2, %dx.types.ResourceProperties { i32 4107, i32 0 })  ; AnnotateHandle(res,props)  resource: RWByteAddressBuffer
  call void @dx.op.barrierByMemoryHandle(i32 245, %dx.types.Handle %3, i32 8)  ; BarrierByMemoryHandle(object,SemanticFlags)
  ret void
}

; Function Attrs: noduplicate nounwind
declare void @dx.op.barrierByMemoryType(i32, i32, i32) #1

; Function Attrs: noduplicate nounwind
declare void @dx.op.barrierByMemoryHandle(i32, %dx.types.Handle, i32) #1

; Function Attrs: nounwind readnone
declare %dx.types.Handle @dx.op.annotateHandle(i32, %dx.types.Handle, %dx.types.ResourceProperties) #2

; Function Attrs: nounwind readonly
declare %dx.types.Handle @dx.op.createHandleForLib.dx.types.Handle(i32, %dx.types.Handle) #3

attributes #0 = { nounwind }
attributes #1 = { noduplicate nounwind }
attributes #2 = { nounwind readnone }
attributes #3 = { nounwind readonly }

!dx.version = !{!0}
!dx.valver = !{!0}
!dx.shaderModel = !{!1}
!dx.resources = !{!2}
!dx.typeAnnotations = !{!5}
!dx.entryPoints = !{!9, !11}

!0 = !{i32 1, i32 8}
!1 = !{!"lib", i32 6, i32 8}
!2 = !{null, !3, null, null}
!3 = !{!4}
!4 = !{i32 0, %struct.RWByteAddressBuffer* bitcast (%dx.types.Handle* @"\01?BAB@@3URWByteAddressBuffer@@A" to %struct.RWByteAddressBuffer*), !"BAB", i32 0, i32 1, i32 1, i32 11, i1 false, i1 false, i1 false, null}
!5 = !{i32 1, void ()* @"\01?main@@YAXXZ", !6}
!6 = !{!7}
!7 = !{i32 1, !8, !8}
!8 = !{}
!9 = !{null, !"", null, !2, !10}
!10 = !{i32 0, i64 8589934608}
!11 = !{void ()* @"\01?main@@YAXXZ", !"\01?main@@YAXXZ", null, null, !12}
!12 = !{i32 8, i32 7, i32 5, !13}
!13 = !{i32 0}
