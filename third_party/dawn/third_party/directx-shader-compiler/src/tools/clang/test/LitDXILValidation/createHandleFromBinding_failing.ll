; REQUIRES: dxil-1-8
; RUN: not %dxv %s 2>&1 | FileCheck %s

; Verify that createHandleFromBinding rejects out-of-range indices
; and invalid resource classes.

target datalayout = "e-m:e-p:32:32-i1:32-i8:8-i16:16-i32:32-i64:64-f16:16-f32:32-f64:64-n8:16:32:64"
target triple = "dxil-ms-dx"

%dx.types.Handle = type { i8* }
%dx.types.ResBind = type { i32, i32, i32, i8 }
%dx.types.ResourceProperties = type { i32, i32 }
%struct.RWByteAddressBuffer = type { i32 }

; --- Index below lower bound ---
; CHECK-DAG: error: Constant values must be in-range for operation.
; CHECK-DAG: note: at '%1 = call %dx.types.Handle @dx.op.createHandleFromBinding(i32 217, %dx.types.ResBind { i32 1, i32 3, i32 0, i8 1 }, i32 0, i1 false)' in block '#0' of function 'main'.

; --- Index above upper bound ---
; CHECK-DAG: error: Constant values must be in-range for operation.
; CHECK-DAG: note: at '%3 = call %dx.types.Handle @dx.op.createHandleFromBinding(i32 217, %dx.types.ResBind { i32 1, i32 3, i32 0, i8 1 }, i32 4, i1 false)' in block '#0' of function 'main'.

; --- Invalid resource class ---
; CHECK-DAG: error: Constant values must be in-range for operation.
; CHECK-DAG: note: at '%5 = call %dx.types.Handle @dx.op.createHandleFromBinding(i32 217, %dx.types.ResBind { i32 0, i32 0, i32 0, i8 5 }, i32 0, i1 false)' in block '#0' of function 'main'.

define void @main() {
  ; Index 0 is below rangeLowerBound=1
  %1 = call %dx.types.Handle @dx.op.createHandleFromBinding(i32 217, %dx.types.ResBind { i32 1, i32 3, i32 0, i8 1 }, i32 0, i1 false)
  %2 = call %dx.types.Handle @dx.op.annotateHandle(i32 216, %dx.types.Handle %1, %dx.types.ResourceProperties { i32 4107, i32 0 })
  ; Index 4 is above rangeUpperBound=3
  %3 = call %dx.types.Handle @dx.op.createHandleFromBinding(i32 217, %dx.types.ResBind { i32 1, i32 3, i32 0, i8 1 }, i32 4, i1 false)
  %4 = call %dx.types.Handle @dx.op.annotateHandle(i32 216, %dx.types.Handle %3, %dx.types.ResourceProperties { i32 4107, i32 0 })
  ; resourceClass=5 is invalid (valid: 0=SRV, 1=UAV, 2=CBuffer, 3=Sampler)
  %5 = call %dx.types.Handle @dx.op.createHandleFromBinding(i32 217, %dx.types.ResBind { i32 0, i32 0, i32 0, i8 5 }, i32 0, i1 false)
  %6 = call %dx.types.Handle @dx.op.annotateHandle(i32 216, %dx.types.Handle %5, %dx.types.ResourceProperties { i32 4107, i32 0 })
  ret void
}

declare %dx.types.Handle @dx.op.annotateHandle(i32, %dx.types.Handle, %dx.types.ResourceProperties) #0
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
!5 = !{i32 0, %struct.RWByteAddressBuffer* undef, !"", i32 0, i32 1, i32 3, i32 11, i1 false, i1 false, i1 false, null}
!6 = !{void ()* @main, !"main", null, !3, !7}
!7 = !{i32 0, i64 8589934608, i32 4, !8}
!8 = !{i32 4, i32 1, i32 1}
