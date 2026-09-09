// REQUIRES: dxil-1-10
// RUN: %dxc -T cs_6_10 -E main %s | FileCheck %s
// RUN: %dxc -T cs_6_10 -E main -fcgl %s | FileCheck %s --check-prefix=CHECK2

RWByteAddressBuffer outbuf;

[numthreads(1,1,1)]
void main() {
  // CHECK-LABEL: define void @main()

  // CHECK: call void @dx.op.linAlgMatrixAccumulateToDescriptor.mC9M4N4U2S1(i32 -2147483621,
  // CHECK-SAME: %dx.types.LinAlgMatrixC9M4N4U2S1 %{{.*}}, %dx.types.Handle %{{.*}}, i32 0, i32 0, i32 0, i32 128)
  // CHECK-SAME: ; LinAlgMatrixAccumulateToDescriptor(matrix,handle,offset,stride,layout,align)

  // CHECK2: call void @"dx.hl.op..void (i32, %dx.types.LinAlgMatrixC9M4N4U2S1, %dx.types.Handle, i32, i32, i32, i32)"
  // CHECK2-SAME: (i32 415, %dx.types.LinAlgMatrixC9M4N4U2S1 %{{.*}}, %dx.types.Handle {{.*}}, i32 0, i32 0, i32 0, i32 128)

  // Matrix<F16, 4, 4, Accumulator, Wave>
  __builtin_LinAlgMatrix [[__LinAlgMatrix_Attributes(9, 4, 4, 2, 1)]] mat;
  __builtin_LinAlg_FillMatrix(mat, 1);
  __builtin_LinAlg_MatrixAccumulateToDescriptor(mat, outbuf, 0, 0, 0, 128);
}
