// REQUIRES: dxil-1-10
// RUN: %dxc -T cs_6_10 -HV 202x -E main %s | FileCheck %s
// RUN: %dxc -T cs_6_10 -HV 202x -E main -fcgl %s | FileCheck %s --check-prefix=CHECK2

// CHECK: @"\01?SharedArr@@3PAMA" = external addrspace(3) global [64 x float], align 4
groupshared float SharedArr[64];

[numthreads(4,1,1)]
void main() {
  // CHECK-LABEL: define void @main()

  // CHECK: call %dx.types.LinAlgMatrixC4M5N4U1S2 @dx.op.linAlgMatrixLoadFromMemory.mC4M5N4U1S2.f32
  // CHECK-SAME; (i32 -2147483633, float addrspace(3)* getelementptr inbounds ([64 x float],
  // CHECK-SAME: [64 x float] addrspace(3)* @"\01?SharedArr@@3PAMA", i32 0, i32 0), i32 1, i32 2, i32 3)
  // CHECK-SAME: ; LinAlgMatrixLoadFromMemory(memory,offset,stride,layout)

  // CHECK2: call void @"dx.hl.op..void (i32, %dx.types.LinAlgMatrixC4M5N4U1S2*, [64 x float] addrspace(3)*,
  // CHECK2-SAME: i32, i32, i32)"(i32 407, %dx.types.LinAlgMatrixC4M5N4U1S2* %mat, [64 x float] addrspace(3)*
  // CHECK2-SAME: @"\01?SharedArr@@3PAMA", i32 1, i32 2, i32 3)
  __builtin_LinAlgMatrix [[__LinAlgMatrix_Attributes(4, 5, 4, 1, 2)]] mat;
  __builtin_LinAlg_MatrixLoadFromMemory(mat, SharedArr, 1, 2, 3);
}
