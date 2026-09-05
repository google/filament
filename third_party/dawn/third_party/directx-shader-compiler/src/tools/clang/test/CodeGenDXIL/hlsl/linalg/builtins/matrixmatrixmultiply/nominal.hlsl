// REQUIRES: dxil-1-10
// RUN: %dxc -T cs_6_10 -E main %s | FileCheck %s
// RUN: %dxc -T cs_6_10 -E main -fcgl %s | FileCheck %s --check-prefix=CHECK2

[numthreads(1,1,1)]
void main() {
  // CHECK-LABEL: define void @main()

  // Matrix<I32, 8, 4, A, ThreadGroup>
  __builtin_LinAlgMatrix [[__LinAlgMatrix_Attributes(4, 8, 4, 0, 2)]] matA;
  // Matrix<I32, 4, 8, B, ThreadGroup>
  __builtin_LinAlgMatrix [[__LinAlgMatrix_Attributes(4, 4, 8, 1, 2)]] matB;
  // Matrix<I32, 8, 8, Acc, ThreadGroup>
  __builtin_LinAlgMatrix [[__LinAlgMatrix_Attributes(4, 8, 8, 2, 2)]] matC;

  __builtin_LinAlg_FillMatrix(matA, 1);
  __builtin_LinAlg_FillMatrix(matB, 2);

  // CHECK: call %dx.types.LinAlgMatrixC4M8N8U2S2 @dx.op.linAlgMatrixMultiply.mC4M8N8U2S2.mC4M8N4U0S2.mC4M4N8U1S2(i32 -2147483625,
  // CHECK-SAME: %dx.types.LinAlgMatrixC4M8N4U0S2 %{{.*}}, %dx.types.LinAlgMatrixC4M4N8U1S2 %{{.*}}) ; LinAlgMatrixMultiply(matrixA,matrixB)

  // CHECK2: call void @"dx.hl.op..void (i32, %dx.types.LinAlgMatrixC4M8N8U2S2*, %dx.types.LinAlgMatrixC4M8N4U0S2,
  // CHECK2-SAME: %dx.types.LinAlgMatrixC4M4N8U1S2)"(i32 412, %dx.types.LinAlgMatrixC4M8N8U2S2* %matC,
  // CHECK2-SAME: %dx.types.LinAlgMatrixC4M8N4U0S2 %{{[0-9]+}}, %dx.types.LinAlgMatrixC4M4N8U1S2 %{{[0-9]+}})

  __builtin_LinAlg_MatrixMatrixMultiply(matC, matA, matB);
}
