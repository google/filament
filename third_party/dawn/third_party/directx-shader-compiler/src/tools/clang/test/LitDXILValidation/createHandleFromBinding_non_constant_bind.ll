; REQUIRES: dxil-1-8
; RUN: not %dxv %s 2>&1 | FileCheck %s

target datalayout = "e-m:e-p:32:32-i1:32-i8:8-i16:16-i32:32-i64:64-f16:16-f32:32-f64:64-n8:16:32:64"
target triple = "dxil-ms-dx"

%dx.types.Handle = type { i8* }
%dx.types.ResBind = type { i32, i32, i32, i8 }
%dx.types.ResourceProperties = type { i32, i32 }
%struct.RWByteAddressBuffer = type { i32 }

; CHECK: error: Constant values must be in-range for operation.
; CHECK: note: at '%2 = call %dx.types.Handle @dx.op.createHandleFromBinding(i32 217, %dx.types.ResBind %1,

define void @main(i32 %lowerBound) {
  ; Construct a non-constant bind struct using a function parameter
  %1 = insertvalue %dx.types.ResBind { i32 undef, i32 1, i32 0, i8 1 }, i32 %lowerBound, 0
  ; Pass non-constant bind to createHandleFromBinding — should fail validation
  %2 = call %dx.types.Handle @dx.op.createHandleFromBinding(i32 217, %dx.types.ResBind %1, i32 0, i1 false)  ; CreateHandleFromBinding(bind,index,nonUniformIndex)
  %3 = call %dx.types.Handle @dx.op.annotateHandle(i32 216, %dx.types.Handle %2, %dx.types.ResourceProperties { i32 4107, i32 0 })  ; AnnotateHandle(res,props)  resource: RWByteAddressBuffer
  ret void
}

; Function Attrs: nounwind readnone
declare %dx.types.Handle @dx.op.annotateHandle(i32, %dx.types.Handle, %dx.types.ResourceProperties) #0

; Function Attrs: nounwind readnone
declare %dx.types.Handle @dx.op.createHandleFromBinding(i32, %dx.types.ResBind, i32, i1) #0

attributes #0 = { nounwind readnone }

!llvm.ident = !{!0}
!dx.version = !{!1}
!dx.valver = !{!1}
!dx.shaderModel = !{!2}
!dx.resources = !{!3}
!dx.entryPoints = !{!6}

!0 = !{!"dxc(private) 1.8.0.15017 (main, 4e0f5364a-dirty)"}
!1 = !{i32 1, i32 8}
!2 = !{!"cs", i32 6, i32 8}
!3 = !{null, !4, null, null}
!4 = !{!5}
!5 = !{i32 0, %struct.RWByteAddressBuffer* undef, !"", i32 0, i32 0, i32 1, i32 11, i1 false, i1 false, i1 false, null}
!6 = !{void (i32)* @main, !"main", null, !3, !7}
!7 = !{i32 0, i64 8589934608, i32 4, !8}
!8 = !{i32 4, i32 1, i32 1}
