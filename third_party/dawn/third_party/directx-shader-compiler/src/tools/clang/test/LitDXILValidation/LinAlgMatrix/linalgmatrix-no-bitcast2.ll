; REQUIRES: dxil-1-10
; RUN: not %dxv %s 2>&1 | FileCheck %s

target datalayout = "e-m:e-p:32:32-i1:32-i8:32-i16:32-i32:32-i64:64-f16:32-f32:32-f64:64-n8:16:32:64"
target triple = "dxil-ms-dx"

%dx.types.LinAlgMatrixC4M5N4U1S2 = type { i8* }
%dx.types.LinAlgMatrixC4M4N5U1S2 = type { i8* }

define void @main() {
  %1 = call %dx.types.LinAlgMatrixC4M5N4U1S2 @dx.op.linAlgFillMatrix.mC4M5N4U1S2.i32(i32 -2147483636, i32 5)  ; LinAlgFillMatrix(value)
  %2 = bitcast %dx.types.LinAlgMatrixC4M5N4U1S2 %1 to i32
  ret void
  ; CHECK: shader: invalid cast opcode for cast from '%dx.types.LinAlgMatrixC4M5N4U1S2 = type { i8* }' to 'i32'
}

; Function Attrs: nounwind
declare %dx.types.LinAlgMatrixC4M5N4U1S2 @dx.op.linAlgFillMatrix.mC4M5N4U1S2.i32(i32, i32) #0

attributes #0 = { nounwind }

!dx.targetTypes = !{!0,!7}
!llvm.ident = !{!1}
!dx.version = !{!2}
!dx.valver = !{!2}
!dx.shaderModel = !{!3}
!dx.entryPoints = !{!4}

!0 = !{%dx.types.LinAlgMatrixC4M5N4U1S2 undef, i32 4, i32 5, i32 4, i32 1, i32 2}
!1 = !{!"dxc(private) 1.9.0.5391 (linalg-validation-no-undef, a2cfae071-dirty)"}
!2 = !{i32 1, i32 10}
!3 = !{!"cs", i32 6, i32 10}
!4 = !{void ()* @main, !"main", null, null, !5}
!5 = !{i32 4, !6}
!6 = !{i32 1, i32 1, i32 1}
!7 = !{%dx.types.LinAlgMatrixC4M4N5U1S2 undef, i32 4, i32 4, i32 5, i32 1, i32 2}
