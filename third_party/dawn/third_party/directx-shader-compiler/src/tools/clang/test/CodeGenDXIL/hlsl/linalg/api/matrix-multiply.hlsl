// REQUIRES: dxil-1-10
// RUN: %dxc -T cs_6_10 %s | FileCheck %s

#include <dx/linalg.h>
using namespace dx::linalg;

[numthreads(4, 4, 4)]
void main()
{
// Multiply in Wave scope
//
  using MatrixAF16WTy = Matrix<ComponentType::F16, 3, 4, MatrixUse::A, MatrixScope::Wave>;
  using MatrixAI32WTy = Matrix<ComponentType::I32, 3, 4, MatrixUse::A, MatrixScope::Wave>;
  using MatrixBI32WTy = Matrix<ComponentType::I32, 4, 5, MatrixUse::B, MatrixScope::Wave>;
  using MatrixAccF32WTy = Matrix<ComponentType::F32, 3, 5, MatrixUse::Accumulator, MatrixScope::Wave>;
  using MatrixAccI32WTy = Matrix<ComponentType::I32, 3, 5, MatrixUse::Accumulator, MatrixScope::Wave>;

// CHECK: %[[MATA1:.*]] = call %dx.types.LinAlgMatrixC8M3N4U0S1 @dx.op.linAlgFillMatrix.mC8M3N4U0S1.f32(
// CHECK-SAME: i32 -2147483636, float 1.500000e+00)  ; LinAlgFillMatrix(value)
  MatrixAF16WTy MatA1 = MatrixAF16WTy::Splat(1.5f);

// CHECK: %[[MATA2:.*]] = call %dx.types.LinAlgMatrixC4M3N4U0S1 @dx.op.linAlgFillMatrix.mC4M3N4U0S1.i32(
// CHECK-SAME: i32 -2147483636, i32 45)  ; LinAlgFillMatrix(value)
  MatrixAI32WTy MatA2 = MatrixAI32WTy::Splat(45);

// CHECK: %[[MATB1:.*]] = call %dx.types.LinAlgMatrixC4M4N5U1S1 @dx.op.linAlgFillMatrix.mC4M4N5U1S1.i32(
// CHECK-SAME: i32 -2147483636, i32 13)  ; LinAlgFillMatrix(value)
  MatrixBI32WTy MatB1 = MatrixBI32WTy::Splat(13);

// CHECK: %[[MATC1:.*]] = call %dx.types.LinAlgMatrixC9M3N5U2S1
// CHECK-SAME: @dx.op.linAlgMatrixMultiply.mC9M3N5U2S1.mC8M3N4U0S1.mC4M4N5U1S1(i32 -2147483625,
// CHECK-SAME: %dx.types.LinAlgMatrixC8M3N4U0S1 %[[MATA1]], %dx.types.LinAlgMatrixC4M4N5U1S1 %[[MATB1]])
// CHECK-SAME: ; LinAlgMatrixMultiply(matrixA,matrixB)
  MatrixAccF32WTy MatCFlt1 = Multiply<ComponentType::F32>(MatA1, MatB1);

// CHECK: %[[MATC2:.*]] = call %dx.types.LinAlgMatrixC4M3N5U2S1
// CHECK-SAME: @dx.op.linAlgMatrixMultiply.mC4M3N5U2S1.mC4M3N4U0S1.mC4M4N5U1S1(i32 -2147483625,
// CHECK-SAME: %dx.types.LinAlgMatrixC4M3N4U0S1 %2, %dx.types.LinAlgMatrixC4M4N5U1S1 %3)
// CHECK-SAME: ; LinAlgMatrixMultiply(matrixA,matrixB)
  MatrixAccI32WTy MatCInt1 = Multiply(MatA2, MatB1);

// Multiply in ThreadGroup scope
//
  using MatrixAF16TGTy = Matrix<ComponentType::F16, 3, 4, MatrixUse::A, MatrixScope::ThreadGroup>;
  using MatrixAI32TGTy = Matrix<ComponentType::I32, 3, 4, MatrixUse::A, MatrixScope::ThreadGroup>;
  using MatrixBI32TGTy = Matrix<ComponentType::I32, 4, 5, MatrixUse::B, MatrixScope::ThreadGroup>;
  using MatrixAccF32TGTy = Matrix<ComponentType::F32, 3, 5, MatrixUse::Accumulator, MatrixScope::ThreadGroup>;
  using MatrixAccI32TGTy = Matrix<ComponentType::I32, 3, 5, MatrixUse::Accumulator, MatrixScope::ThreadGroup>;

// CHECK: %[[MATA3:.*]] = call %dx.types.LinAlgMatrixC8M3N4U0S2 @dx.op.linAlgFillMatrix.mC8M3N4U0S2.f32(
// CHECK-SAME: i32 -2147483636, float 2.500000e+00)  ; LinAlgFillMatrix(value)
  MatrixAF16TGTy MatA3 = MatrixAF16TGTy::Splat(2.5f);

// CHECK: %[[MATA4:.*]] = call %dx.types.LinAlgMatrixC4M3N4U0S2 @dx.op.linAlgFillMatrix.mC4M3N4U0S2.i32(
// CHECK-SAME: i32 -2147483636, i32 23)  ; LinAlgFillMatrix(value)
  MatrixAI32TGTy MatA4 = MatrixAI32TGTy::Splat(23);
  
// CHECK: %[[MATB3:.*]] = call %dx.types.LinAlgMatrixC4M4N5U1S2 @dx.op.linAlgFillMatrix.mC4M4N5U1S2.i32(
// CHECK-SAME: i32 -2147483636, i32 7)  ; LinAlgFillMatrix(value)
  MatrixBI32TGTy MatB3 = MatrixBI32TGTy::Splat(7);

// CHECK: %[[MATC3:.*]] = call %dx.types.LinAlgMatrixC9M3N5U2S2
// CHECK-SAME: @dx.op.linAlgMatrixMultiply.mC9M3N5U2S2.mC8M3N4U0S2.mC4M4N5U1S2(i32 -2147483625,
// CHECK-SAME: %dx.types.LinAlgMatrixC8M3N4U0S2 %[[MATA3]], %dx.types.LinAlgMatrixC4M4N5U1S2 %[[MATB3]])
// CHECK-SAME: ; LinAlgMatrixMultiply(matrixA,matrixB)
  MatrixAccF32TGTy MatCFlt2 = Multiply<ComponentType::F32>(MatA3, MatB3);

// CHECK: %[[MATC4:.*]] = call %dx.types.LinAlgMatrixC4M3N5U2S2
// CHECK-SAME: @dx.op.linAlgMatrixMultiply.mC4M3N5U2S2.mC4M3N4U0S2.mC4M4N5U1S2(i32 -2147483625,
// CHECK-SAME: %dx.types.LinAlgMatrixC4M3N4U0S2 %[[MATA4]], %dx.types.LinAlgMatrixC4M4N5U1S2 %[[MATB3]])
// CHECK-SAME: ; LinAlgMatrixMultiply(matrixA,matrixB)
  MatrixAccI32TGTy MatCInt2 = Multiply(MatA4, MatB3);
}
