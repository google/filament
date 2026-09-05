// REQUIRES: dxil-1-10
// RUN: %dxc -T cs_6_10 -E main %s | FileCheck %s
// RUN: %dxc -T cs_6_10 -E main -fcgl %s | FileCheck %s --check-prefix=CHECK2

ByteAddressBuffer inbuf;

[numthreads(1,1,1)]
void main() {
  // CHECK-LABEL: define void @main()

  // CHECK: %{{.*}} = call %dx.types.LinAlgMatrixC2M4N4U0S0 @dx.op.linAlgMatrixLoadFromDescriptor.mC2M4N4U0S0
  // CHECK-SAME: (i32 -2147483634, %dx.types.Handle %{{.*}}, i32 0, i32 0, i32 0, i32 256)
  // CHECK-SAME: ; LinAlgMatrixLoadFromDescriptor(handle,offset,stride,layout,align)

  // CHECK2: call void @"dx.hl.op..void (i32, %dx.types.LinAlgMatrixC2M4N4U0S0*, %dx.types.Handle, i32, i32, i32, i32)
  // CHECK2-SAME: "(i32 406, %dx.types.LinAlgMatrixC2M4N4U0S0* %mat, %dx.types.Handle {{.*}}, i32 0, i32 0, i32 0, i32 256)
  __builtin_LinAlgMatrix [[__LinAlgMatrix_Attributes(2, 4, 4, 0, 0)]] mat;
  __builtin_LinAlg_MatrixLoadFromDescriptor(mat, inbuf, 0, 0, 0, 256);
}
