// REQUIRES: dxil-1-10
// RUN: %dxc -T cs_6_10 -E main %s | FileCheck %s
// RUN: %dxc -T cs_6_10 -E main -fcgl %s | FileCheck %s --check-prefix=CHECK2

[numthreads(1,1,1)]
void main() {
  // CHECK-LABEL: define void @main()

  // CHECK: call %dx.types.LinAlgMatrixC4M5N4U1S2 @dx.op.linAlgCopyConvertMatrix.mC4M5N4U1S2.mC2M5N4U1S2
  // CHECK-SAME: (i32 -2147483635, %dx.types.LinAlgMatrixC2M5N4U1S2 %{{.*}}, i1 false)  
  // CHECK-SAME: ; LinAlgCopyConvertMatrix(srcMatrix,transpose)

  // CHECK2: call void @"dx.hl.op..void (i32, %dx.types.LinAlgMatrixC4M5N4U1S2*, %dx.types.LinAlgMatrixC2M5N4U1S2, i1)"
  // CHECK2-SAME: (i32 401, %dx.types.LinAlgMatrixC4M5N4U1S2* %{{.*}}, %dx.types.LinAlgMatrixC2M5N4U1S2 {{.*}}, i1 false)
  __builtin_LinAlgMatrix [[__LinAlgMatrix_Attributes(2, 5, 4, 1, 2)]] mat1;
  __builtin_LinAlgMatrix [[__LinAlgMatrix_Attributes(4, 5, 4, 1, 2)]] mat2;

  __builtin_LinAlg_FillMatrix(mat1, 1);

  __builtin_LinAlg_CopyConvertMatrix(mat2, mat1, false);
}
