; RUN: %dxv %s 2>&1 | FileCheck %s

; CHECK-NOT: uses a reserved prefix.
; CHECK-NOT: Validation failed.
; CHECK: Validation succeeded.

target datalayout = "e-m:e-p:32:32-i1:32-i8:8-i16:16-i32:32-i64:64-f16:16-f32:32-f64:64-n8:16:32:64"
target triple = "dxil-ms-dx"

%dx.types.LinAlgMatrixC8M16N16U0S0 = type { i8* }
%struct.ByteAddressBuffer = type { i32 }

define void @main() {
  %1 = alloca %dx.types.LinAlgMatrixC8M16N16U0S0
  ret void
}

!dx.targetTypes = !{!0}
!llvm.ident = !{!1}
!dx.version = !{!2}
!dx.valver = !{!2}
!dx.shaderModel = !{!3}
!dx.resources = !{!4}
!dx.entryPoints = !{!7}

!0 = !{%dx.types.LinAlgMatrixC8M16N16U0S0 undef, i32 8, i32 16, i32 16, i32 0, i32 0}
!1 = !{!"dxc(private) 1.9.0.5348 (issue-8433, 167fd829e-dirty)"}
!2 = !{i32 1, i32 10}
!3 = !{!"cs", i32 6, i32 10}
!4 = !{!5, null, null, null}
!5 = !{!6}
!6 = !{i32 0, %struct.ByteAddressBuffer* undef, !"", i32 0, i32 0, i32 1, i32 11, i32 0, null}
!7 = !{void ()* @main, !"main", null, !4, !8}
!8 = !{i32 0, i64 8388625, i32 4, !9}
!9 = !{i32 8, i32 1, i32 1}
