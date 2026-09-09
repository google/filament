// REQUIRES: dxil-1-10
// RUN: %dxc -T cs_6_10 -E main %s | FileCheck %s
// RUN: %dxc -T cs_6_10 -E main -fcgl %s | FileCheck %s --check-prefix=CHECK2

[numthreads(1,1,1)]
void main() {
  // CHECK-LABEL: define void @main()

  __builtin_LinAlgMatrix [[__LinAlgMatrix_Attributes(5, 4, 4, 0, 1)]] mat1;
  __builtin_LinAlgMatrix [[__LinAlgMatrix_Attributes(5, 4, 4, 2, 1)]] mat2;
  __builtin_LinAlgMatrix [[__LinAlgMatrix_Attributes(5, 4, 4, 2, 1)]] mat3;

  __builtin_LinAlg_FillMatrix(mat1, 1);
  __builtin_LinAlg_FillMatrix(mat2, 2);

  // CHECK: call %dx.types.LinAlgMatrixC5M4N4U2S1 @dx.op.linAlgMatrixAccumulate.mC5M4N4U2S1.mC5M4N4U2S1.mC5M4N4U0S1
  // CHECK-SAME: (i32 -2147483624, %dx.types.LinAlgMatrixC5M4N4U2S1 %{{.*}}, %dx.types.LinAlgMatrixC5M4N4U0S1 %{{.*}}) ; LinAlgMatrixAccumulate(matrixLHS,matrixRHS)

  // CHECK2: call void @"dx.hl.op..void (i32, %dx.types.LinAlgMatrixC5M4N4U2S1*, %dx.types.LinAlgMatrixC5M4N4U2S1,
  // CHECK2-SAME:  %dx.types.LinAlgMatrixC5M4N4U0S1)"(i32 411, %dx.types.LinAlgMatrixC5M4N4U2S1* %mat3,
  // CHECK2-SAME: %dx.types.LinAlgMatrixC5M4N4U2S1 %{{[0-9]+}}, %dx.types.LinAlgMatrixC5M4N4U0S1 %{{[0-9]+}})
  __builtin_LinAlg_MatrixAccumulate(mat3, mat2, mat1);
}
