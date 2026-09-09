; REQUIRES: dxil-1-9
; RUN: not %dxv %s 2>&1 | FileCheck %s

; COM: Original HLSL source:
; COM: reordercoherent RWStructuredBuffer<float> buffer;
; COM:
; COM:
; COM: [Shader("raygeneration")]
; COM: void
; COM: main()
; COM: {
; COM:   buffer.IncrementCounter();
; COM:   buffer.DecrementCounter();
; COM: }

; CHECK: error: reordercoherent cannot be used on buffer with counter 'buffer'
; CHECK-NEXT: Validation failed.

; shader hash: 638950814a9023bf537d61dbb330a4c8
;
; Buffer Definitions:
;
; Resource bind info for buffer
; {
;
;   float $Element;                                   ; Offset:    0 Size:     4
;
; }
;
;
; Resource Bindings:
;
; Name                                 Type  Format         Dim      ID      HLSL Bind  Count
; ------------------------------ ---------- ------- ----------- ------- -------------- ------
; buffer                                UAV  struct     r/w+cnt      U0u4294967295,space4294967295     1
;
target datalayout = "e-m:e-p:32:32-i1:32-i8:32-i16:32-i32:32-i64:64-f16:32-f32:32-f64:64-n8:16:32:64"
target triple = "dxil-ms-dx"

%dx.types.Handle = type { i8* }
%dx.types.ResourceProperties = type { i32, i32 }
%"class.RWStructuredBuffer<float>" = type { float }

@"\01?buffer@@3V?$RWStructuredBuffer@M@@A" = external constant %dx.types.Handle, align 4

; Function Attrs: nounwind
define void @"\01?main@@YAXXZ"() #0 {
  %1 = load %dx.types.Handle, %dx.types.Handle* @"\01?buffer@@3V?$RWStructuredBuffer@M@@A", align 4
  %2 = call %dx.types.Handle @dx.op.createHandleForLib.dx.types.Handle(i32 160, %dx.types.Handle %1)  ; CreateHandleForLib(Resource)
  %3 = call %dx.types.Handle @dx.op.annotateHandle(i32 216, %dx.types.Handle %2, %dx.types.ResourceProperties { i32 102412, i32 4 })  ; AnnotateHandle(res,props)  resource: reordercoherent RWStructuredBuffer<stride=4, counter>
  %4 = call i32 @dx.op.bufferUpdateCounter(i32 70, %dx.types.Handle %3, i8 1)  ; BufferUpdateCounter(uav,inc)
  %5 = call %dx.types.Handle @dx.op.createHandleForLib.dx.types.Handle(i32 160, %dx.types.Handle %1)  ; CreateHandleForLib(Resource)
  %6 = call %dx.types.Handle @dx.op.annotateHandle(i32 216, %dx.types.Handle %5, %dx.types.ResourceProperties { i32 102412, i32 4 })  ; AnnotateHandle(res,props)  resource: reordercoherent RWStructuredBuffer<stride=4, counter>
  %7 = call i32 @dx.op.bufferUpdateCounter(i32 70, %dx.types.Handle %6, i8 -1)  ; BufferUpdateCounter(uav,inc)
  ret void
}

; Function Attrs: nounwind
declare i32 @dx.op.bufferUpdateCounter(i32, %dx.types.Handle, i8) #0

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
!dx.typeAnnotations = !{!6}
!dx.entryPoints = !{!10, !12}

!0 = !{i32 1, i32 9}
!1 = !{!"lib", i32 6, i32 9}
!2 = !{null, !3, null, null}
!3 = !{!4}
!4 = !{i32 0, %"class.RWStructuredBuffer<float>"* bitcast (%dx.types.Handle* @"\01?buffer@@3V?$RWStructuredBuffer@M@@A" to %"class.RWStructuredBuffer<float>"*), !"buffer", i32 -1, i32 -1, i32 1, i32 12, i1 false, i1 true, i1 false, !5}
!5 = !{i32 1, i32 4, i32 4, i1 true}
!6 = !{i32 1, void ()* @"\01?main@@YAXXZ", !7}
!7 = !{!8}
!8 = !{i32 1, !9, !9}
!9 = !{}
!10 = !{null, !"", null, !2, !11}
!11 = !{i32 0, i64 8589934608}
!12 = !{void ()* @"\01?main@@YAXXZ", !"\01?main@@YAXXZ", null, null, !13}
!13 = !{i32 8, i32 7, i32 5, !14}
!14 = !{i32 0}