; REQUIRES: dxil-1-8
; RUN: not %dxv %s 2>&1 | FileCheck %s


; CHECK: error: reordercoherent requires SM 6.9 or later. 'buf'
; CHECK-NEXT: Function: ?main@@YAXXZ: error: reordercoherent requires SM 6.9 or later.
; CHECK-NEXT: note: at '%3 = call %dx.types.Handle @dx.op.annotateHandle(i32 216, %dx.types.Handle %2, %dx.types.ResourceProperties { i32 69643, i32 0 })' in block '#0' of function '?main@@YAXXZ'.
; CHECK-NEXT: Function: ?main@@YAXXZ: error: reordercoherent requires SM 6.9 or later.
; CHECK-NEXT: note: at '%3 = call %dx.types.Handle @dx.op.annotateHandle(i32 216, %dx.types.Handle %2, %dx.types.ResourceProperties { i32 69643, i32 0 })' in block '#0' of function '?main@@YAXXZ'.
; CHECK-NEXT: Validation failed.
; COM: Original HLSL source:
; COM: reordercoherent RWByteAddressBuffer buf;
; COM:
; COM: [Shader("raygeneration")]
; COM: void main()
; COM: {
; COM:   buf.Store(0, 11.f);
; COM: }

; shader hash: f7be6354830d1423764991adcfc26b0b
;
; Buffer Definitions:
;
;
; Resource Bindings:
;
; Name                                 Type  Format         Dim      ID      HLSL Bind  Count
; ------------------------------ ---------- ------- ----------- ------- -------------- ------
; buf                                   UAV    byte         r/w      U0u4294967295,space4294967295     1
;
target datalayout = "e-m:e-p:32:32-i1:32-i8:32-i16:32-i32:32-i64:64-f16:32-f32:32-f64:64-n8:16:32:64"
target triple = "dxil-ms-dx"

%dx.types.Handle = type { i8* }
%dx.types.ResourceProperties = type { i32, i32 }
%struct.RWByteAddressBuffer = type { i32 }

@"\01?buf@@3URWByteAddressBuffer@@A" = external constant %dx.types.Handle, align 4

; Function Attrs: nounwind
define void @"\01?main@@YAXXZ"() #0 {
  %1 = load %dx.types.Handle, %dx.types.Handle* @"\01?buf@@3URWByteAddressBuffer@@A", align 4
  %2 = call %dx.types.Handle @dx.op.createHandleForLib.dx.types.Handle(i32 160, %dx.types.Handle %1)  ; CreateHandleForLib(Resource)
  %3 = call %dx.types.Handle @dx.op.annotateHandle(i32 216, %dx.types.Handle %2, %dx.types.ResourceProperties { i32 69643, i32 0 })  ; AnnotateHandle(res,props)  resource: reordercoherent RWByteAddressBuffer
  call void @dx.op.rawBufferStore.f32(i32 140, %dx.types.Handle %3, i32 0, i32 undef, float 1.100000e+01, float undef, float undef, float undef, i8 1, i32 4)  ; RawBufferStore(uav,index,elementOffset,value0,value1,value2,value3,mask,alignment)
  ret void
}

; Function Attrs: nounwind
declare void @dx.op.rawBufferStore.f32(i32, %dx.types.Handle, i32, i32, float, float, float, float, i8, i32) #0

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
!dx.entryPoints = !{!4, !5}

!0 = !{i32 1, i32 8}
!1 = !{!"lib", i32 6, i32 8}
!2 = !{null, !6, null, null}
!3 = !{i32 1, void ()* @"\01?main@@YAXXZ", !7}
!4 = !{null, !"", null, !2, !8}
!5 = !{void ()* @"\01?main@@YAXXZ", !"\01?main@@YAXXZ", null, null, !9}
!6 = !{!10}
!7 = !{!11}
!8 = !{i32 0, i64 8589934608}
!9 = !{i32 8, i32 7, i32 5, !12}
!10 = !{i32 0, %struct.RWByteAddressBuffer* bitcast (%dx.types.Handle* @"\01?buf@@3URWByteAddressBuffer@@A" to %struct.RWByteAddressBuffer*), !"buf", i32 -1, i32 -1, i32 1, i32 11, i1 false, i1 false, i1 false, !13}
!11 = !{i32 1, !14, !14}
!12 = !{i32 0}
!13 = !{i32 4, i1 true}
!14 = !{}
