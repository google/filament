; REQUIRES: dxil-1-10
; RUN: not %dxv %s 2>&1 | FileCheck %s

target datalayout = "e-m:e-p:32:32-i1:32-i8:8-i16:16-i32:32-i64:64-f16:16-f32:32-f64:64-n8:16:32:64"
target triple = "dxil-ms-dx"

%dx.types.LinAlgMatrixC8M4N4U2S0 = type { i8* }

define void @main() {
  ; CHECK: Function: main: error: Return matrix scope 'Thread' does not match expected scope Wave or ThreadGroup.
  ; CHECK-NEXT: note: at {{.*}} @dx.op.linAlgFillMatrix.mC8M4N4U2S0.i32
  %1 = call %dx.types.LinAlgMatrixC8M4N4U2S0 @dx.op.linAlgFillMatrix.mC8M4N4U2S0.i32(i32 -2147483636, i32 1)  ; LinAlgFillMatrix(value)

  ; CHECK-NEXT: Function: main: error: Input matrix scope 'Thread' does not match expected scope Wave or ThreadGroup.
  ; CHECK-NEXT: note: at {{.*}} @dx.op.linAlgMatrixGetElement.f32.mC8M4N4U2S0
  %2 = call float @dx.op.linAlgMatrixGetElement.f32.mC8M4N4U2S0(i32 -2147483630, %dx.types.LinAlgMatrixC8M4N4U2S0 %1, i32 1)  ; LinAlgMatrixGetElement(matrix,threadLocalIndex)

  ; CHECK-NEXT: Function: main: error: Input matrix scope 'Thread' does not match expected scope Wave or ThreadGroup.
  ; CHECK-NEXT: note: at {{.*}} @dx.op.linAlgMatrixSetElement.mC8M4N4U2S0.mC8M4N4U2S0.f32
  ; CHECK-NEXT: Function: main: error: Return matrix scope 'Thread' does not match expected scope Wave or ThreadGroup.
  ; CHECK-NEXT: note: at {{.*}} @dx.op.linAlgMatrixSetElement.mC8M4N4U2S0.mC8M4N4U2S0.f32
  %3 = call %dx.types.LinAlgMatrixC8M4N4U2S0 @dx.op.linAlgMatrixSetElement.mC8M4N4U2S0.mC8M4N4U2S0.f32(i32 -2147483629, %dx.types.LinAlgMatrixC8M4N4U2S0 %1, i32 1, float %2)  ; LinAlgMatrixSetElement(matrix,threadLocalIndex,value)

  ; CHECK-NEXT: Function: main: error: Input matrix scope 'Thread' does not match expected scope Wave or ThreadGroup.
  ; CHECK-NEXT: note: at {{.*}} @dx.op.linAlgMatrixGetCoordinate.mC8M4N4U2S0
  %4 = call <2 x i32> @dx.op.linAlgMatrixGetCoordinate.mC8M4N4U2S0(i32 -2147483631, %dx.types.LinAlgMatrixC8M4N4U2S0 %3, i32 0)  ; LinAlgMatrixGetCoordinate(matrix,threadLocalIndex)

  ; CHECK-NEXT: Function: main: error: Input matrix scope 'Thread' does not match expected scope Wave or ThreadGroup.
  ; CHECK-NEXT: note: at {{.*}} @dx.op.linAlgMatrixLength.mC8M4N4U2S0
  %5 = call i32 @dx.op.linAlgMatrixLength.mC8M4N4U2S0(i32 -2147483632, %dx.types.LinAlgMatrixC8M4N4U2S0 %3)  ; LinAlgMatrixLength(matrix)

  ; CHECK-NEXT: Validation failed.
  ret void
}

; Function Attrs: nounwind
declare %dx.types.LinAlgMatrixC8M4N4U2S0 @dx.op.linAlgFillMatrix.mC8M4N4U2S0.i32(i32, i32) #0

; Function Attrs: nounwind
declare float @dx.op.linAlgMatrixGetElement.f32.mC8M4N4U2S0(i32, %dx.types.LinAlgMatrixC8M4N4U2S0, i32) #0

; Function Attrs: nounwind
declare %dx.types.LinAlgMatrixC8M4N4U2S0 @dx.op.linAlgMatrixSetElement.mC8M4N4U2S0.mC8M4N4U2S0.f32(i32, %dx.types.LinAlgMatrixC8M4N4U2S0, i32, float) #0

; Function Attrs: nounwind
declare <2 x i32> @dx.op.linAlgMatrixGetCoordinate.mC8M4N4U2S0(i32, %dx.types.LinAlgMatrixC8M4N4U2S0, i32) #0

; Function Attrs: nounwind
declare i32 @dx.op.linAlgMatrixLength.mC8M4N4U2S0(i32, %dx.types.LinAlgMatrixC8M4N4U2S0) #0

attributes #0 = { nounwind }

!dx.targetTypes = !{!0}
!llvm.ident = !{!1}
!dx.version = !{!2}
!dx.valver = !{!2}
!dx.shaderModel = !{!3}
!dx.entryPoints = !{!4}

!0 = !{%dx.types.LinAlgMatrixC8M4N4U2S0 undef, i32 8, i32 4, i32 4, i32 2, i32 0}
!1 = !{!"dxc(private) 1.9.0.5430 (linalg-vali-matrixaccumtodescriptor, b02cf0883-dirty)"}
!2 = !{i32 1, i32 10}
!3 = !{!"cs", i32 6, i32 10}
!4 = !{void ()* @main, !"main", null, null, !5}
!5 = !{i32 0, i64 8388608, i32 4, !6}
!6 = !{i32 1, i32 1, i32 1}

