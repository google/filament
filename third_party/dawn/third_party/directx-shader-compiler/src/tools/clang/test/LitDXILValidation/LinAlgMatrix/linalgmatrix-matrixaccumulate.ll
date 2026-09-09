; REQUIRES: dxil-1-10
; RUN: not %dxv %s 2>&1 | FileCheck %s
target datalayout = "e-m:e-p:32:32-i1:32-i8:32-i16:32-i32:32-i64:64-f16:32-f32:32-f64:64-n8:16:32:64"
target triple = "dxil-ms-dx"

%dx.types.LinAlgMatrixC8M4N4U2S2 = type { i8* }
%dx.types.LinAlgMatrixC8M4N4U0S2 = type { i8* }
%dx.types.LinAlgMatrixC8M4N4U2S0 = type { i8* }
%dx.types.LinAlgMatrixC8M4N4U1S0 = type { i8* }
%dx.types.LinAlgMatrixC8M4N4U1S1 = type { i8* }
%dx.types.LinAlgMatrixC8M8N8U0S2 = type { i8* }

define void @main() {
  %1 = call %dx.types.LinAlgMatrixC8M4N4U2S2 @dx.op.linAlgFillMatrix.mC8M4N4U2S2.i32(i32 -2147483636, i32 1)  ; LinAlgFillMatrix(value)
  %2 = call %dx.types.LinAlgMatrixC8M4N4U0S2 @dx.op.linAlgFillMatrix.mC8M4N4U0S2.i32(i32 -2147483636, i32 2)  ; LinAlgFillMatrix(value)
  %3 = call %dx.types.LinAlgMatrixC8M4N4U2S0 @dx.op.linAlgFillMatrix.mC8M4N4U2S0.i32(i32 -2147483636, i32 4)  ; LinAlgFillMatrix(value)
  %4 = call %dx.types.LinAlgMatrixC8M4N4U1S0 @dx.op.linAlgFillMatrix.mC8M4N4U1S0.i32(i32 -2147483636, i32 5)  ; LinAlgFillMatrix(value)
  %5 = call %dx.types.LinAlgMatrixC8M4N4U1S1 @dx.op.linAlgFillMatrix.mC8M4N4U1S1.i32(i32 -2147483636, i32 6)  ; LinAlgFillMatrix(value)
  %6 = call %dx.types.LinAlgMatrixC8M8N8U0S2 @dx.op.linAlgFillMatrix.mC8M8N8U0S2.i32(i32 -2147483636, i32 7)  ; LinAlgFillMatrix(value)

  ; CHECK: Function: main: error: Return matrix 'dx.types.LinAlgMatrixC8M4N4U2S2' must exactly match arg 1 matrix 'dx.types.LinAlgMatrixC8M4N4U0S2'.
  ; CHECK-NEXT: note: at {{.*}} @dx.op.linAlgMatrixAccumulate.mC8M4N4U2S2.mC8M4N4U0S2.mC8M4N4U0S2
  %7 = call %dx.types.LinAlgMatrixC8M4N4U2S2 @dx.op.linAlgMatrixAccumulate.mC8M4N4U2S2.mC8M4N4U0S2.mC8M4N4U0S2(i32 -2147483624, %dx.types.LinAlgMatrixC8M4N4U0S2 %2, %dx.types.LinAlgMatrixC8M4N4U0S2 %2)  ; LinAlgMatrixAccumulate(matrixLHS,matrixRHS)

  ; CHECK-NEXT: Function: main: error: Return matrix use 'A' does not match expected use Accumulator.
  ; CHECK-NEXT: note: at {{.*}} @dx.op.linAlgMatrixAccumulate.mC8M4N4U0S2.mC8M4N4U0S2.mC8M4N4U0S2
  %8 = call %dx.types.LinAlgMatrixC8M4N4U0S2 @dx.op.linAlgMatrixAccumulate.mC8M4N4U0S2.mC8M4N4U0S2.mC8M4N4U0S2(i32 -2147483624, %dx.types.LinAlgMatrixC8M4N4U0S2 %2, %dx.types.LinAlgMatrixC8M4N4U0S2 %2)  ; LinAlgMatrixAccumulate(matrixLHS,matrixRHS)

  ; CHECK-NEXT: Function: main: error: Arg 2 matrix use 'Accumulator' does not match expected use A or B.
  ; CHECK-NEXT: note: at {{.*}} @dx.op.linAlgMatrixAccumulate.mC8M4N4U2S2.mC8M4N4U2S2.mC8M4N4U2S2
  %9 = call %dx.types.LinAlgMatrixC8M4N4U2S2 @dx.op.linAlgMatrixAccumulate.mC8M4N4U2S2.mC8M4N4U2S2.mC8M4N4U2S2(i32 -2147483624, %dx.types.LinAlgMatrixC8M4N4U2S2 %1, %dx.types.LinAlgMatrixC8M4N4U2S2 %1)  ; LinAlgMatrixAccumulate(matrixLHS,matrixRHS)

  ; CHECK-NEXT: Function: main: error: Return matrix scope 'ThreadGroup' must match Arg 2 matrix scope 'Wave'.
  ; CHECK-NEXT: note: at {{.*}} @dx.op.linAlgMatrixAccumulate.mC8M4N4U2S2.mC8M4N4U2S2.mC8M4N4U1S1
  %10 = call %dx.types.LinAlgMatrixC8M4N4U2S2 @dx.op.linAlgMatrixAccumulate.mC8M4N4U2S2.mC8M4N4U2S2.mC8M4N4U1S1(i32 -2147483624, %dx.types.LinAlgMatrixC8M4N4U2S2 %1, %dx.types.LinAlgMatrixC8M4N4U1S1 %5)  ; LinAlgMatrixAccumulate(matrixLHS,matrixRHS)

  ; CHECK-NEXT: Function: main: error: Arg 2 matrix scope 'Thread' does not match expected scope Wave or ThreadGroup.
  ; CHECK-NEXT: note: at {{.*}} @dx.op.linAlgMatrixAccumulate.mC8M4N4U2S0.mC8M4N4U2S0.mC8M4N4U1S0
  ; CHECK-NEXT: Function: main: error: Return matrix scope 'Thread' does not match expected scope Wave or ThreadGroup.
  ; CHECK-NEXT: note: at {{.*}} @dx.op.linAlgMatrixAccumulate.mC8M4N4U2S0.mC8M4N4U2S0.mC8M4N4U1S0
  %11 = call %dx.types.LinAlgMatrixC8M4N4U2S0 @dx.op.linAlgMatrixAccumulate.mC8M4N4U2S0.mC8M4N4U2S0.mC8M4N4U1S0(i32 -2147483624, %dx.types.LinAlgMatrixC8M4N4U2S0 %3, %dx.types.LinAlgMatrixC8M4N4U1S0 %4)  ; LinAlgMatrixAccumulate(matrixLHS,matrixRHS)

  ; CHECK-NEXT: Function: main: error: Arg 2 matrix dimension '8x8' must match return matrix dimension '4x4'.
  ; CHECK-NEXT: note: at {{.*}} @dx.op.linAlgMatrixAccumulate.mC8M4N4U2S2.mC8M4N4U2S2.mC8M8N8U0S2
  %12 = call %dx.types.LinAlgMatrixC8M4N4U2S2 @dx.op.linAlgMatrixAccumulate.mC8M4N4U2S2.mC8M4N4U2S2.mC8M8N8U0S2(i32 -2147483624, %dx.types.LinAlgMatrixC8M4N4U2S2 %1, %dx.types.LinAlgMatrixC8M8N8U0S2 %6)  ; LinAlgMatrixAccumulate(matrixLHS,matrixRHS)

  ; CHECK-NEXT: Validation failed

  ret void
}

; Function Attrs: nounwind
declare %dx.types.LinAlgMatrixC8M4N4U2S2 @dx.op.linAlgFillMatrix.mC8M4N4U2S2.i32(i32, i32) #0

; Function Attrs: nounwind
declare %dx.types.LinAlgMatrixC8M4N4U0S2 @dx.op.linAlgFillMatrix.mC8M4N4U0S2.i32(i32, i32) #0

; Function Attrs: nounwind
declare %dx.types.LinAlgMatrixC8M4N4U2S0 @dx.op.linAlgFillMatrix.mC8M4N4U2S0.i32(i32, i32) #0

; Function Attrs: nounwind
declare %dx.types.LinAlgMatrixC8M4N4U1S0 @dx.op.linAlgFillMatrix.mC8M4N4U1S0.i32(i32, i32) #0

; Function Attrs: nounwind
declare %dx.types.LinAlgMatrixC8M4N4U1S1 @dx.op.linAlgFillMatrix.mC8M4N4U1S1.i32(i32, i32) #0

; Function Attrs: nounwind
declare %dx.types.LinAlgMatrixC8M8N8U0S2 @dx.op.linAlgFillMatrix.mC8M8N8U0S2.i32(i32, i32) #0

; Function Attrs: nounwind
declare %dx.types.LinAlgMatrixC8M4N4U2S2 @dx.op.linAlgMatrixAccumulate.mC8M4N4U2S2.mC8M4N4U0S2.mC8M4N4U0S2(i32, %dx.types.LinAlgMatrixC8M4N4U0S2, %dx.types.LinAlgMatrixC8M4N4U0S2) #0

; Function Attrs: nounwind
declare %dx.types.LinAlgMatrixC8M4N4U0S2 @dx.op.linAlgMatrixAccumulate.mC8M4N4U0S2.mC8M4N4U0S2.mC8M4N4U0S2(i32, %dx.types.LinAlgMatrixC8M4N4U0S2, %dx.types.LinAlgMatrixC8M4N4U0S2) #0

; Function Attrs: nounwind
declare %dx.types.LinAlgMatrixC8M4N4U2S2 @dx.op.linAlgMatrixAccumulate.mC8M4N4U2S2.mC8M4N4U2S2.mC8M4N4U2S2(i32, %dx.types.LinAlgMatrixC8M4N4U2S2, %dx.types.LinAlgMatrixC8M4N4U2S2) #0

; Function Attrs: nounwind
declare %dx.types.LinAlgMatrixC8M4N4U2S2 @dx.op.linAlgMatrixAccumulate.mC8M4N4U2S2.mC8M4N4U2S2.mC8M4N4U1S1(i32, %dx.types.LinAlgMatrixC8M4N4U2S2, %dx.types.LinAlgMatrixC8M4N4U1S1) #0

; Function Attrs: nounwind
declare %dx.types.LinAlgMatrixC8M4N4U2S0 @dx.op.linAlgMatrixAccumulate.mC8M4N4U2S0.mC8M4N4U2S0.mC8M4N4U1S0(i32, %dx.types.LinAlgMatrixC8M4N4U2S0, %dx.types.LinAlgMatrixC8M4N4U1S0) #0

; Function Attrs: nounwind
declare %dx.types.LinAlgMatrixC8M4N4U2S2 @dx.op.linAlgMatrixAccumulate.mC8M4N4U2S2.mC8M4N4U2S2.mC8M8N8U0S2(i32, %dx.types.LinAlgMatrixC8M4N4U2S2, %dx.types.LinAlgMatrixC8M8N8U0S2) #0

attributes #0 = { nounwind }

!dx.targetTypes = !{!0, !1, !2, !3, !4, !5}
!llvm.ident = !{!6}
!dx.version = !{!7}
!dx.valver = !{!7}
!dx.shaderModel = !{!8}
!dx.entryPoints = !{!9}

!0 = !{%dx.types.LinAlgMatrixC8M4N4U2S2 undef, i32 8, i32 4, i32 4, i32 2, i32 2}
!1 = !{%dx.types.LinAlgMatrixC8M4N4U0S2 undef, i32 8, i32 4, i32 4, i32 0, i32 2}
!2 = !{%dx.types.LinAlgMatrixC8M4N4U2S0 undef, i32 8, i32 4, i32 4, i32 2, i32 0}
!3 = !{%dx.types.LinAlgMatrixC8M4N4U1S0 undef, i32 8, i32 4, i32 4, i32 1, i32 0}
!4 = !{%dx.types.LinAlgMatrixC8M4N4U1S1 undef, i32 8, i32 4, i32 4, i32 1, i32 1}
!5 = !{%dx.types.LinAlgMatrixC8M8N8U0S2 undef, i32 8, i32 8, i32 8, i32 0, i32 2}
!6 = !{!"dxc(private) 1.9.0.5391 (linalg-validation-copyconvert, 977f44792-dirty)"}
!7 = !{i32 1, i32 10}
!8 = !{!"cs", i32 6, i32 10}
!9 = !{void ()* @main, !"main", null, null, !10}
!10 = !{i32 4, !11}
!11 = !{i32 1, i32 1, i32 1}
