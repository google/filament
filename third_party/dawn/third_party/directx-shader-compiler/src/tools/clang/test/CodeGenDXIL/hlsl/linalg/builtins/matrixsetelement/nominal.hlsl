// REQUIRES: dxil-1-10
// RUN: %dxc -T cs_6_10 -E main %s | FileCheck %s
// RUN: %dxc -T cs_6_10 -E main -fcgl %s | FileCheck %s --check-prefix=CHECK2

ByteAddressBuffer inbuf;
RWByteAddressBuffer outbuf;

[numthreads(1,1,1)]
void main() {
  // CHECK-LABEL: define void @main()

  // CHECK: call %dx.types.LinAlgMatrixC4M5N4U1S2 @dx.op.linAlgMatrixSetElement.mC4M5N4U1S2.mC4M5N4U1S2.i32
  // CHECK-SAME: (i32 -2147483629, %dx.types.LinAlgMatrixC4M5N4U1S2 %{{.*}}, i32 1, i32 5)
  // CHECK-SAME: ; LinAlgMatrixSetElement(matrix,threadLocalIndex,value)

  // CHECK2: call void @"dx.hl.op..void (i32, %dx.types.LinAlgMatrixC4M5N4U1S2*, %dx.types.LinAlgMatrixC4M5N4U1S2, i32, i32)
  // CHECK2-SAME: "(i32 408, %dx.types.LinAlgMatrixC4M5N4U1S2* %{{.*}}, %dx.types.LinAlgMatrixC4M5N4U1S2 %{{.*}}, i32 1, i32 5)

  __builtin_LinAlgMatrix [[__LinAlgMatrix_Attributes(4, 5, 4, 1, 2)]] mat1;
  __builtin_LinAlg_FillMatrix(mat1, 1);

  __builtin_LinAlgMatrix [[__LinAlgMatrix_Attributes(4, 5, 4, 1, 2)]] mat2;
  __builtin_LinAlg_MatrixSetElement(mat2, mat1, 1, 5);
}
