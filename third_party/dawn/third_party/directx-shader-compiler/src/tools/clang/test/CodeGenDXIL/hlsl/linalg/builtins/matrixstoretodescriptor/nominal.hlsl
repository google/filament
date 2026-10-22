// REQUIRES: dxil-1-10
// RUN: %dxc -T cs_6_10 -E main %s | FileCheck %s
// RUN: %dxc -T cs_6_10 -E main -fcgl %s | FileCheck %s --check-prefix=CHECK2

RWByteAddressBuffer outbuf;

[numthreads(1,1,1)]
void main() {
  // CHECK-LABEL: define void @main()

  // CHECK: call void @dx.op.linAlgMatrixStoreToDescriptor.mC4M5N4U1S2(i32 -2147483628,
  // CHECK-SAME: %dx.types.LinAlgMatrixC4M5N4U1S2 %{{.*}}, %dx.types.Handle %{{.*}}, i32 1, i32 1, i32 0, i32 256)
  // CHECK-SAME:  ; LinAlgMatrixStoreToDescriptor(matrix,handle,offset,stride,layout,align)

  // CHECK2: call void @"dx.hl.op..void (i32, %dx.types.LinAlgMatrixC4M5N4U1S2, %dx.types.Handle, i32, i32, i32, i32)
  // CHECK2-SAME: "(i32 409, %dx.types.LinAlgMatrixC4M5N4U1S2 %{{.*}}, %dx.types.Handle {{.*}}, i32 1, i32 1, i32 0, i32 256)
  __builtin_LinAlgMatrix [[__LinAlgMatrix_Attributes(4, 5, 4, 1, 2)]] mat;
  __builtin_LinAlg_FillMatrix(mat, 1);
  __builtin_LinAlg_MatrixStoreToDescriptor(mat, outbuf, 1, 1, 0, 256);
}
