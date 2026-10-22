// REQUIRES: dxil-1-10
// RUN: %dxc -T cs_6_10 -E main %s | FileCheck %s
// RUN: %dxc -T cs_6_10 -E main -fcgl %s | FileCheck %s --check-prefix=CHECK2

[numthreads(1,1,1)]
void main() {
  // CHECK-LABEL: define void @main()

  // CHECK: call <2 x i32> @dx.op.linAlgMatrixGetCoordinate.mC4M5N4U1S2(i32 -2147483631,
  // CHECK-SAME: %dx.types.LinAlgMatrixC4M5N4U1S2 %{{.*}}, i32 1) 
  // CHECK-SAME: ; LinAlgMatrixGetCoordinate(matrix,threadLocalIndex)

  // CHECK2: call <2 x i32> @"dx.hl.op..<2 x i32> (i32, %dx.types.LinAlgMatrixC4M5N4U1S2, i32)"
  // CHECK2-SAME: (i32 403, %dx.types.LinAlgMatrixC4M5N4U1S2 %{{.*}}, i32 1)
  __builtin_LinAlgMatrix [[__LinAlgMatrix_Attributes(4, 5, 4, 1, 2)]] mat;
  __builtin_LinAlg_FillMatrix(mat, 1);
  uint2 coord = __builtin_LinAlg_MatrixGetCoordinate(mat, 1);
}
