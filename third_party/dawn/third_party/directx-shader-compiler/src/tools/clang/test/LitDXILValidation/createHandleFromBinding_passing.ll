; REQUIRES: dxil-1-8
; RUN: %dxv %s | FileCheck %s

; CHECK: Validation succeeded.

; Verify that valid createHandleFromBinding calls pass DXIL validation:
; in-range constant indices, valid resource classes, and non-constant
; indices on array resources.

target datalayout = "e-m:e-p:32:32-i1:32-i8:8-i16:16-i32:32-i64:64-f16:16-f32:32-f64:64-n8:16:32:64"
target triple = "dxil-ms-dx"

%dx.types.Handle = type { i8* }
%dx.types.ResBind = type { i32, i32, i32, i8 }
%dx.types.ResourceProperties = type { i32, i32 }
%struct.RWByteAddressBuffer = type { i32 }

define void @main() {
  ; Valid: index 1 is within [1, 3], resourceClass=1 (UAV)
  %1 = call %dx.types.Handle @dx.op.createHandleFromBinding(i32 217, %dx.types.ResBind { i32 1, i32 3, i32 0, i8 1 }, i32 1, i1 false)
  %2 = call %dx.types.Handle @dx.op.annotateHandle(i32 216, %dx.types.Handle %1, %dx.types.ResourceProperties { i32 4107, i32 0 })
  ; Valid: index 3 is within [1, 3] (upper bound)
  %3 = call %dx.types.Handle @dx.op.createHandleFromBinding(i32 217, %dx.types.ResBind { i32 1, i32 3, i32 0, i8 1 }, i32 3, i1 false)
  %4 = call %dx.types.Handle @dx.op.annotateHandle(i32 216, %dx.types.Handle %3, %dx.types.ResourceProperties { i32 4107, i32 0 })
  ; Valid: non-constant index on array resource (range [1, 3])
  %5 = call i32 @dx.op.flattenedThreadIdInGroup.i32(i32 96)
  %6 = call %dx.types.Handle @dx.op.createHandleFromBinding(i32 217, %dx.types.ResBind { i32 1, i32 3, i32 0, i8 1 }, i32 %5, i1 false)
  %7 = call %dx.types.Handle @dx.op.annotateHandle(i32 216, %dx.types.Handle %6, %dx.types.ResourceProperties { i32 4107, i32 0 })
  ret void
}

declare i32 @dx.op.flattenedThreadIdInGroup.i32(i32) #0
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
