// REQUIRES: dxil-1-10
// RUN: %dxc -T cs_6_10 -E main %s | FileCheck %s
// RUN: %dxc -T cs_6_10 -E main -fcgl %s | FileCheck %s --check-prefix=CHECK2

[numthreads(1,1,1)]
void main() {
  // CHECK-LABEL: define void @main()

  // The FillMatrix calls are similar enough that they start matching
  // the CHECK-SAME lines so we consume them first.
  // CHECK: ; LinAlgFillMatrix(value)
  // CHECK: ; LinAlgFillMatrix(value)
  // CHECK: ; LinAlgFillMatrix(value)

  // Matrix<I32, 8, 4, A, ThreadGroup>
  __builtin_LinAlgMatrix [[__LinAlgMatrix_Attributes(4, 8, 4, 0, 2)]] matA;
  // Matrix<I32, 4, 8, B, ThreadGroup>
  __builtin_LinAlgMatrix [[__LinAlgMatrix_Attributes(4, 4, 8, 1, 2)]] matB;
  // Matrix<I32, 8, 8, Acc, ThreadGroup>
  __builtin_LinAlgMatrix [[__LinAlgMatrix_Attributes(4, 8, 8, 2, 2)]] matC;
  // Matrix<I32, 8, 8, Acc, ThreadGroup>
  __builtin_LinAlgMatrix [[__LinAlgMatrix_Attributes(4, 8, 8, 2, 2)]] matR;

  __builtin_LinAlg_FillMatrix(matA, 1);
  __builtin_LinAlg_FillMatrix(matB, 2);
  __builtin_LinAlg_FillMatrix(matC, 3);

  // CHECK: call %dx.types.LinAlgMatrixC4M8N8U2S2
  // CHECK-SAME: @dx.op.linAlgMatrixMultiplyAccumulate.mC4M8N8U2S2.mC4M8N4U0S2.mC4M4N8U1S2.mC4M8N8U2S2
  // CHECK-SAME: (i32 -2147483637, %dx.types.LinAlgMatrixC4M8N4U0S2 %{{[0-9]+}}, %dx.types.LinAlgMatrixC4M4N8U1S2 %{{[0-9]+}},
  // CHECK-SAME: %dx.types.LinAlgMatrixC4M8N8U2S2 %{{[0-9]+}}) ; LinAlgMatrixMultiplyAccumulate(matrixA,matrixB,matrixC)

  // CHECK2: call void @"dx.hl.op..void (i32, %dx.types.LinAlgMatrixC4M8N8U2S2*, %dx.types.LinAlgMatrixC4M8N4U0S2,
  // CHECK2-SAME: %dx.types.LinAlgMatrixC4M4N8U1S2, %dx.types.LinAlgMatrixC4M8N8U2S2)"(i32 413,
  // CHECK2-SAME: %dx.types.LinAlgMatrixC4M8N8U2S2* %{{.*}}, %dx.types.LinAlgMatrixC4M8N4U0S2 %{{[0-9]+}},
  // CHECK2-SAME: %dx.types.LinAlgMatrixC4M4N8U1S2 %{{[0-9]+}}, %dx.types.LinAlgMatrixC4M8N8U2S2 %{{[0-9]+}})

  __builtin_LinAlg_MatrixMatrixMultiplyAccumulate(matR, matA, matB, matC);
}
